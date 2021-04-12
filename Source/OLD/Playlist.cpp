/*
  ==============================================================================

    Playlist.cpp
    Created: 10 Feb 2021 2:39:14pm
    Author:  Lucien

  ==============================================================================
*/

#include <JuceHeader.h>
#include "Playlist.h"

//==============================================================================
Playlist::Playlist(int splaylistType)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    playerNumber = 8;
    playlistType = splaylistType;
    setSize(getParentWidth(), getParentHeight());
    for (auto i = 0; i < playerNumber; ++i)
    {
        //Add Player
        players.add(new Player(i));
        //players.getLast()->playerPrepareToPlay(actualSamplesPerBlockExpected, actualSampleRate);
        playlistMixer.addInputSource(&players.getLast()->resampledSource, false);


        if (playlistType == 0)
        {
            //Add Swap Next Button
            swapNextButtons.add(new juce::TextButton());
            swapNextButtons.getLast()->setButtonText("s");
            swapNextButtons.getLast()->onClick = [i, this] { swapNext(i); };

            //Add Remove Button
            removePlayersButtons.add(new juce::TextButton());
            removePlayersButtons.getLast()->setButtonText("-");
            removePlayersButtons.getLast()->onClick = [i, this] { removePlayer(i); };

            //Add Add Button
            addPlayersButtons.add(new juce::TextButton());
            addPlayersButtons.getLast()->setButtonText("+");
            addPlayersButtons.getLast()->onClick = [i, this] { addPlayer(i); };
        }

        else if (playlistType == 1)
        {
            assignLeftFaderButtons.add(new juce::TextButton());
            assignLeftFaderButtons.getLast()->setButtonText("");
            assignLeftFaderButtons.getLast()->onClick = [i, this] { assignLeftFader(i); };

            assignRightFaderButtons.add(new juce::TextButton());
            assignRightFaderButtons.getLast()->setButtonText("");
            assignRightFaderButtons.getLast()->onClick = [i, this] { assignRightFader(i); };

            assignLeftFader(0);
            assignRightFader(1);
        }
        players[i]->transport.addChangeListener(this);
    }
    minimumPlayer.addListener(this);
    fader1Value.addListener(this);
    rearrangePlayers();
    updateNextPlayer();

}

Playlist::~Playlist()
{
}

void Playlist::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()

    for (auto i = 0; i < playerNumber; ++i)
    {
       players[i]->playerPrepareToPlay(samplesPerBlockExpected, sampleRate);
    }
    //playlistMixer.prepareToPlay(samplesPerBlockExpected, sampleRate);
    actualSampleRate = sampleRate;
    actualSamplesPerBlockExpected = samplesPerBlockExpected;
}

void Playlist::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    //bufferToFill.clearActiveBufferRegion();

    //playlistMixer.getNextAudioBlock(bufferToFill);
}

