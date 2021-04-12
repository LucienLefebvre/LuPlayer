#include "MainComponent.h"


//TODO vumetre
//Bug entre clavier et faders

//==============================================================================
MainComponent::MainComponent() : audioSetupComp(deviceManager,
    0,     // minimum input channels
    0,   // maximum input channels
    0,     // minimum output channels
    2,   // maximum output channels
    true, // ability to select midi inputs
    false, // ability to select midi output device
    true, // treat channels as stereo pairs
    true) // hide advanced options

{
    playerNumber = 4;
    setSize(1200, playersStartHeightPosition + (playerHeight + spaceBetweenPlayer)*playerNumber);
    juce::Timer::startTimer(100);


    // Some platforms require permissions to open input channels so request that here
    if (juce::RuntimePermissions::isRequired(juce::RuntimePermissions::recordAudio)
        && !juce::RuntimePermissions::isGranted(juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request(juce::RuntimePermissions::recordAudio,
            [&](bool granted) { setAudioChannels(granted ? 2 : 0, 2); });
    }
    else
    {

        setAudioChannels(0, 2);
    }


    //addAndMakeVisible(audioSetupComp);
    //audioSetupComp.addToDesktop(juce::ComponentPeer::StyleFlags::windowHasCloseButton);
    //audioSetupComp.setBounds(800, 200,200, 200);


    addAndMakeVisible(playersResetPosition);
    playersResetPosition.setBounds(totalPlayerWidthWithButtons, playersStartHeightPosition + controlButtonHeight, 70, controlButtonHeight);
    playersResetPosition.setButtonText("Reset");
    playersResetPosition.onClick = [this] { playersResetPositionClicked(); };

    addAndMakeVisible(playersPreviousPosition);
    playersPreviousPosition.setBounds(totalPlayerWidthWithButtons, playersStartHeightPosition, 70, controlButtonHeight);
    playersPreviousPosition.setButtonText("Up");
    playersPreviousPosition.onClick = [this] { playersPreviousPositionClicked(); };

    addAndMakeVisible(playersNextPosition);
    playersNextPosition.setBounds(totalPlayerWidthWithButtons, playersStartHeightPosition + 2* controlButtonHeight, 70, controlButtonHeight);
    playersNextPosition.setButtonText("Down");
    playersNextPosition.onClick = [this] { playersNextPositionClicked(); };

    addAndMakeVisible(saveButton);
    saveButton.setBounds(200, 0, 100, 25);
    saveButton.setButtonText("Save");
    saveButton.onClick = [this] { savePlaylist(); };

    addAndMakeVisible(loadButton);
    loadButton.setBounds(300, 0, 100, 25);
    loadButton.setButtonText("Load");
    loadButton.onClick = [this] { loadPlaylist(); };

    auto viewportPlayer = new Player(10);
    addAndMakeVisible(playlistViewport);
    playlistViewport.setViewedComponent(viewportPlayer);
    playlistViewport.setBounds(800, 200, 200, 100);
    viewportPlayer->setSize(800, 100);


    setOpaque(true);

    midiInitialize();
       
    for (auto i = 0; i < playerNumber; ++i)
    {
        //Add Player
        players.add(new Player(i));
        players.getLast()->playerPrepareToPlay(actualSamplesPerBlockExpected, actualSampleRate);
        myMixer.addInputSource(&players.getLast()->resampledSource, false);
        
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

        players[i]->transport.addChangeListener(this);
    }
    rearrangePlayers();
    updateNextPlayer();

    addKeyListener(this);
    setWantsKeyboardFocus(true);

}

MainComponent::~MainComponent()
{
    shutdownAudio();
}


void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()
    
    actualSampleRate = sampleRate;
    actualSamplesPerBlockExpected = samplesPerBlockExpected;

    
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();

    myMixer.getNextAudioBlock(bufferToFill);
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
}


