/*
  ==============================================================================

    KeyboardMappedSoundboard.cpp
    Created: 16 Jan 2022 9:21:14pm
    Author:  Lucien Lefebvre

  ==============================================================================
*/

#include <JuceHeader.h>
#include "KeyboardMappedSoundboard.h"

//==============================================================================
KeyboardMappedSoundboard::KeyboardMappedSoundboard(Settings* s)
{
    settings = s;
    settings->keyMappedSoundboardSize->addChangeListener(this);

    juce::Timer::startTimer(50);
}

KeyboardMappedSoundboard::~KeyboardMappedSoundboard()
{
}

void KeyboardMappedSoundboard::paint (juce::Graphics& g)
{

    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
}

void KeyboardMappedSoundboard::resized()
{
    rowNumber = Settings::keyMappedSoundboardRows;
    columnNumber = Settings::keyMappedSoundboardColumns;

    playerWidth = (getWidth() - spaceBetweenPlayers * (columnNumber + 1)) / columnNumber ;
    if (Settings::showMeter)
    {
        playerWidth = playerWidth * 4 / 5;
        meterWidth = playerWidth / 5 - 4;
    }
    else
        meterWidth = 0;

    playerHeight = getHeight() / rowNumber - spaceBetweenPlayers * (rowNumber + 1);

    int playerIdInLine = 0;
    for (int i = 0; i < mappedPlayers.size(); i++)
    {
        int line = i / 10;
        int lineXStart = spaceBetweenPlayers + (spaceBetweenPlayers + playerHeight) * line;

        mappedPlayers[i]->setBounds(spaceBetweenPlayers + (playerIdInLine * (playerWidth + spaceBetweenPlayers + meterWidth)), lineXStart, playerWidth, playerHeight);
        meters[i]->setBounds(mappedPlayers[i]->getRight() + 2, lineXStart, meterWidth, playerHeight);

        if (playerIdInLine < Settings::keyMappedSoundboardColumns)
            mappedPlayers[i]->setVisible(true);
        else
            mappedPlayers[i]->setVisible(false);
        
        playerIdInLine++;
        if (playerIdInLine == 10)
            playerIdInLine = 0;
    }
}

void KeyboardMappedSoundboard::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    for (auto meter : meters)
    {
        meter->prepareToPlay(samplesPerBlockExpected, sampleRate);
    }

}

void KeyboardMappedSoundboard::fileDragMove(const juce::StringArray& files, int x, int y)
{
    juce::Point<int> mousePosition(x, y);
    auto pointOnTopLevel = getLocalPoint(getTopLevelComponent(), mousePosition);
    for (auto* player : mappedPlayers)
    {
        if (player->getBounds().contains(pointOnTopLevel))
        {
            player->isDraggedOver(true);
        }
        else
            player->isDraggedOver(false);
    }
}

void KeyboardMappedSoundboard::fileDragExit(const juce::StringArray& files, int x, int y)
{
    for (auto* player : mappedPlayers)
    {
        player->isDraggedOver(false);
    }
}

bool KeyboardMappedSoundboard::isInterestedInFileDrag(const juce::StringArray& files)
{
    bool isInterested = isInterested;
    for (auto file : files)
    {
        juce::File f(file);
        for (auto format : Settings::getAcceptedFileFormats())
        {
            if (f.hasFileExtension(format))
                return true;
        }
    }
    return false;
}

void KeyboardMappedSoundboard::filesDropped(const juce::StringArray& files, int x, int y)
{
    auto pos = juce::Point<int>(x, y);
    for (auto* player : mappedPlayers)
    {
        if (player->getBounds().contains(pos))
        {
            player->loadFile(files[0], "");
        }
        player->isDraggedOver(false);
    }
}
void KeyboardMappedSoundboard::setDroppedFile(juce::Point<int> p,juce::String path,juce::String name)
{
    auto pointOnTopLevel = getLocalPoint(getTopLevelComponent(), p);
    for (auto* player : mappedPlayers)
    {
        if (player->getBounds().contains(pointOnTopLevel))
        {
            player->loadFile(path, name);
        }
    }
}