void Playlist::paint(juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    //Vertical and horizontal lines between players
    g.setColour(juce::Colours::black);
    for (auto i = 1; i < playerNumber; i++)
    {
        if (playlistType == 0)
        {
            g.drawHorizontalLine((playersStartHeightPosition + (i * (playerHeight + spaceBetweenPlayer)) - 3), 0, getParentWidth());
            g.drawHorizontalLine((playersStartHeightPosition + (i * (playerHeight + spaceBetweenPlayer)) - 2), 0, getParentWidth());
        }
        else if (playlistType == 1)
        {
            g.drawHorizontalLine((playersStartHeightPosition + (i * (playerHeight + spaceBetweenPlayer)) - 3), 0, playerWidth + 2 * assignFaderButtonWidth);
            g.drawHorizontalLine((playersStartHeightPosition + (i * (playerHeight + spaceBetweenPlayer)) - 2), 0, playerWidth + 2 * assignFaderButtonWidth);
        }
    }
    if (playlistType == 0)
    {
        //g.drawVerticalLine(playerWidth, playersStartHeightPosition, 1000);
        //g.drawVerticalLine(playerWidth + 1, playersStartHeightPosition, 1000);
    }
    else if (playlistType == 1)
    {
        //g.drawVerticalLine(2 * assignFaderButtonWidth + playerWidth, playersStartHeightPosition, 1000);
        //g.drawVerticalLine(2 * assignFaderButtonWidth + playerWidth + 1, playersStartHeightPosition, 1000);
    }


    g.setColour(juce::Colours::red);
    //Paint Lines Between players when dragging
    if (fileDragPaintLine == true)
    {
        //Draw two lines at the insert position
        g.drawHorizontalLine((playersStartHeightPosition + (playerHeight + spaceBetweenPlayer) + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) - 3), 0, getParentWidth());
        g.drawHorizontalLine((playersStartHeightPosition + (playerHeight + spaceBetweenPlayer) + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) - 2), 0, getParentWidth() + 1);
    }
    else if (fileDragPaintRectangle == true)
    {
        //Draw a rectangle surroundig the player :
        //two top lines
        g.drawHorizontalLine((playersStartHeightPosition + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) - 3), 0, getParentWidth());
        g.drawHorizontalLine((playersStartHeightPosition + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) - 2), 0, getParentWidth() + 1);
        //two bottoms lines
        g.drawHorizontalLine((playersStartHeightPosition + (playerHeight + spaceBetweenPlayer) + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) - 3), 0, getParentWidth());
        g.drawHorizontalLine((playersStartHeightPosition + (playerHeight + spaceBetweenPlayer) + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) - 2), 0, getParentWidth() + 1);
        //two lines at the left
        //g.drawVerticalLine(2, playersStartHeightPosition + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)),
        //    playersStartHeightPosition + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) + playerHeight);
        //g.drawVerticalLine(3, playersStartHeightPosition + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)),
        //    playersStartHeightPosition + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) + playerHeight);
        ////two lines at the right
        //g.drawVerticalLine(getParentWidth() - controlButtonWidth - 1, playersStartHeightPosition + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)),
        //    playersStartHeightPosition + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) + playerHeight);
        //g.drawVerticalLine(getParentWidth() - controlButtonWidth - 2, playersStartHeightPosition + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)),
        //    playersStartHeightPosition + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) + playerHeight);
    }
}

void Playlist::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
    for (auto i = 0; i < playerNumber; i++)
    {
        if (players[i] != nullptr)
        {
            rearrangePlayers();
        }
    }

}

