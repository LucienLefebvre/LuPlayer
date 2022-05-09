/*
  ==============================================================================

    Playlist.cpp
    Created: 10 Feb 2021 2:39:14pm
    Author:  Lucien

  ==============================================================================
*/

#include <JuceHeader.h>
#include "Playlist.h"

using namespace std;

//==============================================================================
Playlist::Playlist(int splaylistType, Settings* s)
{
    settings = s;
    playerNumber = 4;
    playlistType = splaylistType;

    juce::Timer::startTimer(100);
    setMouseClickGrabsKeyboardFocus(false);
    if (playlistType == 1)
    {
        handleFader3(127);
        handleFader3(0);
        handleFader4(127);
        handleFader4(0);
        fader1IsPlaying == false;
    }
    setSize(getParentWidth(), getParentHeight());

    playBroadcaster = new juce::ChangeBroadcaster();
    cuePlaylistBroadcaster = new juce::ChangeBroadcaster();
    cuePlaylistActionBroadcaster = new juce::ActionBroadcaster();
    fxButtonBroadcaster = new juce::ChangeBroadcaster();
    envButtonBroadcaster = new juce::ChangeBroadcaster();
    grabFocusBroadcaster = new juce::ChangeBroadcaster();
    playerLaunchedBroadcaster.reset(new juce::ChangeBroadcaster());

    minimumPlayer.addListener(this);
    fader1Value.addListener(this);
    rearrangePlayers();
    updateNextPlayer();
    mouseDragX = 0;
    mouseDragY = 0;
    mouseDragSource = -1 ;
    mouseDraggedUp = -1 ;
    draggedPlayer = -1;

    assignLeftFader(-1);
    assignLeftFader(0);
    assignRightFader(-1);
    assignRightFader(1);

    if (Settings::showMeter == false)
        meterWidth = 0;

    setMouseClickGrabsKeyboardFocus(false);
}

Playlist::~Playlist()
{
    delete playBroadcaster;
    delete cuePlaylistBroadcaster;
    delete cuePlaylistActionBroadcaster;
    delete fxButtonBroadcaster;
    delete envButtonBroadcaster;
    delete grabFocusBroadcaster;
    removeMouseListener(this);
    playlistMixer.removeAllInputs();
    playlistCueMixer.removeAllInputs();
    players.clear(true);
    for (auto i = 0; i < players.size(); i++)
        players.remove(i, true);
}

void Playlist::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    actualSampleRate = sampleRate;
    actualSamplesPerBlockExpected = samplesPerBlockExpected;

    for (auto* player : players)
    {
        player->playerPrepareToPlay(samplesPerBlockExpected, sampleRate);
    }
}

void Playlist::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill, const juce::AudioSourceChannelInfo& cueBuffer)
{
    for (auto player : players)
    {
        player->getNextAudioBlock(bufferToFill, cueBuffer);
    }
}

void Playlist::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    for (auto i = 1; i < playerNumber; i++)
    {
            g.setColour(juce::Colour(37, 45, 49));
            g.drawHorizontalLine((playersStartHeightPosition + (i * (playerHeight + spaceBetweenPlayer)) - 4), 0, getParentWidth());
            g.setColour(juce::Colours::black);
            g.drawHorizontalLine((playersStartHeightPosition + (i * (playerHeight + spaceBetweenPlayer)) - 3), 0, getParentWidth());
            g.setColour(juce::Colour(37, 45, 49));
            g.drawHorizontalLine((playersStartHeightPosition + (i * (playerHeight + spaceBetweenPlayer)) - 2), 0, getParentWidth());
    }
    if (playlistType == 0)
    {
        for (auto i = 0; i < players.size(); i++)
        {
            g.setColour(juce::Colour(40, 134, 189));
            g.fillRoundedRectangle(2, ( 10 + (105 * i)), 32, 83, 3);
        }
    }
    else if (playlistType == 1)
    {
        for (auto i = 0; i < players.size(); i++)
        {
            g.setColour(juce::Colour(40, 134, 189));
            if (eightPlayerMode)
            {
                if (!isEightPlayerSecondCart)
                    g.fillRoundedRectangle(2, (10 + (105 * i)), 32, 83, 3);
                else
                    g.fillRoundedRectangle(getParentWidth() - 36, (10 + (105 * i)), 32, 83, 3);
            }
            else
                g.fillRoundedRectangle(getParentWidth() - 36, (10 + (105 * i)), 32, 83, 3);
        }
    }

    if (dragPaintSourceRectangle == true)
    {

        g.setColour(juce::Colour(40, 134, 189));
        //Draw a rectangle surroundig the player :
        //two top lines
        g.drawHorizontalLine((playersStartHeightPosition + (fileDragPlayerSource * (playerHeight + spaceBetweenPlayer)) - 3), 0, getParentWidth());
        g.drawHorizontalLine((playersStartHeightPosition + (fileDragPlayerSource * (playerHeight + spaceBetweenPlayer)) - 2), 0, getParentWidth());
        //g.drawHorizontalLine((playersStartHeightPosition + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) - 1), 0, getParentWidth());
        //two bottoms lines
        g.drawHorizontalLine((playersStartHeightPosition + (playerHeight + spaceBetweenPlayer) + (fileDragPlayerSource * (playerHeight + spaceBetweenPlayer)) - 3), 0, getParentWidth());
        g.drawHorizontalLine((playersStartHeightPosition + (playerHeight + spaceBetweenPlayer) + (fileDragPlayerSource * (playerHeight + spaceBetweenPlayer)) - 2), 0, getParentWidth());
        //two lines at the left
        g.drawVerticalLine(0, playersStartHeightPosition + (fileDragPlayerSource * (playerHeight + spaceBetweenPlayer)) - 2,
            playersStartHeightPosition + (fileDragPlayerSource * (playerHeight + spaceBetweenPlayer)) + playerHeight + 2);
        g.drawVerticalLine(1, playersStartHeightPosition + (fileDragPlayerSource * (playerHeight + spaceBetweenPlayer)) - 2,
            playersStartHeightPosition + (fileDragPlayerSource * (playerHeight + spaceBetweenPlayer)) + playerHeight + 2);
        //two lines at the right
        g.drawVerticalLine(getParentWidth() - 1, playersStartHeightPosition + (fileDragPlayerSource * (playerHeight + spaceBetweenPlayer)) - 2,
            playersStartHeightPosition + (fileDragPlayerSource * (playerHeight + spaceBetweenPlayer)) + playerHeight + 2);
        g.drawVerticalLine(getParentWidth() - 2, playersStartHeightPosition + (fileDragPlayerSource * (playerHeight + spaceBetweenPlayer)) - 2,
            playersStartHeightPosition + (fileDragPlayerSource * (playerHeight + spaceBetweenPlayer)) + playerHeight + 2);


    }
    g.setColour(juce::Colours::red);

    if (fileDragPaintLine == true && !eightPlayerMode)
    {
        //Draw two lines at the insert position
        g.drawHorizontalLine((playersStartHeightPosition + (playerHeight + spaceBetweenPlayer) + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) - 4), 0, getParentWidth());
        g.drawHorizontalLine((playersStartHeightPosition + (playerHeight + spaceBetweenPlayer) + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) - 3), 0, getParentWidth());
        g.drawHorizontalLine((playersStartHeightPosition + (playerHeight + spaceBetweenPlayer) + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) - 2), 0, getParentWidth());
        g.drawHorizontalLine((playersStartHeightPosition + (playerHeight + spaceBetweenPlayer) + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) - 1), 0, getParentWidth());
    }
    else if (fileDragPaintRectangle == true && (fileDragPlayerSource != fileDragPlayerDestination))
    {
        //Draw a rectangle surroundig the player :
        //two top lines
        g.drawHorizontalLine((playersStartHeightPosition + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) - 3), 0, getParentWidth());
        g.drawHorizontalLine((playersStartHeightPosition + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) - 2), 0, getParentWidth());
        //g.drawHorizontalLine((playersStartHeightPosition + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) - 1), 0, getParentWidth());
        //two bottoms lines
        g.drawHorizontalLine((playersStartHeightPosition + (playerHeight + spaceBetweenPlayer) + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) - 3), 0, getParentWidth());
        g.drawHorizontalLine((playersStartHeightPosition + (playerHeight + spaceBetweenPlayer) + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) - 2), 0, getParentWidth());
        //two lines at the left
        g.drawVerticalLine(0, playersStartHeightPosition + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) - 2,
            playersStartHeightPosition + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) + playerHeight + 2);
        g.drawVerticalLine(1, playersStartHeightPosition + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) - 2,
            playersStartHeightPosition + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) + playerHeight + 2);
        //two lines at the right
        g.drawVerticalLine(getParentWidth() - 1, playersStartHeightPosition + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) - 2,
            playersStartHeightPosition + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) + playerHeight + 2);
        g.drawVerticalLine(getParentWidth() - 2, playersStartHeightPosition + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) - 2,
            playersStartHeightPosition + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) + playerHeight + 2);
        if (players[fileDragPlayerDestination] != nullptr)
            g.fillRect(players[fileDragPlayerDestination]->getBounds());
    }
}