void MainComponent::paint(juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    ////Rectangle Fader1
    //{
    //    int x = 0, y = (fader1Player * (playerHeight + spaceBetweenPlayer)) + playersStartHeightPosition, width = borderRectangleWidth, height = borderRectangleHeight;
    //    if (fader1IsPlaying == false)
    //        g.setColour(juce::Colour(255, 163, 0));
    //    else if (fader1IsPlaying == true)
    //        g.setColour(juce::Colours::green);
    //    g.fillRect(x, y, width, height);
    //}

    //Rectangle Fader2
    //{
    //    int x = 662, y = (fader2Player * (playerHeight + spaceBetweenPlayer)) + playersStartHeightPosition, width = borderRectangleWidth, height = borderRectangleHeight;
    //    if (fader2IsPlaying == false)
    //        g.setColour(juce::Colour(255, 163, 0));
    //    else if (fader2IsPlaying == true)
    //        g.setColour(juce::Colours::green);
    //    g.fillRect(x, y, width, height);
    //    g.fillRect(x, y, width, height);
    //}

    //Vertical and horizontal lines between players
    g.setColour(juce::Colours::black);
    for (auto i = 0; i < playerNumber; i++)
    {
        g.drawHorizontalLine((playersStartHeightPosition + (i * (playerHeight + spaceBetweenPlayer)) - 3), 0, playerWidth);
        g.drawHorizontalLine((playersStartHeightPosition + (i * (playerHeight + spaceBetweenPlayer)) - 2), 0, playerWidth + 1);
    }
    g.drawVerticalLine(playerWidth, playersStartHeightPosition, 1000);
    g.drawVerticalLine(playerWidth + 1, playersStartHeightPosition, 1000);


    g.setColour(juce::Colours::red);
    //Paint Lines Between players when dragging
    if (fileDragPaintLine == true)
    {
        //Draw two lines at the insert position
        g.drawHorizontalLine((playersStartHeightPosition + (playerHeight + spaceBetweenPlayer) + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) - 3), 0, playerWidth);
        g.drawHorizontalLine((playersStartHeightPosition + (playerHeight + spaceBetweenPlayer) + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) - 2), 0, playerWidth + 1);
    }
    else if (fileDragPaintRectangle == true)
    {
        //Draw a rectangle surroundig the player :
        //two top lines
        g.drawHorizontalLine((playersStartHeightPosition + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) - 3), 0, playerWidth);
        g.drawHorizontalLine((playersStartHeightPosition + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) - 2), 0, playerWidth + 1);
        //two bottoms lines
        g.drawHorizontalLine((playersStartHeightPosition + (playerHeight + spaceBetweenPlayer) + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) - 3), 0, playerWidth);
        g.drawHorizontalLine((playersStartHeightPosition + (playerHeight + spaceBetweenPlayer) + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) - 2), 0, playerWidth + 1);
        //two lines at the left
        g.drawVerticalLine(0, playersStartHeightPosition + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)), 
                playersStartHeightPosition + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) + playerHeight);
        g.drawVerticalLine(1, playersStartHeightPosition + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)),
            playersStartHeightPosition + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) + playerHeight);
        //two lines at the right
        g.drawVerticalLine(playerWidth, playersStartHeightPosition + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)),
            playersStartHeightPosition + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) + playerHeight);
        g.drawVerticalLine(playerWidth + 1, playersStartHeightPosition + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)),
            playersStartHeightPosition + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) + playerHeight);
    }
}

void MainComponent::resized()
{

}


void MainComponent::timerCallback()
{

    //paintPlayHead(g, thumbnailBounds);
}


void MainComponent::audioInitialize()
{

}


void MainComponent::midiInitialize()
{
    addAndMakeVisible(midiInputListLabel);
    midiInputListLabel.setText("MIDI Input:", juce::dontSendNotification);
    midiInputListLabel.attachToComponent(&midiInputList, true);
    midiInputListLabel.setBounds(0, 0, 100, 25);
    midiInputListLabel.setWantsKeyboardFocus(false);


    //! [midiInputList]
    addAndMakeVisible(midiInputList);
    midiInputList.setTextWhenNoChoicesAvailable("No MIDI Inputs Enabled");
    midiInputList.setBounds(100, 0, 100, 25);
    auto midiInputs = juce::MidiInput::getAvailableDevices();
    midiInputList.setWantsKeyboardFocus(false);


    juce::StringArray midiInputNames;

    for (auto input : midiInputs)
        midiInputNames.add(input.name);

    midiInputList.addItemList(midiInputNames, 1);
    midiInputList.onChange = [this] { setMidiInput(midiInputList.getSelectedItemIndex()); };

    // find the first enabled device and use that by default
    for (auto input : midiInputs)
    {
        if (deviceManager.isMidiInputDeviceEnabled(input.identifier))
        {
            setMidiInput(midiInputs.indexOf(input));
            break;
        }
    }

    // if no enabled devices were found just use the first one in the list
    if (midiInputList.getSelectedId() == 0)
        setMidiInput(0);
}