void Playlist::handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message)
{
    int midiMessageValue = message.getControllerValue();
    int midiMessageNumber = message.getControllerNumber();
    if (playlistType == 0)
    {
        if (midiMessageNumber < playerNumber)
        {
            if (midiMessageNumber == 0 + midiChannelShift)
                handleFader1(midiMessageValue);
            if (midiMessageNumber == 1 + midiChannelShift)
                handleFader2(midiMessageValue);
        }
    }
    if (playlistType == 1)
    {
        if (midiMessageNumber == 2 + midiChannelShift)
            handleFader3(midiMessageValue);
        if (midiMessageNumber == 3 + midiChannelShift)
            handleFader4(midiMessageValue);
    }
    const juce::ScopedValueSetter<bool> scopedInputFlag(isAddingFromMidiInput, true);

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
                    if (fader1ActualMidiLevel >= 1 && fader1PreviousMidiLevel == 0) //fader start
                    {
                        players[fader1Player]->play();
                        players[fader1Player]->fader1IsPlaying = true;
                        fader1IsPlaying = true;
                        //spaceBarIsPlaying = true;
                        updateButtonsStates();
                        fader1PreviousMidiLevel = fader1ActualMidiLevel;
                        if (fader2IsPlaying == false)
                            fader2Player++;
                        if (fader2Player > (playerNumber - 1))
                            fader2Player = playerNumber - 1;
                        //while ((players[fader2Player] != nullptr) && (players[fader2Player]->fileLoaded == false))
                        //{
                        //    spaceBarPlayerId++;
                        //    fader2Player++;
                        //}
                        updateNextPlayer();

                    }
                }
                if (fader1IsPlaying == true) //if it is playing
                {
                    if (fader1ActualMidiLevel == 0 && fader1PreviousMidiLevel >= 1) //fader stop
                    {
                        players[fader1Player]->stopButtonClicked();
                        players[fader1Player]->fader1IsPlaying = false;
                        //players[fader1Player]->handleMidiMessage(midiMessageNumber, 0);
                        fader1IsPlaying = false;
                        //spaceBarIsPlaying = false;
                        updateButtonsStates();
                        fader1PreviousMidiLevel = fader1ActualMidiLevel;
                        if (fader2IsPlaying == false)
                        {
                            fader2Player = std::max(fader1Player, fader2Player);
                            fader1Player = std::max(fader1Player, fader2Player);
                        }
                        else if (fader2IsPlaying == true)
                        {
                            fader1Player = std::max(fader1Player, fader2Player) + 1;
                        }
                        //if (fader1Player > (playerNumber - 1))
                          //  fader1Player = playerNumber - 1;
                        //while ((players[spaceBarPlayerId] != nullptr) && (players[spaceBarPlayerId]->fileLoaded == false))
                        //{
                        //    spaceBarPlayerId++;
                        //    fader1Player++;
                        //    fader2Player++;
                        //}
                        updateNextPlayer();
                    }

                }

            }
            if (players[fader1Player] != nullptr)
                players[fader1Player]->handleMidiMessage(midiMessageNumber, faderValue);
            //fader1Value = fader1ActualMidiLevel;
        }
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
                    if (fader2ActualMidiLevel >= 1 && fader2PreviousMidiLevel == 0) //fader start
                    {
                        players[fader2Player]->play();
                        players[fader2Player]->fader2IsPlaying = true;
                        fader2IsPlaying = true;
                        //spaceBarIsPlaying = true;
                        updateButtonsStates();
                        fader2PreviousMidiLevel = fader2ActualMidiLevel;
                        if (fader1IsPlaying == false)
                            fader1Player++;
                        if (fader1Player > (playerNumber - 1))
                            fader1Player = playerNumber - 1;
                        //while ((players[fader1Player] != nullptr) && (players[fader1Player]->fileLoaded == false))
                        //{
                        //    spaceBarPlayerId++;
                        //    fader1Player++;
                        //}
                        updateNextPlayer();
                    }
                }
                if (fader2IsPlaying == true) //if it is playing
                {
                    if (fader2ActualMidiLevel == 0 && fader2PreviousMidiLevel >= 1) //fader stop
                    {
                        players[fader2Player]->stopButtonClicked();
                        players[fader2Player]->fader2IsPlaying = false;
                        //players[fader2Player]->handleMidiMessage(midiMessageNumber, 0);
                        fader2IsPlaying = false;
                        //spaceBarIsPlaying = false;
                        updateButtonsStates();
                        fader2PreviousMidiLevel = fader2ActualMidiLevel;
                        if (fader1IsPlaying == false)
                        {
                            fader1Player = std::max(fader2Player, fader1Player);
                            fader2Player = std::max(fader2Player, fader1Player);
                        }
                        else if (fader1IsPlaying == true)
                        {
                            fader2Player = std::max(fader2Player, fader1Player) + 1;
                        }
                        //if (fader2Player > (playerNumber - 1))
                        //    fader2Player = playerNumber - 1;
                        //while ((players[spaceBarPlayerId] != nullptr) && (players[spaceBarPlayerId]->fileLoaded == false))
                        //{
                        //    spaceBarPlayerId++;
                        //    fader1Player++;
                        //    fader2Player++;
                        //}
                        updateNextPlayer();
                    }

                }

            }
            if (players[fader2Player] != nullptr)
                players[fader2Player]->handleMidiMessage(midiMessageNumber, faderValue);
        }
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
                    if (fader1ActualMidiLevel >= 1 && fader1PreviousMidiLevel == 0) //fader start
                    {
                        players[fader1Player]->play();
                        players[fader1Player]->fader1IsPlaying = true;
                        fader1IsPlaying = true;
                        fader1PreviousMidiLevel = fader1ActualMidiLevel;
                    }
                }
                if (fader1IsPlaying == true) //if it is playing
                {
                    if (fader1ActualMidiLevel == 0 && fader1PreviousMidiLevel >= 1) //fader stop
                    {
                        players[fader1Player]->stopButtonClicked();
                        players[fader1Player]->fader1IsPlaying = false;
                        fader1IsPlaying = false;
                        fader1PreviousMidiLevel = fader1ActualMidiLevel;
                    }

                }

            }
            if (players[fader1Player] != nullptr)
                players[fader1Player]->handleMidiMessage(midiMessageNumber, faderValue);
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
                if (fader2ActualMidiLevel >= 1 && fader2PreviousMidiLevel == 0) //fader start
                {
                    players[fader2Player]->play();
                    players[fader2Player]->fader2IsPlaying = true;
                    fader2IsPlaying = true;
                    fader2PreviousMidiLevel = fader2ActualMidiLevel;

                }
            }
            if (fader2IsPlaying == true) //if it is playing
            {
                if (fader2ActualMidiLevel == 0 && fader2PreviousMidiLevel >= 1) //fader stop
                {
                    players[fader2Player]->stopButtonClicked();
                    players[fader2Player]->fader2IsPlaying = false;
                    fader2IsPlaying = false;
                    fader2PreviousMidiLevel = fader2ActualMidiLevel;
                }

            }

        }
        if (players[fader2Player] != nullptr)
            players[fader2Player]->handleMidiMessage(midiMessageNumber, faderValue);
    }
}