void Playlist::resized()
{
    rearrangePlayers();
}

bool Playlist::handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message, MidiMapper* mapper)
{
    int midiMessageValue = message.getControllerValue();
    int midiMessageNumber = message.getControllerNumber();
    if (playlistType == 0)
    {
        if (midiMessageNumber == mapper->getMidiCCForCommand(MidiMapper::MidiCommands::Fader1Level))
        {
            handleFader1(midiMessageValue);
            return true;
        }
        else if (midiMessageNumber == mapper->getMidiCCForCommand(MidiMapper::MidiCommands::Fader2Level))
        {
            handleFader2(midiMessageValue);
            return true;
        }
        else if (handleMidiTrim(midiMessageValue, midiMessageNumber, mapper))
            return true;
        else if (handleMidiRelativeTrim(midiMessageValue, midiMessageNumber, mapper))
            return true;
        else
            return false;
    }
    else if (playlistType == 1)
    {
        if (midiMessageNumber == mapper->getMidiCCForCommand(MidiMapper::MidiCommands::Fader3Level))
        {
            handleFader3(midiMessageValue);
            return true;
        }
        else if (midiMessageNumber == mapper->getMidiCCForCommand(MidiMapper::MidiCommands::Fader4Level))
        {
            handleFader4(midiMessageValue);
            return true;
        }
        else if (handleMidiTrim(midiMessageValue, midiMessageNumber, mapper))
            return true;
        else if (handleMidiRelativeTrim(midiMessageValue, midiMessageNumber, mapper))
            return true;
        else
            return false;
    }

    const juce::ScopedValueSetter<bool> scopedInputFlag(isAddingFromMidiInput, true);
}

bool Playlist::handleMidiTrim(int value, int number, MidiMapper* mapper)
{
    if (playlistType == 0)
    {
        if (number == mapper->getMidiCCForCommand(MidiMapper::MidiCommands::Fader1Trim) && players[fader1Player] != nullptr)
        {
            players[fader1Player]->handleMidiTrimMessage(value);
            return true;
        }
        else if (number == mapper->getMidiCCForCommand(MidiMapper::MidiCommands::Fader2Trim) && players[fader2Player] != nullptr)
        {
            players[fader2Player]->handleMidiTrimMessage(value);
            return true;
        }
        else
            return false;
    }
    else if (playlistType == 1)
    {
        if (number == mapper->getMidiCCForCommand(MidiMapper::MidiCommands::Fader3Trim) && players[fader1Player] != nullptr)
        {
            players[fader1Player]->handleMidiTrimMessage(value);
            return true;
        }
        else if (number == mapper->getMidiCCForCommand(MidiMapper::MidiCommands::Fader4Trim) && players[fader2Player] != nullptr)
        {
            players[fader2Player]->handleMidiTrimMessage(value);
            return true;
        }
        else
            return false;
    }
    else
        return false;
}

bool Playlist::handleMidiRelativeTrim(int value, int number, MidiMapper* mapper)
{
    if (playlistType == 0)
    {
        if (number == mapper->getMidiCCForCommand(MidiMapper::MidiCommands::Fader1TrimR) && players[fader1Player] != nullptr)
        {
            if (value == 127)
                players[fader1Player]->handleMidiTrimMessage(true);
            else
                players[fader1Player]->handleMidiTrimMessage(false);
            return true;
        }

        else if (number == mapper->getMidiCCForCommand(MidiMapper::MidiCommands::Fader2TrimR) && players[fader2Player] != nullptr)
        {
            if (value == 127)
                players[fader2Player]->handleMidiTrimMessage(true);
            else
                players[fader2Player]->handleMidiTrimMessage(false);
            return true;
        }
        else
            return false;
    }
    else if (playlistType == 1)
    {
        if (number == mapper->getMidiCCForCommand(MidiMapper::MidiCommands::Fader3TrimR) && players[fader1Player] != nullptr)
        {
            if (value == 127)
                players[fader1Player]->handleMidiTrimMessage(true);
            else
                players[fader1Player]->handleMidiTrimMessage(false);
            return true;
        }
        else if (number == mapper->getMidiCCForCommand(MidiMapper::MidiCommands::Fader4TrimR) && players[fader2Player] != nullptr)
        {
            if (value == 127)
                players[fader2Player]->handleMidiTrimMessage(true);
            else
                players[fader2Player]->handleMidiTrimMessage(false);
            return true;
        }
        else
            return false;
    }
    else
        return false;
}

void Playlist::handleIncomingMidiMessageEightPlayers(juce::MidiInput* source, const juce::MidiMessage& message, MidiMapper* mapper, int startIndex)
{
    int midiMessageValue = message.getControllerValue();
    int midiMessageNumber = message.getControllerNumber();

    Player* destinationPlayer = nullptr;
    for (int i = 0; i < 4; i++)
    {
        if (midiMessageNumber == mapper->getMidiCCForCommand(i + startIndex))
        {
            if (players[i] != nullptr)
                destinationPlayer = players[i];
        }
    }

    actualMidiLevels[midiMessageNumber] = midiMessageValue;

    if (destinationPlayer != nullptr)
    {
        if (destinationPlayer->fileLoaded == true)
        {

            if (actualMidiLevels[midiMessageNumber] >= 1 && previousMidiLevels[midiMessageNumber] == 0 && Settings::enableFaderStart)
            {
                destinationPlayer->transport.setGain(0.0);
                destinationPlayer->play();
            }
            else if (actualMidiLevels[midiMessageNumber] == 0 && previousMidiLevels[midiMessageNumber] >= 1 && Settings::enableFaderStart)
            {
                destinationPlayer->transport.stop();
            }
        }
        destinationPlayer->handleMidiMessage(0, actualMidiLevels[midiMessageNumber]);
    }

    previousMidiLevels[midiMessageNumber] = actualMidiLevels[midiMessageNumber];
    const juce::ScopedValueSetter<bool> scopedInputFlag(isAddingFromMidiInput, true);
}

void Playlist::fader1Start()
{
    juce::FileLogger::getCurrentLogger()->writeToLog("fader 1 start");
    players[fader1Player]->play(true);
    players[fader1Player]->fader1IsPlaying = true;
    fader1IsPlaying = true;
    fader1Stopped = false;
    fader1StartPlayer = fader1Player;
    updateButtonsStates();
    if (fader2IsPlaying == false)
        fader2Player++;
    fader1StartTime = juce::Time::currentTimeMillis();
    updateNextPlayer();
}
void Playlist::fader1Stop(bool stoppedByFader)
{
    juce::FileLogger::getCurrentLogger()->writeToLog("fader 1 stop");
    players[fader1Player]->stopButtonClicked();
    players[fader1Player]->fader1IsPlaying = false;

    fader1IsPlaying = false;
    fader1Stopped = true;

    updateButtonsStates();
    fader1PreviousMidiLevel = fader1ActualMidiLevel;
    fader1StopTime = juce::Time::currentTimeMillis();
    if (fader1StopTime - fader1StartTime >= Settings::faderTempTime)
    {
        if (fader2IsPlaying == false)
        {
            fader2Player = max(fader1Player, fader2Player);
            fader1Player = max(fader1Player, fader2Player);
        }
        else if (fader2IsPlaying == true)
        {
            fader1Player = max(fader1Player, fader2Player) + 1;
        }
    }
    else if (fader2IsPlaying == false)
    {
        fader2Player--;
        if (fader1StopTime - fader2StartTime < Settings::faderTempTime)
        {
            fader2Player = max(fader2Player, fader1Player);
        }
    }
    updateNextPlayer();
}
void Playlist::fader2Start()
{
    juce::FileLogger::getCurrentLogger()->writeToLog("fader 2 start");
    players[fader2Player]->transport.setGain(0.0);
    players[fader2Player]->play();
    players[fader2Player]->fader2IsPlaying = true;
    fader2IsPlaying = true;
    fader2StartPlayer = fader2Player;
    updateButtonsStates();
    fader2Stopped = false;
    fader2StartTime = juce::Time::currentTimeMillis();
    if (fader1IsPlaying == false)
        fader1Player++;
    updateNextPlayer();
}
void Playlist::fader2Stop(bool stoppedByFader)
{
    juce::FileLogger::getCurrentLogger()->writeToLog("fader 2 stop");
    players[fader2Player]->stopButtonClicked();
    players[fader2Player]->fader2IsPlaying = false;
    fader2IsPlaying = false;
    updateButtonsStates();
    fader2PreviousMidiLevel = fader2ActualMidiLevel;
    fader2StopTime = juce::Time::currentTimeMillis();
    if (fader2StopTime - fader2StartTime >= Settings::faderTempTime)
    {
        if (fader1IsPlaying == false)
        {
            fader1Player = max(fader2Player, fader1Player);
            fader2Player = max(fader2Player, fader1Player);
        }
        else if (fader1IsPlaying == true)
        {
            fader2Player = max(fader2Player, fader1Player) + 1;
        }
    }
    else if (fader1IsPlaying == false)
    {
        fader1Player--;
        if (fader2StopTime - fader1StartTime < Settings::faderTempTime)
        {
            fader1Player = max(fader2Player, fader1Player);
        }
    }
    updateNextPlayer();
}

