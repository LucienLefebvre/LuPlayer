/*
  ==============================================================================

    KeyboardMappedSoundboard.cpp
    Created: 16 Jan 2022 9:21:14pm
    Author:  DPR

  ==============================================================================
*/

#include <JuceHeader.h>
#include "KeyboardMappedSoundboard.h"

//==============================================================================
KeyboardMappedSoundboard::KeyboardMappedSoundboard()
{

    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

KeyboardMappedSoundboard::~KeyboardMappedSoundboard()
{
}

void KeyboardMappedSoundboard::paint (juce::Graphics& g)
{

    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
    //g.fillAll (juce::Colours::red);   // clear the background
}

void KeyboardMappedSoundboard::resized()
{
    playerWidth = (getWidth() - spaceBetweenPlayers * 11) / 10 ;
    playerHeight = getHeight() / 3 - spaceBetweenPlayers * 4;
    int playerIdInLine = 0;
    for (int i = 0; i < mappedPlayers.size(); i++)
    {
        int line = i / 10;
        int lineXStart = spaceBetweenPlayers + (spaceBetweenPlayers + playerHeight) * line;
        mappedPlayers[i]->setBounds(spaceBetweenPlayers + (playerIdInLine * (playerWidth + spaceBetweenPlayers)) , lineXStart, playerWidth, playerHeight);
        playerIdInLine++;
        if (playerIdInLine == 10)
            playerIdInLine = 0;
    }
}

void KeyboardMappedSoundboard::fileDragMove(const juce::StringArray& files, int x, int y)
{
    juce::Point<int> mousePosition(x, y);
    for (auto* player : mappedPlayers)
    {
        if (player->getBounds().contains(mousePosition))
        {
            player->isDraggedOver(true);
        }
        else
            player->isDraggedOver(false);
    }
}

void KeyboardMappedSoundboard::setDroppedFile(juce::Point<int> p,juce::String path,juce::String name)
{
    for (auto* player : mappedPlayers)
    {
        if (player->getBounds().contains(p))
        {
            player->loadFile(path, name);
        }
    }
}

void KeyboardMappedSoundboard::fileDragExit()
{
    for (auto* player : mappedPlayers)
    {
        player->isDraggedOver(false);
    }
}
void KeyboardMappedSoundboard::mouseDrag(const juce::MouseEvent& event)
{
}

KeyMappedPlayer* KeyboardMappedSoundboard::getPlayer(int i)
{
    return mappedPlayers[i];
}

void KeyboardMappedSoundboard::setPlayer(Player* p, int i)
{
    mappedPlayers[i]->setPlayer(p);
}

void KeyboardMappedSoundboard::addPlayer(Player* p)
{
    mappedPlayers.add(new KeyMappedPlayer());
    addAndMakeVisible(*mappedPlayers.getLast());
    mappedPlayers.getLast()->setPlayer(p);
    mappedPlayers.getLast()->setMouseClickGrabsKeyboardFocus(false);
    mappedPlayers.getLast()->repaint();
    //resized();
}

void KeyboardMappedSoundboard::setShortcutKeys()
{
    for (int i = 0; i < mappedPlayers.size(); i++)
    {
        mappedPlayers[i]->setShortcut(shortcutArray[i]);
    }
}

bool KeyboardMappedSoundboard::keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent)
{
    for (int i = 0; i < mappedPlayers.size(); i++)
    {
        if (juce::KeyPress::createFromDescription(shortcutArray[i]) == key.getKeyCode())
        {
            mappedPlayers[i]->shortcutKeyPressed();
        }
    }
    return false;
}