void Playlist::updateNextPlayer()
{
    minimumPlayer = std::min(fader1Player, fader2Player);
    if (playlistType == 0)
    {
        if (fader1Player == fader2Player)
            nextPlayer = fader1Player;
        else if (fader1IsPlaying && fader2IsPlaying)
            nextPlayer = 1000;
        else if (fader1IsPlaying || fader2IsPlaying)
            nextPlayer = std::max(fader1Player, fader2Player);

        for (auto i = 0; i < playerNumber; i++)
        {
            if (players[i] != nullptr)
            {
                if (nextPlayer == i)
                    players[i]->setNextPlayer(true);
                else if (nextPlayer != i)
                    players[i]->setNextPlayer(false);
            }

        }

        spaceBarPlayerId = nextPlayer;
    }

    const juce::MessageManagerLock mmLock;
    repaint();
}

void Playlist::playersResetPositionClicked()
{
    if ((fader1IsPlaying == 0) && (fader2IsPlaying == 0))
    {
        fader1Player = 0;
        fader2Player = 0;
        updateNextPlayer();

    }
}

void Playlist::playersPreviousPositionClicked()
{
    if ((fader1IsPlaying == 0) && (fader2IsPlaying == 0))
    {
        nextPlayer = (std::min(fader1Player, fader2Player) - 1);
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

void Playlist::playersNextPositionClicked()
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
    setOptions(FFmpegPath, ExiftoolPath, convertedFilesPath, skewFactor, maxFaderValue);
    updateNextPlayer();
}

void Playlist::addPlayer(int playerID)
{
    int idAddedPlayer = playerID + 1;
    players.insert(idAddedPlayer, new Player(idAddedPlayer));
    players[idAddedPlayer]->playerPrepareToPlay(actualSamplesPerBlockExpected, actualSampleRate);
    playlistMixer.addInputSource(&players[idAddedPlayer]->resampledSource, false);

    if (playlistType == 0)
    {
        swapNextButtons.insert(idAddedPlayer, new juce::TextButton());
        swapNextButtons[idAddedPlayer]->setButtonText("swap");
        swapNextButtons[idAddedPlayer]->onClick = [idAddedPlayer, this] { swapNext(idAddedPlayer); };

        removePlayersButtons.insert(idAddedPlayer, new juce::TextButton());
        removePlayersButtons[idAddedPlayer]->setButtonText("-");
        removePlayersButtons[idAddedPlayer]->onClick = [idAddedPlayer, this] { removePlayer(idAddedPlayer); };

        addPlayersButtons.insert(idAddedPlayer, new juce::TextButton());
        addPlayersButtons[idAddedPlayer]->setButtonText("+");
        addPlayersButtons[idAddedPlayer]->onClick = [idAddedPlayer, this] { addPlayer(idAddedPlayer); };
    }
    else if (playlistType == 1)
    {
        assignLeftFaderButtons.insert(idAddedPlayer, new juce::TextButton());
        assignLeftFaderButtons[idAddedPlayer]->setButtonText("");
        assignLeftFaderButtons[idAddedPlayer]->onClick = [idAddedPlayer, this] { assignLeftFader(idAddedPlayer); };

        assignRightFaderButtons.insert(idAddedPlayer, new juce::TextButton());
        assignRightFaderButtons[idAddedPlayer]->setButtonText("");
        assignRightFaderButtons[idAddedPlayer]->onClick = [idAddedPlayer, this] { assignRightFader(idAddedPlayer); };

    }
    playerNumber = players.size();
    rearrangePlayers();
    updateNextPlayer();
    setOptions(FFmpegPath, ExiftoolPath, convertedFilesPath, skewFactor, maxFaderValue);
    repaint();
}

void Playlist::removePlayer(int playerID)
{
    if ((fader1IsPlaying == 0) && (fader2IsPlaying == 0))
    {
        if (players.size() > 0)
        {
            if (players[playerID] != nullptr)
            {
                playlistMixer.removeInputSource(&players[playerID]->resampledSource);
                players.remove(playerID);
                swapNextButtons.remove(playerID);
                removePlayersButtons.remove(playerID);
                addPlayersButtons.remove(playerID);
                playerNumber = players.size();
                updateNextPlayer();
                rearrangePlayers();
            }
        }
    }
}

void Playlist::swapNext(int playerID)
{
    players.swap(playerID, playerID + 1);
    updateNextPlayer();
    rearrangePlayers();
}


void Playlist::assignLeftFader(int playerID)
{
    if (players[playerID] != nullptr)
    {
        if (fader1Player == playerID)
        {
            fader1Player = 1000;
            players[playerID]->faderLeftAssigned = !players[playerID]->faderLeftAssigned;
        }
        else if ((fader1IsPlaying == false) && (players[playerID]->faderRightAssigned == false))
        {
            fader1Player = playerID;
            for (auto i = 0; i <= playerNumber; ++i)
            {
                if (players[i] != nullptr)
                {

                    if (i == fader1Player)
                        players[i]->setLeftFaderAssigned(true);
                    else
                        players[i]->setLeftFaderAssigned(false);
                }
            }
        }
    }
}

void Playlist::assignRightFader(int playerID)
{

    if (players[playerID] != nullptr)
    {
        if (fader2Player == playerID)
        {
            fader2Player = 1000;
            players[playerID]->faderRightAssigned = !players[playerID]->faderRightAssigned;
        }
        else if ((fader2IsPlaying == false) && (players[playerID]->faderLeftAssigned == false))
        {
            fader2Player = playerID;
            for (auto i = 0; i <= playerNumber; ++i)
            {
                if (players[i] != nullptr)
                {
                    if (i == fader2Player)
                        players[i]->setRightFaderAssigned(true);
                    else
                        players[i]->setRightFaderAssigned(false);
                }
            }
        }
    }
}

void Playlist::rearrangePlayers()
{
    playerNumber = players.size();

    for (auto i = 0; i <= playerNumber; ++i)
    {
        if (players[i] != nullptr)
        {
            // Player
            addAndMakeVisible(players[i]);
            if (playlistType == 1)
            {
                players[i]->setBounds(assignFaderButtonWidth, playersStartHeightPosition + (i * (playerHeight + spaceBetweenPlayer)), getParentWidth() - (assignFaderButtonWidth*2), playerHeight);
                players[i]->isCart = true;
                addAndMakeVisible(assignLeftFaderButtons[i]);
                assignLeftFaderButtons[i]->setBounds(1, 33 + playersStartHeightPosition + (i * (playerHeight + spaceBetweenPlayer)), assignFaderButtonWidth - 1, controlButtonHeight + spaceBetweenPlayer + 2);
                assignLeftFaderButtons[i]->setButtonText("");
                assignLeftFaderButtons[i]->onClick = [i, this] { assignLeftFader(i); };

                addAndMakeVisible(assignRightFaderButtons[i]);
                assignRightFaderButtons[i]->setBounds(getParentWidth() - assignFaderButtonWidth, 33 + playersStartHeightPosition + (i * (playerHeight + spaceBetweenPlayer)), assignFaderButtonWidth, controlButtonHeight + spaceBetweenPlayer + 2);
                assignRightFaderButtons[i]->setButtonText("");
                assignRightFaderButtons[i]->onClick = [i, this] { assignRightFader(i); };

                players[i]->transport.addChangeListener(this);
                players[i]->setLabelPlayerPosition(i);
                setSize(getParentWidth(), ((playerHeight + spaceBetweenPlayer) * (playerNumber)));
            }
               
            // Swap Buttonns
            else if (playlistType == 0)
            {
                controlButtonXStart = getParentWidth() - controlButtonWidth;
                players[i]->setBounds(0, playersStartHeightPosition + (i * (playerHeight + spaceBetweenPlayer)), controlButtonXStart, playerHeight);
                players[i]->loopButton.setVisible(false);
                if (i < (playerNumber - 1))
                {
                    addAndMakeVisible(swapNextButtons[i]);
                    swapNextButtons[i]->setBounds(controlButtonXStart, 16 + playersStartHeightPosition + ((i * (playerHeight + spaceBetweenPlayer)) + (2 * controlButtonHeight)), controlButtonWidth, controlButtonHeight + spaceBetweenPlayer + 2);
                    swapNextButtons[i]->setButtonText("s");
                    swapNextButtons[i]->onClick = [i, this] { swapNext(i); };
                }
                // Remove Buttons
                addAndMakeVisible(removePlayersButtons[i]);
                removePlayersButtons[i]->setBounds(controlButtonXStart, 16 + playersStartHeightPosition + ((i * (playerHeight + spaceBetweenPlayer))), controlButtonWidth, controlButtonHeight);
                removePlayersButtons[i]->onClick = [i, this] { removePlayer(i); };

                // Add Buttons
                addAndMakeVisible(addPlayersButtons[i]);
                addPlayersButtons[i]->setBounds(controlButtonXStart, 16 + playersStartHeightPosition + ((i * (playerHeight + spaceBetweenPlayer))) + controlButtonHeight, controlButtonWidth, controlButtonHeight);
                addPlayersButtons[i]->setButtonText("+");
                addPlayersButtons[i]->onClick = [i, this] { addPlayer(i); };
                players[i]->transport.addChangeListener(this);
                setSize(getParentWidth(), ((playerHeight + spaceBetweenPlayer) * (playerNumber)));
                updateButtonsStates();
                players[i]->setLabelPlayerPosition(i);
                //players[i]->volumeSliderValue.addListener(this);
            }

        }
    }
    updateNextPlayer();

    //setSize(715, getParentHeight());

}

void Playlist::updateButtonsStates()
{
    const juce::MessageManagerLock mmLock;
    for (auto i = 0; i < playerNumber; i++)
    {
        if (players[i] != nullptr)
        {
            if (((fader1Player == i) && fader1IsPlaying == true) || ((fader2Player == i) && fader2IsPlaying == true))
            {
                swapNextButtons[i]->setEnabled(false);
                removePlayersButtons[i]->setEnabled(false);
            }
            else
            {
                swapNextButtons[i]->setEnabled(true);
                removePlayersButtons[i]->setEnabled(true);
            }

            if (i == (players.size() - 1))
                swapNextButtons[i]->setEnabled(false);
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
                    spaceBarStop();
                    players[i]->isPlaying = false;
                    updateNextPlayer();
                }
            }
        }
    }
}