void Playlist::handleMidiTrimEightPlayers(int value, int number, MidiMapper* mapper, int startIndex)
{
    for (int i = 0; i < 4; i++)
    {
        if (number == mapper->getMidiCCForCommand(startIndex + i) && players[i] != nullptr)
        {
            players[i]->handleMidiTrimMessage(value);
            return;
        }
    }
}


void Playlist::handleFader1(int faderValue)
{
    fader1ActualMidiLevel = faderValue;
    if (spaceBarIsPlaying == false)
    {
        if (players[fader1Player] != nullptr)
        {
            if (players[fader1Player]->fileLoaded != 0) //if there is a loaded file
            {

                if (fader1IsPlaying == false) //if it is not playing
                {
                    if (fader1ActualMidiLevel >= 1 && fader1PreviousMidiLevel == 0 && Settings::enableFaderStart) //fader start
                    {
                        fader1Start();
                    }
                }
                if (fader1IsPlaying == true) //if it is playing
                {
                    if (fader1ActualMidiLevel == 0 && fader1PreviousMidiLevel >= 1 && Settings::enableFaderStart) //fader stop
                    {
                        players[fader1Player]->transport.stop();
                        //fader1IsFlying = false;
                    }
                }
            }
            fader1PreviousMidiLevel = fader1ActualMidiLevel;
            if (players[fader1Player] != nullptr && fader1IsPlaying)
                players[fader1Player]->handleMidiMessage(0, faderValue);
        }
        else
            fader1PreviousMidiLevel = fader1ActualMidiLevel;
    }
}

void Playlist::handleFader1OSC(float faderValue)
{
    fader1ActualMidiLevel = juce::int32(faderValue*127);
    if (spaceBarIsPlaying == false)
    {
        if (players[fader1Player] != nullptr)
        {
            if (players[fader1Player]->fileLoaded != 0) //if there is a loaded file
            {
                if (fader1IsPlaying == false) //if it is not playing
                {
                    if (fader1ActualMidiLevel >= 1 && fader1PreviousMidiLevel == 0 && Settings::enableFaderStart) //fader start
                    {
                        fader1Start();
                    }
                }
                if (fader1IsPlaying == true) //if it is playing
                {
                    if (fader1ActualMidiLevel == 0 && fader1PreviousMidiLevel >= 1 && Settings::enableFaderStart) //fader stop
                    {
                        players[fader1Player]->transport.stop();
                    }
                }
            }
            fader1PreviousMidiLevel = fader1ActualMidiLevel;
            if (players[fader1Player] != nullptr)
                players[fader1Player]->handleOSCMessage(faderValue);

        }
        else
            fader1PreviousMidiLevel = fader1ActualMidiLevel;
    }
}

void Playlist::handleFader2(int faderValue)
{
    if (spaceBarIsPlaying == false)
    {
        fader2ActualMidiLevel = faderValue;
        if (players[fader2Player] != nullptr)
        {
            if (players[fader2Player]->fileLoaded != 0) //if there is a loaded file
            {
                if (fader2IsPlaying == false) //if it is not playing
                {
                    if (fader2ActualMidiLevel >= 1 && fader2PreviousMidiLevel == 0 && Settings::enableFaderStart) //fader start
                    {
                        fader2Start();
                    }
                }
                if (fader2IsPlaying == true) //if it is playing
                {
                    if (fader2ActualMidiLevel == 0 && fader2PreviousMidiLevel >= 1 && Settings::enableFaderStart) //fader stop
                    {
                        players[fader2Player]->transport.stop();
                        //fader2Stopped = false;
                    }
                }
            }
            fader2PreviousMidiLevel = fader2ActualMidiLevel;
            if (players[fader2Player] != nullptr && fader2IsPlaying)
                players[fader2Player]->handleMidiMessage(0, faderValue);
        }
        else
            fader2PreviousMidiLevel = fader2ActualMidiLevel;
    }
}

void Playlist::handleFader2OSC(float faderValue)
{
    fader2ActualMidiLevel = juce::int32(faderValue * 127);
    if (spaceBarIsPlaying == false)
    {
        if (players[fader2Player] != nullptr)
        {
            if (players[fader2Player]->fileLoaded != 0) //if there is a loaded file
            {
                if (fader2IsPlaying == false) //if it is not playing
                {
                    if (fader2ActualMidiLevel >= 1 && fader2PreviousMidiLevel == 0 && Settings::enableFaderStart) //fader start
                    {
                        fader2Start();
                    }
                }
                if (fader2IsPlaying == true) //if it is playing
                {
                    if (fader2ActualMidiLevel == 0 && fader2PreviousMidiLevel >= 1 && Settings::enableFaderStart) //fader stop
                    {
                        players[fader2Player]->transport.stop();
                    }

                }

            }
            fader2PreviousMidiLevel = fader2ActualMidiLevel;
            if (players[fader2Player] != nullptr)
                players[fader2Player]->handleOSCMessage(faderValue);
        }
        else
            fader2PreviousMidiLevel = fader2ActualMidiLevel;
    }
}

void Playlist::handleFader3(int faderValue)
{
    fader1ActualMidiLevel = faderValue;
        if (players[fader1Player] != nullptr)
        {
            if (players[fader1Player]->fileLoaded != 0) //if there is a loaded file
            {
                if (fader1IsPlaying == false) //if it is not playing
                {
                    if (fader1ActualMidiLevel >= 1 && fader1PreviousMidiLevel == 0 && Settings::enableFaderStart) //fader start
                    {
                            players[fader1Player]->play();
                            players[fader1Player]->fader1IsPlaying = true;
                            fader1IsPlaying = true;
                            updateButtonsStates();
                    }
                }
                if (fader1IsPlaying == true) //if it is playing
                {
                    if (fader1ActualMidiLevel == 0 && fader1PreviousMidiLevel >= 1 && Settings::enableFaderStart) //fader stop
                    {
                        players[fader1Player]->stopButtonClicked();
                        players[fader1Player]->fader1IsPlaying = false;
                        fader1IsPlaying = false;
                        updateButtonsStates();
                    }

                }

            }
            fader1PreviousMidiLevel = fader1ActualMidiLevel;
            if (players[fader1Player] != nullptr)
                players[fader1Player]->handleMidiMessage(0, faderValue);
        }
        else
            fader1PreviousMidiLevel = fader1ActualMidiLevel;
}

void Playlist::handleFader3OSC(float faderValue)
{
    fader1ActualMidiLevel = juce::int32(faderValue * 127);
    if (players[fader1Player] != nullptr)
    {
        if (players[fader1Player]->fileLoaded != 0) //if there is a loaded file
        {
            if (fader1IsPlaying == false) //if it is not playing
            {
                if (fader1ActualMidiLevel >= 1 && fader1PreviousMidiLevel == 0 && Settings::enableFaderStart) //fader start
                {
                    players[fader1Player]->play();
                    players[fader1Player]->fader1IsPlaying = true;
                    fader1IsPlaying = true;
                    fader1PreviousMidiLevel = fader1ActualMidiLevel;
                }
            }
            if (fader1IsPlaying == true) //if it is playing
            {
                if (fader1ActualMidiLevel == 0 && fader1PreviousMidiLevel >= 1 && Settings::enableFaderStart) //fader stop
                {
                    players[fader1Player]->stopButtonClicked();
                    players[fader1Player]->fader1IsPlaying = false;
                    fader1IsPlaying = false;
                    fader1PreviousMidiLevel = fader1ActualMidiLevel;
                }

            }

        }
        fader1PreviousMidiLevel = fader1ActualMidiLevel;
        if (players[fader1Player] != nullptr)
            players[fader1Player]->handleOSCMessage(faderValue);
    }
}