void MainComponent::setMidiInput(int index)
{
    auto list = juce::MidiInput::getAvailableDevices();

    deviceManager.removeMidiInputDeviceCallback(list[lastInputIndex].identifier, this);

    auto newInput = list[index];

    if (!deviceManager.isMidiInputDeviceEnabled(newInput.identifier))
        deviceManager.setMidiInputDeviceEnabled(newInput.identifier, true);

    deviceManager.addMidiInputDeviceCallback(newInput.identifier, this);
    midiInputList.setSelectedId(index + 1, juce::dontSendNotification);
    
    lastInputIndex = index;
}

void MainComponent::handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message)
{
    midiMessageValue = message.getControllerValue();
    midiMessageNumber = message.getControllerNumber();

    if (midiMessageNumber < playerNumber)
    {
        if (midiMessageNumber == 0)
            handleFader1(midiMessageValue);
        if (midiMessageNumber == 1)
            handleFader2(midiMessageValue);
    }

    const juce::ScopedValueSetter<bool> scopedInputFlag(isAddingFromMidiInput, true);
 
}

void MainComponent::handleFader1(int faderValue)
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
                        while ((players[fader2Player] != nullptr) && (players[fader2Player]->fileLoaded == false))
                        {
                            spaceBarPlayerId++;
                            fader2Player++;
                        }
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
                        while ((players[spaceBarPlayerId] != nullptr) && (players[spaceBarPlayerId]->fileLoaded == false))
                        {
                            spaceBarPlayerId++;
                            fader1Player++;
                            fader2Player++;
                        }
                        updateNextPlayer();
                    }

                }

            }
            if (players[fader1Player] != nullptr)
                players[fader1Player]->handleMidiMessage(midiMessageNumber, faderValue);
        }
    }
}

void MainComponent::handleFader2(int faderValue)
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
                        while ((players[fader1Player] != nullptr) && (players[fader1Player]->fileLoaded == false))
                        {
                            spaceBarPlayerId++;
                            fader1Player++;
                        }
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
                        while ((players[spaceBarPlayerId] != nullptr) && (players[spaceBarPlayerId]->fileLoaded == false))
                        {
                            spaceBarPlayerId++;
                            fader1Player++;
                            fader2Player++;
                        }
                        updateNextPlayer();
                    }

                }

            }
            if (players[fader2Player] != nullptr)
                players[fader2Player]->handleMidiMessage(midiMessageNumber, faderValue);
        }
    }
}


void MainComponent::updateNextPlayer()
{

    /*if (nextPlayer == players.size())
        nextPlayer = players.size() - 1;*/

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

    const juce::MessageManagerLock mmLock;
    repaint();
}

void MainComponent::playersResetPositionClicked()
{
    if ((fader1IsPlaying == 0) && (fader2IsPlaying == 0))
    {
        fader1Player = 0;
        fader2Player = 0;
        updateNextPlayer();

    }
}

void MainComponent::playersPreviousPositionClicked()
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

void MainComponent::playersNextPositionClicked()
{
        if (nextPlayer < playerNumber)
            nextPlayer ++;
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
            fader1Player = (players.size()-1);
        if (fader2Player == players.size())
            fader2Player = (players.size()-1);

        updateNextPlayer();
}