bool Playlist::keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent)
{
    if (playlistType == 0)
    {
        if (key == juce::KeyPress::spaceKey)
            spaceBarPressed();
        else if (key == juce::KeyPress::escapeKey)
        {
            if (spaceBarIsPlaying == false)
                playersResetPositionClicked();
        }
        else if (key == juce::KeyPress::upKey)
        {
            if (spaceBarIsPlaying == false)
                playersPreviousPositionClicked();
        }
        else if (key == juce::KeyPress::downKey)
        {
            if (spaceBarIsPlaying == false)
                playersNextPositionClicked();
        }
    }

    return false;
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
                    players[nextPlayer]->playButtonClicked();
                    players[nextPlayer]->fader1IsPlaying = true;
                    players[nextPlayer]->fader2IsPlaying = true;
                    spaceBarIsPlaying = true;
                    spaceBarPlayerId = nextPlayer;
                }
            }
            else if (spaceBarIsPlaying == true)
            {
                spaceBarStop();
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
            players[spaceBarPlayerId]->stopButtonClicked();
            players[nextPlayer]->fader1IsPlaying = false;
            players[nextPlayer]->fader2IsPlaying = false;
            spaceBarPlayerId++;
            fader1Player++;
            fader2Player++;
            //while ((players[spaceBarPlayerId] != nullptr) && (players[spaceBarPlayerId]->fileLoaded == false))
            //{
            //    spaceBarPlayerId++;
            //    fader1Player++;
            //    fader2Player++;
            //    spaceBarIsPlaying = false;
            //}
            updateNextPlayer();
        }
        spaceBarIsPlaying = false;
    }
    spaceBarIsPlaying = false;

}