void Playlist::handleFader4(int faderValue)
{
    fader2ActualMidiLevel = faderValue;
    if (players[fader2Player] != nullptr)
    {
        if (players[fader2Player]->fileLoaded != 0) //if there is a loaded file
        {
            if (fader2IsPlaying == false) //if it is not playing
            {
                if (fader2ActualMidiLevel >= 1 && fader2PreviousMidiLevel == 0 && Settings::enableFaderStart) //fader start
                {
                    players[fader2Player]->play();
                    players[fader2Player]->fader2IsPlaying = true;
                    fader2IsPlaying = true;
                    fader2PreviousMidiLevel = fader2ActualMidiLevel;
                    updateButtonsStates();
                }
            }
            if (fader2IsPlaying == true) //if it is playing
            {
                if (fader2ActualMidiLevel == 0 && fader2PreviousMidiLevel >= 1 && Settings::enableFaderStart) //fader stop
                {
                    players[fader2Player]->stopButtonClicked();
                    players[fader2Player]->fader2IsPlaying = false;
                    fader2IsPlaying = false;
                    fader2PreviousMidiLevel = fader2ActualMidiLevel;
                    updateButtonsStates();
                }

            }

        }
        fader2PreviousMidiLevel = fader2ActualMidiLevel;
        if (players[fader2Player] != nullptr)
            players[fader2Player]->handleMidiMessage(0, faderValue);
    }
    else
        fader2PreviousMidiLevel = fader2ActualMidiLevel;
}

void Playlist::handleFader4OSC(float faderValue)
{
    fader2ActualMidiLevel = juce::int32(faderValue * 127);
    if (players[fader2Player] != nullptr)
    {
        if (players[fader2Player]->fileLoaded != 0) //if there is a loaded file
        {
            if (fader2IsPlaying == false) //if it is not playing
            {
                if (fader2ActualMidiLevel >= 1 && fader2PreviousMidiLevel == 0 && Settings::enableFaderStart) //fader start
                {
                    players[fader2Player]->play();
                    players[fader2Player]->fader2IsPlaying = true;
                    fader2IsPlaying = true;
                    fader2PreviousMidiLevel = fader2ActualMidiLevel;

                }
            }
            if (fader2IsPlaying == true) //if it is playing
            {
                if (fader2ActualMidiLevel == 0 && fader2PreviousMidiLevel >= 1 && Settings::enableFaderStart) //fader stop
                {
                    players[fader2Player]->stopButtonClicked();
                    players[fader2Player]->fader2IsPlaying = false;
                    fader2IsPlaying = false;
                    fader2PreviousMidiLevel = fader2ActualMidiLevel;
                }

            }

        }
        fader2PreviousMidiLevel = fader2ActualMidiLevel;
        if (players[fader2Player] != nullptr)
            players[fader2Player]->handleOSCMessage(faderValue);
    }
}

void Playlist::updateNextPlayer()
{
    if (players[fader1Player] != nullptr)
        fader1Name.setValue(players[fader1Player]->soundName.getText());
    if (players[fader2Player] != nullptr)
        fader2Name.setValue(players[fader2Player]->soundName.getText());

    minimumPlayer = min(fader1Player, fader2Player);
    if (playlistType == 0)
    {
        if (fader1Player == fader2Player)
            nextPlayer = fader1Player;
        else if (fader1IsPlaying && fader2IsPlaying)
            nextPlayer = -1;
        else if (fader1IsPlaying || fader2IsPlaying)
            nextPlayer = max(fader1Player, fader2Player);

        for (auto i = 0; i < playerNumber; i++)
        {
            if (players[i] != nullptr)
            {
                if (nextPlayer == i)
                    players[i]->setNextPlayer(true);
                else if (nextPlayer != i)
                    players[i]->setNextPlayer(false);
                if (playlistType == 0)
                {
                    const juce::MessageManagerLock mmLock;
                    if (i == fader1Player)
                        players[i]->setLeftFaderAssigned(true);
                    else
                        players[i]->setLeftFaderAssigned(false);
                    if (i == fader2Player)
                        players[i]->setRightFaderAssigned(true);
                    else
                        players[i]->setRightFaderAssigned(false);
                }
            }
        }
        spaceBarPlayerId = nextPlayer;
    }
    const juce::MessageManagerLock mmLock;
    repaint();
}

void Playlist::playersResetPositionClicked()
{
    if (!spaceBarIsPlaying)
    {
        if ((fader1IsPlaying == 0) && (fader2IsPlaying == 0))
        {
            fader1Player = 0;
            fader2Player = 0;
            updateNextPlayer();
        }
    }
}

void Playlist::playersPreviousPositionClicked()
{
    if (!spaceBarIsPlaying)
    {
        if ((fader1IsPlaying == 0) && (fader2IsPlaying == 0))
        {
            nextPlayer = (min(fader1Player, fader2Player) - 1);
            fader1Player = fader2Player = nextPlayer;
        }
        else if ((fader2IsPlaying == 0) && (fader2Player > (fader1Player + 1)))
        {
            nextPlayer = fader2Player - 1;
            fader2Player = nextPlayer;
        }
        else if ((fader1IsPlaying == 0) && (fader1Player > (fader2Player + 1)))
        {
            nextPlayer = fader1Player - 1;
            fader1Player = nextPlayer;
        }

        if (fader1Player < 0)
            fader1Player = 0;
        if (fader2Player < 0)
            fader2Player = 0;

        updateNextPlayer();
    }
}

void Playlist::playersNextPositionClicked()
{
    if (!spaceBarIsPlaying)
    {
        if (nextPlayer < playerNumber)
            nextPlayer++;
        if ((fader1IsPlaying == 0) && (fader2IsPlaying == 0))
        {
            fader1Player = nextPlayer;
            fader2Player = nextPlayer;
        }
        else if (fader2IsPlaying == 0)
            fader2Player = nextPlayer;
        else if (fader1IsPlaying == 0)
            fader1Player = nextPlayer;

        if (fader1Player == players.size())
            fader1Player = (players.size() - 1);
        if (fader2Player == players.size())
            fader2Player = (players.size() - 1);
        setOptions();
        updateNextPlayer();
    }
}