void MainComponent::addPlayer(int playerID)
{
    int idAddedPlayer = playerID + 1;
    players.insert(idAddedPlayer, new Player(idAddedPlayer));
    players[idAddedPlayer]->playerPrepareToPlay(actualSamplesPerBlockExpected, actualSampleRate);
    myMixer.addInputSource(&players[idAddedPlayer]->resampledSource, false);

    swapNextButtons.insert(idAddedPlayer, new juce::TextButton());
    swapNextButtons[idAddedPlayer]->setButtonText("swap");
    swapNextButtons[idAddedPlayer]->onClick = [idAddedPlayer, this] { swapNext(idAddedPlayer); };

    removePlayersButtons.insert(idAddedPlayer, new juce::TextButton());
    removePlayersButtons[idAddedPlayer]->setButtonText("-");
    removePlayersButtons[idAddedPlayer]->onClick = [idAddedPlayer, this] { removePlayer(idAddedPlayer); };

    addPlayersButtons.insert(idAddedPlayer, new juce::TextButton());
    addPlayersButtons[idAddedPlayer]->setButtonText("+");
    addPlayersButtons[idAddedPlayer]->onClick = [idAddedPlayer, this] { addPlayer(idAddedPlayer); };

    playerNumber = players.size();
    rearrangePlayers();
    updateNextPlayer();
    repaint();
}

void MainComponent::removePlayer(int playerID)
{
    if ((fader1IsPlaying == 0) && (fader2IsPlaying == 0))
    {
        if (players.size() > 2)
        {
            if (players[playerID] != nullptr)
            {
                myMixer.removeInputSource(&players[playerID]->resampledSource);
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

void MainComponent::swapNext(int playerID)
{
    players.swap(playerID, playerID + 1);
    updateNextPlayer();
    rearrangePlayers();
}

void MainComponent::rearrangePlayers()
{
    playerNumber = players.size();
    DBG(players.size());
    for (auto i = 0; i <= playerNumber; ++i)
    {
        if (players[i] != nullptr)
        {
            // Player
            addAndMakeVisible(players[i]);
            players[i]->setBounds(0, playersStartHeightPosition + (i * (playerHeight + spaceBetweenPlayer)), playerWidth, playerHeight);
            // Swap Buttonns
            if (i < (playerNumber - 1))
            {
                addAndMakeVisible(swapNextButtons[i]);
                swapNextButtons[i]->setBounds(playerWidth + 2, 16 + playersStartHeightPosition + ((i * (playerHeight + spaceBetweenPlayer)) + (2 * controlButtonHeight)), controlButtonWidth, controlButtonHeight + spaceBetweenPlayer + 2);
                swapNextButtons[i]->setButtonText("s");
                swapNextButtons[i]->onClick = [i, this] { swapNext(i); };
            }
            // Remove Buttons
            addAndMakeVisible(removePlayersButtons[i]);
            removePlayersButtons[i]->setBounds(playerWidth + 2, 16 + playersStartHeightPosition + ((i * (playerHeight + spaceBetweenPlayer))), controlButtonWidth, controlButtonHeight);
            removePlayersButtons[i]->onClick = [i, this] { removePlayer(i); };

            // Add Buttons
            addAndMakeVisible(addPlayersButtons[i]);
            addPlayersButtons[i]->setBounds(playerWidth + 2, 16 + playersStartHeightPosition + ((i * (playerHeight + spaceBetweenPlayer))) + controlButtonHeight, controlButtonWidth, controlButtonHeight);
            addPlayersButtons[i]->setButtonText("+");
            addPlayersButtons[i]->onClick = [i, this] { addPlayer(i); };
            players[i]->transport.addChangeListener(this);
        }
    }
    updateNextPlayer();
    updateButtonsStates();
    setSize(1200, (playersStartHeightPosition + (playerHeight + spaceBetweenPlayer) * (playerNumber)));

}

void MainComponent::updateButtonsStates()
{
    const juce::MessageManagerLock mmLock;
    for (auto i = 0; i < playerNumber; i++)
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

        players[i]->setLabelPlayerPosition(i);

    }

}

void MainComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    for (auto i = 0; i < playerNumber; i++)
    {
        if (source == &players[i]->transport)
        {
            if (players[i]->transport.isPlaying())
            {
                players[i]->isPlaying = true;
            }
            else
            {
                spaceBarStop();
                players[i]->isPlaying = false;
            }
        }
    }
}


bool MainComponent::keyPressed(const juce::KeyPress &key, juce::Component* originatingComponent)
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
    return false;
}

void MainComponent::spaceBarPressed()
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

void MainComponent::spaceBarStop()
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
            while ((players[spaceBarPlayerId] != nullptr) && (players[spaceBarPlayerId]->fileLoaded == false))
            {
                spaceBarPlayerId++;
                fader1Player++;
                fader2Player++;
                spaceBarIsPlaying = false;
            }
            updateNextPlayer();
        }
        spaceBarIsPlaying = false;
    }
    spaceBarIsPlaying = false;

}