bool Playlist::isInterestedInFileDrag(const juce::StringArray& files)
{
    for (auto file : files)

        if ((file.contains(".wav")) || (file.contains(".WAV"))
            || (file.contains(".bwf")) || (file.contains(".BWF"))
            || (file.contains(".aiff")) || (file.contains(".AIFF"))
            || (file.contains(".aif")) || (file.contains(".AIF"))
            || (file.contains(".flac")) || (file.contains(".FLAC"))
            || (file.contains(".opus")) || (file.contains(".OPUS"))
            || (file.contains(".mp3")) || (file.contains(".MP3")))
        {
            return true;
        }
    return false;
}

void Playlist::filesDropped(const juce::StringArray& files, int x, int y)
{
    int i = fileDragPlayerDestination;
    for (auto file : files)
    {
        if (isInterestedInFileDrag(files))
        {
            //DBG(files.size());
            if (files.size() > 1)
            {
                
                addPlayer(i);
                players[i + 1]->verifyAudioFileFormat(file);
                //if (fileDragPaintRectangle == true)
                    //removePlayer(fileDragPlayerDestination);
                i++;
                DBG(i);
            }
            else if (fileDragPaintLine == true)
            {
                addPlayer(fileDragPlayerDestination);
                players[fileDragPlayerDestination + 1]->verifyAudioFileFormat(file);
            }
            else if (fileDragPaintRectangle == true)
            {
                players[fileDragPlayerDestination]->verifyAudioFileFormat(file);
            }
        }
    }
    if ((files.size() > 1) && fileDragPaintRectangle == true)
        removePlayer(fileDragPlayerDestination);
    fileDragPaintRectangle = false;
    fileDragPaintLine = false;
    repaint();
}