void Playlist::addPlayer(int playerID)
{
    juce::FileLogger::getCurrentLogger()->writeToLog("Add player");
    int idAddedPlayer = playerID + 1;
    players.insert(idAddedPlayer, new Player(idAddedPlayer, settings));
    if (players[idAddedPlayer] != nullptr)
    {
        if (eightPlayerMode)
            players.getLast()->setEightPlayerMode(true);
        if (isEightPlayerSecondCart)
            players.getLast()->setOSCIndex(idAddedPlayer + 5);
        else
            players.getLast()->setOSCIndex(idAddedPlayer + 1);
        players[idAddedPlayer]->playerPrepareToPlay(actualSamplesPerBlockExpected, actualSampleRate);
        players[idAddedPlayer]->fileName.addListener(this);
        players[idAddedPlayer]->playerPositionLabel.addListener(this);
        players[idAddedPlayer]->draggedPlayer.addListener(this);
        players[idAddedPlayer]->cueStopped.addListener(this);
        players[idAddedPlayer]->transport.addChangeListener(this);
        players[idAddedPlayer]->cueTransport.addChangeListener(this);
        players[idAddedPlayer]->cueBroadcaster->addActionListener(this);
        players[idAddedPlayer]->playBroadcaster->addActionListener(this);
        players[idAddedPlayer]->fxButtonBroadcaster->addChangeListener(this);
        players[idAddedPlayer]->envButtonBroadcaster->addChangeListener(this);
        players[idAddedPlayer]->playerLaunchedBroadcaster->addChangeListener(this);

        playersPositionLabels.insert(idAddedPlayer, new juce::Label);

        meters.insert(idAddedPlayer, new Meter(Meter::Mode::Stereo));
        meters[idAddedPlayer]->prepareToPlay(actualSamplesPerBlockExpected, actualSampleRate);
        meters[idAddedPlayer]->shouldDrawScaleNumbers(false);
        meters[idAddedPlayer]->setMeterColour(juce::Colour(229, 149, 0));
        meters[idAddedPlayer]->setPeakColour(juce::Colours::red);
        meters[idAddedPlayer]->shouldDrawScale(true);
        meters[idAddedPlayer]->setRectangleRoundSize(2);
        meters[idAddedPlayer]->setMouseClickGrabsKeyboardFocus(false);

        playersPositionLabels[idAddedPlayer]->setText(juce::String(idAddedPlayer + 1), juce::NotificationType::dontSendNotification);
        addAndMakeVisible(playersPositionLabels[idAddedPlayer]);
        playersPositionLabels[idAddedPlayer]->setInterceptsMouseClicks(false, false);

        if (playlistType == 0)
        {
            removePlayersButtons.insert(idAddedPlayer, new juce::TextButton());
            removePlayersButtons[idAddedPlayer]->setButtonText("-");
            removePlayersButtons[idAddedPlayer]->onClick = [idAddedPlayer, this] { removePlayer(idAddedPlayer); };
            removePlayersButtons[idAddedPlayer]->setMouseClickGrabsKeyboardFocus(false);

            addPlayersButtons.insert(idAddedPlayer, new juce::TextButton());
            addPlayersButtons[idAddedPlayer]->setButtonText("+");
            addPlayersButtons[idAddedPlayer]->onClick = [idAddedPlayer, this] { addPlayer(idAddedPlayer); };
            addPlayersButtons[idAddedPlayer]->setMouseClickGrabsKeyboardFocus(false);

            playersPositionLabels[idAddedPlayer]->setBounds(2, (105 * idAddedPlayer) + 30, 40, 40);
            playersPositionLabels[idAddedPlayer]->setFont(juce::Font(40.00f, juce::Font::plain).withTypefaceStyle("Regular"));
            playersPositionLabels[idAddedPlayer]->setMouseClickGrabsKeyboardFocus(false);
        }
        else if (playlistType == 1)
        {
            assignLeftFaderButtons.insert(idAddedPlayer, new juce::TextButton());
            assignLeftFaderButtons[idAddedPlayer]->setButtonText("");
            assignLeftFaderButtons[idAddedPlayer]->onClick = [idAddedPlayer, this] { assignLeftFader(idAddedPlayer); };
            assignLeftFaderButtons[idAddedPlayer]->setMouseClickGrabsKeyboardFocus(false);

            assignRightFaderButtons.insert(idAddedPlayer, new juce::TextButton());
            assignRightFaderButtons[idAddedPlayer]->setButtonText("");
            assignRightFaderButtons[idAddedPlayer]->onClick = [idAddedPlayer, this] { assignRightFader(idAddedPlayer); };
            assignRightFaderButtons[idAddedPlayer]->setMouseClickGrabsKeyboardFocus(false);

            playersPositionLabels[idAddedPlayer]->setBounds(getParentWidth() - 36, (105 * idAddedPlayer) + 30, 40, 40);
            playersPositionLabels[idAddedPlayer]->setFont(juce::Font(40.00f, juce::Font::plain).withTypefaceStyle("Regular"));

            removePlayersButtons.insert(idAddedPlayer, new juce::TextButton());
            removePlayersButtons[idAddedPlayer]->setButtonText("-");
            removePlayersButtons[idAddedPlayer]->onClick = [idAddedPlayer, this] { removePlayer(idAddedPlayer); };
            removePlayersButtons[idAddedPlayer]->setMouseClickGrabsKeyboardFocus(false);

            addPlayersButtons.insert(idAddedPlayer, new juce::TextButton());
            addPlayersButtons[idAddedPlayer]->setButtonText("+");
            addPlayersButtons[idAddedPlayer]->onClick = [idAddedPlayer, this] { addPlayer(idAddedPlayer); };
            addPlayersButtons[idAddedPlayer]->setMouseClickGrabsKeyboardFocus(false);
        }
        playerNumber = players.size();
        rearrangePlayers();
        updateNextPlayer();
        setOptions();
        repaint();
    }
}

void Playlist::removePlayer(int playerID)
{
    juce::FileLogger::getCurrentLogger()->writeToLog("remove player");
    if (playlistType == 0)
    {
        if (players.size() > 1)
        {
            if (players[playerID] != nullptr)
            {
                if (!players[playerID]->isThreadRunning())
                {
                    players[playerID]->stopButtonClicked();
                    playlistMixer.removeInputSource(players[playerID]->outputSource.get());
                    playlistCueMixer.removeInputSource(players[playerID]->cueOutputSource.get());
                    players[playerID]->fileName.removeListener(this);
                    players[playerID]->playerPositionLabel.addListener(this);
                    players[playerID]->cueStopped.addListener(this);
                    players[playerID]->cueBroadcaster->removeActionListener(this);
                    players[playerID]->fxButtonBroadcaster->removeChangeListener(this);
                    players[playerID]->envButtonBroadcaster->removeChangeListener(this);
                    players[playerID]->killThreads();

                    players.remove(playerID);
                    playersPositionLabels.remove(playerID);

                    removePlayersButtons.remove(playerID);
                    addPlayersButtons.remove(playerID);
                    playerNumber = players.size();
                    meters.remove(playerID);
                    updateNextPlayer();
                    rearrangePlayers();
                }
            }
        }
    }
    else if (playlistType == 1)
    {
        if (players.size() > 1)
        {
            if (players[playerID] != nullptr)
            {
                if (!players[playerID]->isThreadRunning())
                {
                    players[playerID]->stopButtonClicked();
                    playlistMixer.removeInputSource(players[playerID]->outputSource.get());
                    playlistCueMixer.removeInputSource(players[playerID]->cueOutputSource.get());
                    players[playerID]->fileName.removeListener(this);
                    players[playerID]->playerPositionLabel.addListener(this);
                    players[playerID]->cueStopped.addListener(this);
                    players[playerID]->cueBroadcaster->removeActionListener(this);
                    players[playerID]->fxButtonBroadcaster->removeChangeListener(this);
                    players[playerID]->envButtonBroadcaster->removeChangeListener(this);
                    players[playerID]->killThreads();
                    players.remove(playerID);
                    playersPositionLabels.remove(playerID);
                    assignLeftFaderButtons.remove(playerID);
                    assignRightFaderButtons.remove(playerID);
                    meters.remove(playerID);
                    playerNumber = players.size();
                    updateNextPlayer();
                    rearrangePlayers();
                }
            }
        }
    }
    grabFocusBroadcaster->sendChangeMessage();
}

void Playlist::swapNext(int playerID)
{
    players.swap(playerID, playerID + 1);
    updateNextPlayer();
    rearrangePlayers();
}