bool MainComponent::isInterestedInFileDrag(const juce::StringArray& files)
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

void MainComponent::filesDropped(const juce::StringArray& files, int x, int y)
{
    for (auto file : files)
        if (isInterestedInFileDrag(files))
        {   
            if (fileDragPaintLine == true)
            {
                addPlayer(fileDragPlayerDestination);
                players[fileDragPlayerDestination + 1]->verifyAudioFileFormat(file);
            }
            else if (fileDragPaintRectangle == true)
            {
                players[fileDragPlayerDestination]->verifyAudioFileFormat(file);
            }
        }
    fileDragPaintRectangle = false;
    fileDragPaintLine = false;
    repaint();
}

void MainComponent::fileDragMove(const juce::StringArray& files, int x, int y)
{
    if (x < playerWidth)
    {
        for (auto i = 0; i < playerNumber; i++)
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
    }
    else if (x > playerWidth)
    {
        fileDragPaintRectangle = false;
        fileDragPaintLine = false;
    }
    repaint();
}

void MainComponent::fileDragExit(const juce::StringArray& files)
{
    fileDragPaintRectangle = false;
    fileDragPaintLine = false;
    repaint();
}


void MainComponent::savePlaylist()
{
    juce::XmlElement playlist("PLAYLIST");

    for (int i = 0; i < players.size(); ++i)
    {
        if (players[i]->fileLoaded == true)
        {
            juce::XmlElement* player = new juce::XmlElement("PLAYER");
            player->setAttribute("ID", i);
            player->setAttribute("path", players[i]->getFilePath());
            player->setAttribute("trimvolume", players[i]->getTrimVolume());
            player->setAttribute("islooping", players[i]->getIsLooping());
            playlist.addChildElement(player);
        }
    }

    auto xmlString = playlist.toString();

    juce::FileChooser chooser("Choose an XML file", juce::File::getSpecialLocation(juce::File::currentApplicationFile), "*.xml");

    if (chooser.browseForFileToSave(true))
    {
        juce::File myPlaylistSave;
        myPlaylistSave = chooser.getResult();
        playlist.writeTo(myPlaylistSave);
    }
}

void MainComponent::loadPlaylist()
{
    
    juce::FileChooser chooser("Choose an XML File", juce::File::getSpecialLocation(juce::File::currentApplicationFile), "*.xml");

    if (chooser.browseForFileToOpen())
        myFile = chooser.getResult();


    if (auto xml = parseXML(myFile))
    {
        for (auto i = 0; i < (players.size() + 1); i++)
            removePlayer(i);
        if (xml->hasTagName("PLAYLIST"))
        {
            auto iLoadedPlayer = 0;
            forEachXmlChildElement(*xml, e)
            {
                if (e->hasTagName("PLAYER"))
                {
                    int playerID = e->getIntAttribute("ID");
                    juce::String filePath = juce::String(e->getStringAttribute("path"));
                    int trimvolume = e->getDoubleAttribute("trimvolume");
                    bool islooping = e->getBoolAttribute("islooping");
                    if (players[iLoadedPlayer] == nullptr)
                    {
                        addPlayer(iLoadedPlayer - 1);
                    }
                    players[iLoadedPlayer]->verifyAudioFileFormat(filePath);
                    players[iLoadedPlayer]->setTrimVolume(trimvolume);
                    players[iLoadedPlayer]->setIsLooping(islooping);
                    iLoadedPlayer++;
                }
            }
        }
    }
}