void Playlist::fileDragMove(const juce::StringArray& files, int x, int y)
{
    if (x < getParentWidth())
    {
        for (auto i = 0; i < playerNumber; i++)
        {
            if (playlistType == 0)
            {
                if (y < (playersStartHeightPosition + playerInsertDragZoneHeight))
                {
                    //Insert at the top
                    fileDragPlayerDestination = -1;
                    fileDragPaintRectangle = false;
                    fileDragPaintLine = true;
                }
                else if ((y > (playersStartHeightPosition + (i * (playerHeight + spaceBetweenPlayer)) + playerHeight - playerInsertDragZoneHeight))
                    && (y < (playersStartHeightPosition + (i * (playerHeight + spaceBetweenPlayer)) + playerHeight + playerInsertDragZoneHeight)))
                {
                    //Insert between players
                    fileDragPlayerDestination = i;
                    fileDragPaintRectangle = false;
                    fileDragPaintLine = true;

                }
                else if ((y > (playersStartHeightPosition + playerInsertDragZoneHeight + (i * (playerHeight + spaceBetweenPlayer))))
                    && (y < (playersStartHeightPosition - playerInsertDragZoneHeight + playerHeight + (i * (playerHeight + spaceBetweenPlayer)))))
                {
                    //replace players
                    fileDragPlayerDestination = i;
                    fileDragPaintRectangle = true;
                    fileDragPaintLine = false;
                }
            }
            else if (playlistType == 1)
            {
            if ((y > (playersStartHeightPosition + playerInsertDragZoneHeight + (i * (playerHeight + spaceBetweenPlayer))))
                && (y < (playersStartHeightPosition - playerInsertDragZoneHeight + playerHeight + (i * (playerHeight + spaceBetweenPlayer)))))
            {
                //replace players
                fileDragPlayerDestination = i;
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

void Playlist::setOptions(juce::String sFFmpegPath, juce::String sExiftoolPath, juce::String sconvertedFilesPath, float sskewFactor, int smaxFaderValue)
{
    FFmpegPath = sFFmpegPath;
    convertedFilesPath = sconvertedFilesPath;
    skewFactor = sskewFactor;
    maxFaderValue = smaxFaderValue;
    ExiftoolPath = sExiftoolPath;

    for (auto i = 0; i < playerNumber; i++)
    {
        if (players[i] != nullptr)
        {
            players[i]->setOptions(FFmpegPath, ExiftoolPath, convertedFilesPath, skewFactor, maxFaderValue);
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
        if (value.refersToSameSourceAs(players[fader1Player]->volumeSliderValue))
        {

            if (previousvalueint != valueint)
            {
                valueint = value.toString().getFloatValue() * 127;
                fader1Value = valueint;
            }
            previousvalueint = valueint;
        }
    }
}