void Playlist::assignLeftFader(int playerID)
{
    int previousfader1Player = fader1Player;
    if (players[playerID] != nullptr)
    {
        if (fader1IsPlaying == false)
        {
            if (fader2Player != playerID)
            {
                fader1Player = playerID;
                players[playerID]->setLeftFaderAssigned(true);
            }
            else if (fader2Player == playerID)
            {
                if (fader2IsPlaying)
                {
                    fader1Player = previousfader1Player;
                    players[playerID]->setLeftFaderAssigned(true);
                }
                else if (!fader2IsPlaying)
                {
                    fader1Player = playerID;
                    fader2Player = -1;
                    assignRightFaderButtons[playerID]->setColour(juce::TextButton::ColourIds::buttonColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
                    players[playerID]->setRightFaderAssigned(false);
                    players[playerID]->setLeftFaderAssigned(true);
                }
            }
        }
        for (auto i = 0; i < players.size(); i++)
        {
            if (i != fader1Player)
            {
                assignLeftFaderButtons[i]->setColour(juce::TextButton::ColourIds::buttonColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
            }
            else
                assignLeftFaderButtons[i]->setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::red);
        }
    }
    if (players[fader1Player] != nullptr)
        fader1Name.setValue(players[fader1Player]->soundName.getText());
    else
        fader1Name.setValue("");
}

void Playlist::assignRightFader(int playerID)
{
    int previousfader2Player = fader2Player;
    if (players[playerID] != nullptr)
    {
        if (fader2IsPlaying == false)
        {
            if (fader1Player != playerID)
            {
                fader2Player = playerID;
                players[playerID]->setRightFaderAssigned(true);
            }
            else if (fader1Player == playerID)
            {
                if (fader1IsPlaying)
                {
                    fader2Player = previousfader2Player;
                    players[playerID]->setRightFaderAssigned(true);
                }
                else if (!fader1IsPlaying)
                {
                    fader2Player = playerID;
                    fader1Player = -1;
                    assignLeftFaderButtons[playerID]->setColour(juce::TextButton::ColourIds::buttonColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
                    players[playerID]->setLeftFaderAssigned(false);
                    players[playerID]->setRightFaderAssigned(true);
                }
            }
        }
        for (auto i = 0; i < players.size(); i++)
        {
            if (i != fader2Player)
            {
                assignRightFaderButtons[i]->setColour(juce::TextButton::ColourIds::buttonColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
            }
            else
                assignRightFaderButtons[i]->setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::red);
        }

    }
    if (players[fader2Player] != nullptr)
        fader2Name.setValue(players[fader2Player]->soundName.getText());
    else
        fader2Name.setValue("");
}

void Playlist::rearrangePlayers()
{
    playerNumber = players.size();

    for (auto i = 0; i < players.size() + 1; i++)
    {

        if (players[i] != nullptr && playersPositionLabels[i] != nullptr)
        {
            players[i]->setPlayerIndex(i);
            playersPositionLabels[i]->setText(juce::String(i + 1), juce::NotificationType::dontSendNotification);
            if (isEightPlayerSecondCart)
                playersPositionLabels[i]->setText(juce::String(i + 5), juce::NotificationType::dontSendNotification);
            if (i > 8)
                playersPositionLabels[i]->setFont(juce::Font(30.00f, juce::Font::plain).withTypefaceStyle("Regular"));

            // Player
            addAndMakeVisible(players[i]);
            addAndMakeVisible(meters[i]);
            if (playlistType == 1)
            {
                if (eightPlayerMode)
                {
                    if (!isEightPlayerSecondCart)
                        playersPositionLabels[i]->setBounds(2, (105 * i) + 30, 40, 40);
                    else
                        playersPositionLabels[i]->setBounds(getParentWidth() - 36, (105 * i) + 30, 40, 40);
                }
                else
                    playersPositionLabels[i]->setBounds(getParentWidth() - 36, (105 * i) + 30, 40, 40);
                if (i > 8)
                    playersPositionLabels[i]->setBounds(getParentWidth() - 41, (105 * i) + 30, 40, 40);

                if (!eightPlayerMode)
                {
                    players[i]->setBounds(assignFaderButtonWidth, playersStartHeightPosition + (i * (playerHeight + spaceBetweenPlayer)), getParentWidth() - (assignFaderButtonWidth * 2) - dragZoneWidth - controlButtonWidth - 3 - meterWidth, playerHeight);
                    meters[i]->setBounds(players[i]->getRight(), playersStartHeightPosition + (i * (playerHeight + spaceBetweenPlayer)), meterWidth, playerHeight);
                }
                else
                {
                    players[i]->setBounds(0, playersStartHeightPosition + (i * (playerHeight + spaceBetweenPlayer)), getWidth() - dragZoneWidth - 3 - meterWidth, playerHeight);
                    meters[i]->setBounds(players[i]->getRight(), playersStartHeightPosition + (i * (playerHeight + spaceBetweenPlayer)), meterWidth, playerHeight);
                    if (!isEightPlayerSecondCart)
                    {
                        players[i]->setBounds(3 + dragZoneWidth + meterWidth, playersStartHeightPosition + (i * (playerHeight + spaceBetweenPlayer)), getWidth() - 3 - meterWidth - dragZoneWidth, playerHeight);
                        meters[i]->setBounds(dragZoneWidth, playersStartHeightPosition + (i * (playerHeight + spaceBetweenPlayer)), meterWidth, playerHeight);
                    }
                }
                players[i]->setIsCart(true);
                players[i]->resized();

                if (!eightPlayerMode)
                {
                    addAndMakeVisible(assignLeftFaderButtons[i]);
                    assignLeftFaderButtons[i]->setBounds(1, 33 + playersStartHeightPosition + (i * (playerHeight + spaceBetweenPlayer)), assignFaderButtonWidth - 1, controlButtonHeight + spaceBetweenPlayer + 2);
                    assignLeftFaderButtons[i]->setButtonText("");
                    assignLeftFaderButtons[i]->onClick = [i, this] { assignLeftFader(i); grabFocusBroadcaster->sendChangeMessage(); };
                    assignLeftFaderButtons[i]->setMouseClickGrabsKeyboardFocus(false);

                    addAndMakeVisible(assignRightFaderButtons[i]);
                    assignRightFaderButtons[i]->setBounds(getParentWidth() - assignFaderButtonWidth - dragZoneWidth, 33 + playersStartHeightPosition + (i * (playerHeight + spaceBetweenPlayer)), assignFaderButtonWidth, controlButtonHeight + spaceBetweenPlayer + 2);
                    assignRightFaderButtons[i]->setButtonText("");
                    assignRightFaderButtons[i]->onClick = [i, this] { assignRightFader(i); grabFocusBroadcaster->sendChangeMessage(); };
                    assignRightFaderButtons[i]->setMouseClickGrabsKeyboardFocus(false);

                    addAndMakeVisible(removePlayersButtons[i]);
                    removePlayersButtons[i]->setBounds(players[i]->getRight() + meterWidth, 16 + playersStartHeightPosition + ((i * (playerHeight + spaceBetweenPlayer))), controlButtonWidth - 3, controlButtonHeight);
                    removePlayersButtons[i]->onClick = [i, this] { removePlayer(i); };
                    removePlayersButtons[i]->setMouseClickGrabsKeyboardFocus(false);
                    // Add Buttons
                    addAndMakeVisible(addPlayersButtons[i]);
                    addPlayersButtons[i]->setBounds(players[i]->getRight() + meterWidth, 16 + playersStartHeightPosition + ((i * (playerHeight + spaceBetweenPlayer))) + controlButtonHeight, controlButtonWidth - 3, controlButtonHeight);
                    addPlayersButtons[i]->onClick = [i, this] { addPlayer(i); grabFocusBroadcaster->sendChangeMessage();  };
                    addPlayersButtons[i]->setMouseClickGrabsKeyboardFocus(false);
                }

                players[i]->transport.addChangeListener(this);
                players[i]->setLabelPlayerPosition(i);
                setSize(getParentWidth(), ((playerHeight + spaceBetweenPlayer) * (playerNumber)));
            }
              
            else if (playlistType == 0)
            {
                playersPositionLabels[i]->setBounds(2, (105 * i) + 30, 40, 40);
                if (i > 8)
                    playersPositionLabels[i]->setBounds(-3, (105 * i) + 30, 40, 40);
                controlButtonXStart = getParentWidth() - controlButtonWidth;
                players[i]->setBounds(dragZoneWidth + meterWidth, playersStartHeightPosition + (i * (playerHeight + spaceBetweenPlayer)), controlButtonXStart - dragZoneWidth - meterWidth, playerHeight);
                meters[i]->setBounds(dragZoneWidth - 2, playersStartHeightPosition + (i * (playerHeight + spaceBetweenPlayer)), meterWidth - 2, playerHeight);
                players[i]->loopButton.setVisible(false);
                if (i < (playerNumber - 1))
                {
                }
                players[i]->resized();
                // Remove Buttons
                addAndMakeVisible(removePlayersButtons[i]);
                removePlayersButtons[i]->setBounds(controlButtonXStart, 16 + playersStartHeightPosition + ((i * (playerHeight + spaceBetweenPlayer))), controlButtonWidth - 3, controlButtonHeight);
                removePlayersButtons[i]->onClick = [i, this] { removePlayer(i); };
                removePlayersButtons[i]->setMouseClickGrabsKeyboardFocus(false);
                // Add Buttons
                addAndMakeVisible(addPlayersButtons[i]);
                addPlayersButtons[i]->setBounds(controlButtonXStart, 16 + playersStartHeightPosition + ((i * (playerHeight + spaceBetweenPlayer))) + controlButtonHeight, controlButtonWidth - 3, controlButtonHeight);
                addPlayersButtons[i]->setButtonText("+");
                addPlayersButtons[i]->onClick = [i, this] { addPlayer(i);
                                                            grabFocusBroadcaster->sendChangeMessage(); };
                addPlayersButtons[i]->setMouseClickGrabsKeyboardFocus(false);
                players[i]->transport.addChangeListener(this);
                setSize(getParentWidth(), ((playerHeight + spaceBetweenPlayer) * (playerNumber)));
                updateButtonsStates();
                players[i]->setLabelPlayerPosition(i);
            }
        }
    }
    updateNextPlayer();
}

void Playlist::updateButtonsStates()
{
    const juce::MessageManagerLock mmLock;
    for (auto i = 0; i < playerNumber; i++)
    {
        if (players[i] != nullptr)
        {
            if (fader1IsPlaying == true
                || fader2IsPlaying == true
                || spaceBarIsPlaying == true)
            {
                if (addPlayersButtons[i] != nullptr && removePlayersButtons[i] != nullptr)
                {
                    addPlayersButtons[i]->setEnabled(false);
                    removePlayersButtons[i]->setEnabled(false);
                }
            }
            else
            {
                if (addPlayersButtons[i] != nullptr && removePlayersButtons[i] != nullptr)
                {
                    removePlayersButtons[i]->setEnabled(true);
                    addPlayersButtons[i]->setEnabled(true);
                }
            }
        }     
    }
}

void Playlist::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    for (auto i = 0; i < playerNumber; i++)
    {
        if (players[i] != nullptr)
        {
            if (source == &players[i]->transport)
            {
                if (players[i]->transport.isPlaying())
                {
                    players[i]->isPlaying = true;
                }
                else
                {
                    playerStoppedID = i;
                    players[i]->keyIsPlaying = false;
                    if (playlistType == 0)
                    {
                        if (i == fader1Player)
                        {
                            fader1IsFlying = true;
                            fader1Stop(true);
                        }
                        else if (i == fader2Player)
                        {
                            fader2IsFlying = true;
                            fader2Stop(true);
                        }
                    }
                    if (playlistType == 1)
                    {
                        if (i == fader1Player)
                        {
                            fader1IsPlaying = false;
                        }
                        if (i == fader2Player)
                            fader2IsPlaying = false;
                    }
                    if (i == fader1Player && fader1IsPlaying == false)
                    {
                        playerStoppedID = i;
                        spaceBarStop();
                        players[i]->isPlaying = false;
                        updateNextPlayer();
                    }
                    else if (i == fader2Player && fader2IsPlaying == false)
                    {
                        playerStoppedID = i;
                        spaceBarStop();
                        players[i]->isPlaying = false;
                        updateNextPlayer();
                    }
                }
            }
            else if (source == players[i]->fxButtonBroadcaster)
            {
                if (playlistPosition == 0)
                {
                    Settings::editedPlaylist = 0;
                    Settings::editedPlayer = i;
                }
                else if (playlistPosition == 1)
                {
                    Settings::editedPlaylist = 1;
                    Settings::editedPlayer = i;
                }
                fxButtonBroadcaster->sendChangeMessage();
            }
            else if (source == players[i]->envButtonBroadcaster)
            {
                if (playlistPosition == 0)
                {
                    Settings::editedPlaylist = 0;
                    Settings::editedPlayer = i;
                }
                else if (playlistPosition == 1)
                {
                    Settings::editedPlaylist = 1;
                    Settings::editedPlayer = i;
                }
                envButtonBroadcaster->sendChangeMessage();
            }
            else if (source == players[i]->playerLaunchedBroadcaster.get())
            {
                if (playlistPosition == 0)
                {
                    Settings::editedPlaylist = 0;
                    Settings::editedPlayer = i;
                }
                else if (playlistPosition == 1)
                {
                    Settings::editedPlaylist = 1;
                    Settings::editedPlayer = i;
                }
                playerLaunchedBroadcaster->sendChangeMessage();
            }
        }
    }
}

void Playlist::playPlayer(int playerID)
{
    if (players[playerID] != nullptr)
        if (!players[playerID]->transport.isPlaying() && players[playerID]->fileLoaded)
        {
            players[playerID]->playButtonClicked();
            players[playerID]->keyIsPlaying = true;
        }
        else if (players[playerID]->transport.isPlaying())
        {
            players[playerID]->stopButtonClicked();
            players[playerID]->keyIsPlaying = false;
        }
}

void Playlist::spaceBarPressed()
{
    if ((fader1IsPlaying == false) && (fader2IsPlaying == false))
    {
        if (players[nextPlayer] != nullptr)
        {
            if (spaceBarIsPlaying == false)
            {
                if (players[nextPlayer]->fileLoaded == true)
                {
                    if (players[nextPlayer]->transport.isPlaying())
                    { 
                        players[nextPlayer]->stopButtonClicked();
                    }
                    else
                    {
                        spaceBarStartTime = juce::Time::currentTimeMillis();
                        players[nextPlayer]->spaceBarPlay();
                        players[nextPlayer]->fader1IsPlaying = true;
                        players[nextPlayer]->fader2IsPlaying = true;
                        spaceBarIsPlaying = true;
                        spaceBarPlayerId = nextPlayer;
                        updateButtonsStates();
                    }
                }
            }
            else if (spaceBarIsPlaying == true)
            {
                spaceBarStop();
                updateButtonsStates();
            }
        }
    }
}

void Playlist::spaceBarStop()
{
    if (spaceBarIsPlaying == true)
    {
        if (players[spaceBarPlayerId] != nullptr)
        {
            spaceBarStopTime = juce::Time::currentTimeMillis();
            players[spaceBarPlayerId]->stopButtonClicked();
            players[nextPlayer]->fader1IsPlaying = false;
            players[nextPlayer]->fader2IsPlaying = false;
            if (spaceBarStopTime - spaceBarStartTime >= Settings::faderTempTime)
            {
                spaceBarPlayerId++;
                fader1Player++;
                fader2Player++;
            }
            if (spaceBarPlayerId == players.size())
            {
                spaceBarPlayerId--;
                fader1Player--;
                fader2Player--;
            }
            updateNextPlayer();
        }
        spaceBarIsPlaying = false;
    }
    spaceBarIsPlaying = false;
    updateButtonsStates();
}

bool Playlist::isInterestedInFileDrag(const juce::StringArray& files)
{
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

void Playlist::filesDropped(const juce::StringArray& files, int x, int y)
{
    juce::FileLogger::getCurrentLogger()->writeToLog("file dropped");
    int i = fileDragPlayerDestination;
    for (auto file : files)
    {
        if (isInterestedInFileDrag(files))
        {
            if (files.size() > 1)
            {
                
                addPlayer(i);
                players[i + 1]->verifyAudioFileFormat(file);
                players[i + 1]->setName(droppedName.toStdString());
                i++;
            }
            else if (fileDragPaintLine == true)
            {
                addPlayer(fileDragPlayerDestination);
                players[fileDragPlayerDestination + 1]->verifyAudioFileFormat(file);
                players[fileDragPlayerDestination + 1]->setName(droppedName.toStdString());
                if (i < fader1Player && fader1IsPlaying)
                {
                    fader1Player++;

                }
                if (i < fader2Player && fader2IsPlaying)
                {
                    fader2Player++;

                }
                if (fader2Player == fader1Player && !fader2IsPlaying && fader1IsPlaying)
                    fader2Player++;
                if (fader1Player == fader2Player && !fader1IsPlaying && fader2IsPlaying)
                    fader1Player++;
                if (i < spaceBarPlayerId)
                    spaceBarPlayerId++;

            }
            else if (fileDragPaintRectangle == true)
            {
                players[fileDragPlayerDestination]->verifyAudioFileFormat(file);
                players[fileDragPlayerDestination]->setName(droppedName.toStdString());
            }
            droppedName = "";
            updateNextPlayer();
        }
    }
    if ((files.size() > 1) && fileDragPaintRectangle == true)
        removePlayer(fileDragPlayerDestination);
    fileDragPaintRectangle = false;
    fileDragPaintLine = false;
    repaint();
}

void Playlist::fileDragEnter(const juce::StringArray& files, int x, int y)
{
    for (auto file : files)
    {

    }
}
void Playlist::fileDragMove(const juce::StringArray& files, int x, int y)
{

    if (x < getParentWidth())
    {
        for (auto i = 0; i < playerNumber; i++)
        {
            if ((playlistType == 0)|| (playlistType == 1))
            {
                if (y < (playersStartHeightPosition + playerInsertDragZoneHeight))
                {
                    //Insert at the top
                    fileDragPlayerDestination = -1;
                    fileDragPaintRectangle = false;
                    if (!eightPlayerMode)
                        fileDragPaintLine = true;
                }
                else if ((y > (playersStartHeightPosition + (i * (playerHeight + spaceBetweenPlayer)) + playerHeight - playerInsertDragZoneHeight))
                    && (y < (playersStartHeightPosition + (i * (playerHeight + spaceBetweenPlayer)) + playerHeight + playerInsertDragZoneHeight)))
                {
                    //Insert between players
                    fileDragPlayerDestination = i;
                    fileDragPaintRectangle = false;
                    if (!eightPlayerMode)
                        fileDragPaintLine = true;
                }
                else if ((y > (playersStartHeightPosition + playerInsertDragZoneHeight + (i * (playerHeight + spaceBetweenPlayer))))
                    && (y < (playersStartHeightPosition - playerInsertDragZoneHeight + playerHeight + (i * (playerHeight + spaceBetweenPlayer)))))
                {
                    //replace players
                    fileDragPlayerDestination = i;
                    if (players[fileDragPlayerDestination]->fileLoaded == false)
                        fileDragPaintRectangle = true;
                    fileDragPaintLine = false;
                }
            }
        }

    }
    else if (x > playerWidth)
    {
        fileDragPaintRectangle = false;
        fileDragPaintLine = false;
    }
    repaint();
}

void Playlist::fileDragExit(const juce::StringArray& files)
{
    fileDragPaintRectangle = false;
    fileDragPaintLine = false;

    repaint();
}

void Playlist::setDroppedSoundName(juce::String name)
{
    droppedName = name;

}

void Playlist::setOptions()
{
    for (auto i = 0; i < playerNumber; i++)
    {
        if (players[i] != nullptr)
        {
            players[i]->setOptions();
        }   
    }
}

void Playlist::setMidiShift(int MidiShift)
{
    midiChannelShift = MidiShift;
}

void Playlist::valueChanged(juce::Value& value)
{

    if (players[fader1Player] != nullptr)
    {
        if (value.refersToSameSourceAs(players[fader1Player]->fileName))
        {
            fader1Name.setValue(players[fader1Player]->soundName.getText());
        }
    }
    else
        fader1Name.setValue("");

    if (players[fader2Player] != nullptr)
    {
        if (value.refersToSameSourceAs(players[fader2Player]->fileName))
        {
            fader2Name.setValue(players[fader2Player]->soundName.getText());
        }
    }
    else
        fader2Name.setValue("");
    for (auto i = 0; i < players.size(); i++)
    {
        if (value.refersToSameSourceAs(players[i]->draggedPlayer))
        {
            draggedPlayer.setValue(-1);
            draggedPlayer.setValue(i);
        }
    }
    if (players[draggedPlayer.toString().getIntValue()] != nullptr)
    {
    if (value.refersToSameSourceAs(players[draggedPlayer.toString().getIntValue()]->cueStopped))
        {

        if (players[draggedPlayer.toString().getIntValue()]->cueStopped.toString().getIntValue() == 1)
            {

            }
        }
    }
}

void Playlist::buttonClicked(juce::Button* b)
{
    for (auto i = 0; i < players.size(); i++)
    {
        if (players[i] != nullptr)
        {
            if (b == &players[i]->playerPositionLabel)
            {
                assignPlaylistFader(i);
            }
        }
    }
}

void Playlist::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
    if (getParentComponent() != nullptr) // passes only if it's not a listening event 
        getParentComponent()->mouseWheelMove (event, wheel);
}
void Playlist::mouseDown(const juce::MouseEvent& event)
{

    if (playlistType == 0 && getMouseXYRelative().getX() < getWidth())
    {
        int draggedPlayer = getMouseXYRelative().getY() / (playerHeight + spaceBetweenPlayer);

        if (!players[draggedPlayer]->transport.isPlaying())
        {
            mouseDragSource = draggedPlayer;
            mouseDraggedStartPositionOK = true;
        }
    }
    else if ((playlistType == 1 && getMouseXYRelative().getX() > (getParentWidth() - dragZoneWidth))
        || (playlistType == 1 && getMouseXYRelative().getX() < dragZoneWidth))
    {
        int draggedPlayer = getMouseXYRelative().getY() / (playerHeight + spaceBetweenPlayer);
            if (!players[draggedPlayer]->transport.isPlaying())
            {
                mouseDragSource = getMouseXYRelative().getY() / (playerHeight + spaceBetweenPlayer);
                mouseDraggedStartPositionOK = true;
            }
    }
}

void Playlist::mouseDrag(const juce::MouseEvent& event)
{
    if (mouseDraggedStartPositionOK)
    {
        mouseDragX = getMouseXYRelative().getX();
        mouseDragY = getMouseXYRelative().getY();
        mouseDraggedUp = 0;
        setMouseCursor(juce::MouseCursor::DraggingHandCursor);
        if (event.mods.isCtrlDown())
        {
            mouseCtrlModifier = true;
        }
        if (event.mods.isAltDown())
        {
            mouseAltModifier = true;
        }
    }
}

void Playlist::mouseUp(const juce::MouseEvent& event)
{
    int player = getMouseXYRelative().getY() / (playerHeight + spaceBetweenPlayer);
    if (playlistType == 0 && getMouseXYRelative().getX() < dragZoneWidth)
    {
        assignPlaylistFader(player);
    }
    else if (playlistType == 1 && getMouseXYRelative().getX() > getWidth() - dragZoneWidth)
    {
        if (players[player] != nullptr)
        {
        }
    }
    else if (playlistType == 1 && getMouseXYRelative().getX() < dragZoneWidth)
    {
        if (players[player] != nullptr)
        {
        }
    }
    mouseDraggedUp = 1;
    setMouseCursor(juce::MouseCursor::NormalCursor);
    repaint();
}

void Playlist::removeButtonClicked()
{
    if (players.size() > 1 && !players[players.size() - 1]->transport.isPlaying())
    {
        removePlayer(players.size() - 1);
    }
    else if (players.size() == 1)
    {
        if (players[0] != nullptr
            && !players[0]->fader1IsPlaying
            && !players[0]->fader2IsPlaying
            && !players[0]->keyIsPlaying)
        players[0]->deleteFile();
    }
}

void Playlist::timerCallback()
{
    for (auto i = 0; i < players.size(); i++)
    {
        if (players[i] != nullptr && meters[i] != nullptr)
        {
            meters[i]->setRMSMeterData(players[i]->outMeterSource.getRMSLevel(0), players[i]->outMeterSource.getRMSLevel(1));
            meters[i]->setPeakMeterDate(players[i]->outMeterSource.getMaxLevel(0), players[i]->outMeterSource.getMaxLevel(1));
        }

    }
    if (playlistType == 1)
    {
        for (auto i = 0; i < players.size(); i++)
        {
            if (i != fader1Player && assignLeftFaderButtons[i] != nullptr)
            {
                assignLeftFaderButtons[i]->setColour(juce::TextButton::ColourIds::buttonColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
            }
            else
                assignLeftFaderButtons[i]->setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::red);
            if (i != fader2Player && assignRightFaderButtons[i] != nullptr)
            {
                assignRightFaderButtons[i]->setColour(juce::TextButton::ColourIds::buttonColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
            }
            else
                assignRightFaderButtons[i]->setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::red);
        }
    }
}


void Playlist::assignPlaylistFader(int playerToAssign)
{
    if ((fader1IsPlaying == 0) && (fader2IsPlaying == 0) && !spaceBarIsPlaying)
    {
        fader1Player = playerToAssign;
        fader2Player = playerToAssign;
        updateNextPlayer();
    }
    else if (playerToAssign > min(fader1Player, fader2Player))
    {
        if (fader1IsPlaying && !fader2IsPlaying)
            fader2Player = playerToAssign;
        else if (fader2IsPlaying && !fader1IsPlaying)
            fader1Player = playerToAssign;
        updateNextPlayer();
    }
}

void Playlist::setTimerTime(int timertime)
{
    juce::Timer::startTimer(timertime);
    for (auto i = 0; i < players.size(); i++)
    {
        players[i]->setTimerTime(timertime);
    }
}

void Playlist::setEightPlayersSecondCart(bool isSecondCart)
{
    isEightPlayerSecondCart = isSecondCart;
}

void Playlist::isEightPlayerMode(bool eightPlayersMode)
{
    eightPlayerMode = eightPlayersMode;
    for (auto i = 0; i < players.size(); i++)
    {
        players[i]->setEightPlayerMode(true);
    }
}


void Playlist::actionListenerCallback(const juce::String& message)
{
    if (message.compareIgnoreCase("Play") == 0)
    {
        playBroadcaster->sendChangeMessage();
    }
    else
    {
        cuedPlayer = message.getIntValue();
        for (auto i = 0; i < players.size(); i++)
        {
            if (i != cuedPlayer)
                players[i]->cueTransport.stop(); //stop all other cues on the playlist
        }
        cuePlaylistBroadcaster->sendChangeMessage(); //send message to main component so he can stop cues on all others components
    }
}

void Playlist::stopCues()
{
    for (auto player : players)
    {
        player->cueTransport.stop();
    }
}

void Playlist::setPlaylistType(int t)
{
    playlistType = t;
}

void Playlist::setPlaylistPosition(int p)
{
    playlistPosition = p;
}

void Playlist::resetFxEditedButtons()
{
}

bool Playlist::isPlaying()
{
    for (auto* player : players)
    {
        if (player->transport.isPlaying())
            return true;
    }
    return false;
}

juce::ApplicationCommandTarget* Playlist::getNextCommandTarget()
{
    return nullptr;
}


void Playlist::getAllCommands(juce::Array<juce::CommandID>& commands)
{

}

void Playlist::getCommandInfo(juce::CommandID commandID, juce::ApplicationCommandInfo& result)
{

}

bool Playlist::perform(const InvocationInfo& info)
{
    return true;
}

void Playlist::shouldShowMeters(bool b)
{
    if (b)
        meterWidth = 20;
    else
        meterWidth = 0;
    rearrangePlayers();
}