void KeyboardMappedSoundboard::fileDragExit(const juce::StringArray& files)
{
    for (auto* player : mappedPlayers)
    {
        player->isDraggedOver(false);
    }
}
void KeyboardMappedSoundboard::mouseDrag(const juce::MouseEvent& event)
{
    if (draggedPlayer != nullptr)
    {
        for (auto* player : mappedPlayers)
        {
            if (player->getScreenBounds().contains(event.getScreenPosition()))
            {
                player->isDraggedOver(true);
            }
            else
            {
                player->isDraggedOver(false);
            }
        }
    }
}

void KeyboardMappedSoundboard::mouseExit(const juce::MouseEvent& event)
{
    destinationPlayer = nullptr;
}

void KeyboardMappedSoundboard::mouseUp(const juce::MouseEvent& event)
{
    for (auto* player : mappedPlayers)
    {
        player->isDraggedOver(false);
        if (player->getScreenBounds().contains(event.getScreenPosition()))
        {
            destinationPlayer = player;
        }
    }
    if (draggedPlayer != nullptr && destinationPlayer != nullptr && draggedPlayer != destinationPlayer)
    {
        auto source = draggedPlayer->getPlayer();
        auto destination = destinationPlayer->getPlayer();
        if (source != nullptr && destination != nullptr)
        {
            destination->setPlayerInfo(source->getPlayerInfo());
            source->deleteFile();
        }
    }
    draggedPlayer = nullptr;
    destinationPlayer = nullptr;
    grabFocusBroadcaster->sendChangeMessage();
}

KeyMappedPlayer* KeyboardMappedSoundboard::getPlayer(int i)
{
    return mappedPlayers[i];
}

void KeyboardMappedSoundboard::setPlayer(Player* p, int i)
{
    mappedPlayers[i]->setPlayer(p);
}

KeyMappedPlayer* KeyboardMappedSoundboard::addPlayer(Player* p)
{
    auto addedPlayer = mappedPlayers.add(new KeyMappedPlayer());
    addAndMakeVisible(addedPlayer);
    addedPlayer->setPlayer(p);
    addedPlayer->setMouseClickGrabsKeyboardFocus(false);
    addedPlayer->repaint();

    auto addedMeter = meters.add(new Meter(Meter::Mode::Stereo));
    addAndMakeVisible(*addedMeter);
    addedMeter->shouldDrawScale(false);
    addedMeter->shouldDrawScaleNumbers(false);
    addedMeter->setMeterColour(juce::Colours::green);
    addedMeter->setRectangleRoundSize(4);
    addedMeter->setMouseClickGrabsKeyboardFocus(false);
    
    return addedPlayer;
}

void KeyboardMappedSoundboard::setShortcutKeys()
{
    for (int i = 0; i < mappedPlayers.size(); i++)
    {
        if (Settings::keyboardLayout == 1)
            mappedPlayers[i]->setShortcut(qwertyShortcutArray[i]);
        else if (Settings::keyboardLayout == 2)
            mappedPlayers[i]->setShortcut(azertyShortcutArray[i]);
    }
}

bool KeyboardMappedSoundboard::keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent)
{
    bool isCommandDown = false;
    if (key.getModifiers().isCommandDown())
    {
        isCommandDown = true;
    }
    for (auto player : mappedPlayers)
    {
        if (key.isKeyCode(player->getShortcut().getKeyCode()))
        {
            player->shortcutKeyPressed(isCommandDown);
        }
    }
    return false;
}


void KeyboardMappedSoundboard::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == settings->keyMappedSoundboardSize.get())
    {
        resized();
        return;
    }
    for (auto player : mappedPlayers)
    {
        if (source == player->playerDraggedBroadcaster.get())
        {
            draggedPlayer = player;
            return;
        }
    }
}

void KeyboardMappedSoundboard::timerCallback()
{
    if (Settings::showMeter)
    {
        for (int i = 0; i < meters.size(); i++)
        {         
            meters[i]->setRMSMeterData(mappedPlayers[i]->getPlayer()->outMeterSource.getRMSLevel(0), 
                                        mappedPlayers[i]->getPlayer()->outMeterSource.getRMSLevel(1));
            meters[i]->setPeakMeterDate(mappedPlayers[i]->getPlayer()->outMeterSource.getMaxLevel(0), 
                                        mappedPlayers[i]->getPlayer()->outMeterSource.getMaxLevel(1));
        }
    }
}