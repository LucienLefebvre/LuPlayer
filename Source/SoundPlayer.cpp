/*
  ==============================================================================

    SoundPlayer.cpp
    Created: 15 Mar 2021 2:18:44pm
    Author:  Lucien

  ==============================================================================
*/

#include <JuceHeader.h>
#include "SoundPlayer.h"
#include "Settings.h"

//==============================================================================
SoundPlayer::SoundPlayer(SoundPlayer::Mode m)
{
    soundPlayerMode = m;
    if (m == SoundPlayer::Mode::EightFaders)
        isEightPlayerMode = true;;
    juce::Timer::startTimer(50);
    //addKeyListener(this);

    if (soundPlayerMode == SoundPlayer::Mode::OnePlaylistOneCart)
        myPlaylists.add(new Playlist(0));
    else if (soundPlayerMode == SoundPlayer::Mode::EightFaders)
        myPlaylists.add(new Playlist(1));
    else
    {
        myPlaylists.add(new Playlist(0));
        myPlaylists.add(new Playlist(1));
        keyMappedSoundboard.reset(new KeyboardMappedSoundboard());
        addAndMakeVisible(keyMappedSoundboard.get());
        keyMappedSoundboard->setBounds(getLocalBounds());

    }


    myPlaylists[0]->playerStoppedID.addListener(this);
    myPlaylists[0]->fader1Name.addListener(this);
    myPlaylists[0]->fader2Name.addListener(this);
    myPlaylists[0]->mouseDragX.addListener(this);
    myPlaylists[0]->mouseDragY.addListener(this);
    myPlaylists[0]->mouseDragSource.addListener(this);
    myPlaylists[0]->mouseDraggedUp.addListener(this);
    myPlaylists[0]->setWantsKeyboardFocus(false);
    myPlaylists[0]->draggedPlayer.addListener(this);
    myPlaylists[0]->minimumPlayer.addListener(this);
    myPlaylists[0]->cuePlaylistBroadcaster->addChangeListener(this);
    myPlaylists[0]->playBroadcaster->addChangeListener(this);
    myPlaylists[0]->cuePlaylistActionBroadcaster->addActionListener(this);


    if (soundPlayerMode != SoundPlayer::Mode::KeyMap)
    {
        playlistViewport.setBounds(0, playersStartHeightPosition, 715, (getParentHeight() - playersStartHeightPosition));
        playlistViewport.setViewedComponent(myPlaylists[0], false);
        playlistViewport.setWantsKeyboardFocus(false);
        addAndMakeVisible(playlistViewport);
        playlistViewport.setMouseClickGrabsKeyboardFocus(false);

        if (!isEightPlayerMode)
        {
            addAndMakeVisible(addPlayerPlaylist);
            addPlayerPlaylist.setButtonText("Add Player");
            addPlayerPlaylist.onClick = [this] { myPlaylists[0]->addPlayer(myPlaylists[0]->players.size() - 1); };
            addAndMakeVisible(removePlayerPlaylist);
            removePlayerPlaylist.setButtonText("Remove Player");
            removePlayerPlaylist.onClick = [this] { myPlaylists[0]->removeButtonClicked();    };
        }

        //ADD CART
        myPlaylists.add(new Playlist(1));


        myPlaylists[1]->playerStoppedID.addListener(this);
        myPlaylists[1]->fader1Name.addListener(this);
        myPlaylists[1]->fader2Name.addListener(this);
        myPlaylists[1]->mouseDragX.addListener(this);
        myPlaylists[1]->mouseDragY.addListener(this);
        myPlaylists[1]->mouseDragSource.addListener(this);
        myPlaylists[1]->mouseDraggedUp.addListener(this);
        myPlaylists[1]->draggedPlayer.addListener(this);
        myPlaylists[1]->cuePlaylistBroadcaster->addChangeListener(this);
        myPlaylists[1]->playBroadcaster->addChangeListener(this);
        myPlaylists[1]->cuePlaylistActionBroadcaster->addActionListener(this);
        playlistbisViewport.setBounds(getParentWidth() / 2, playersStartHeightPosition,
            getParentWidth() / 2, (getParentHeight() - playersStartHeightPosition));
        playlistbisViewport.setViewedComponent(myPlaylists[1], false);
        playlistbisViewport.setWantsKeyboardFocus(false);
        addAndMakeVisible(playlistbisViewport);

        if (!isEightPlayerMode)
        {
            addAndMakeVisible(addPlayerCart);
            addPlayerCart.setButtonText("Add Player");
            addPlayerCart.onClick = [this] { myPlaylists[1]->addPlayer(myPlaylists[1]->players.size() - 1); };

            addAndMakeVisible(removePlayerCart);
            removePlayerCart.setButtonText("Remove Player");
            removePlayerCart.onClick = [this] { if (myPlaylists[1]->players.size() > 1)
                myPlaylists[1]->removePlayer(myPlaylists[1]->players.size() - 1);
            else
                myPlaylists[1]->players[0]->deleteFile();    };

            //ADD BUTTONS
            addAndMakeVisible(playersResetPosition);
            playersResetPosition.setButtonText("Reset");
            playersResetPosition.onClick = [this] { myPlaylists[0]->playersResetPositionClicked(); };

            addAndMakeVisible(playersPreviousPosition);
            playersPreviousPosition.setButtonText("Up");
            playersPreviousPosition.onClick = [this] { myPlaylists[0]->playersPreviousPositionClicked(); };

            addAndMakeVisible(playersNextPosition);
            playersNextPosition.setButtonText("Down");
            playersNextPosition.onClick = [this] { myPlaylists[0]->playersNextPositionClicked(); };
            playersNextPosition.setColour(1, juce::Colour(255, 163, 0));
        }
    }




    myMixer.addInputSource(&myPlaylists[0]->playlistMixer, false);
    myMixer.addInputSource(&myPlaylists[1]->playlistMixer, false);
    myCueMixer.addInputSource(&myPlaylists[0]->playlistCueMixer, false);
    myCueMixer.addInputSource(&myPlaylists[1]->playlistCueMixer, false);
    Settings::audioOutputModeValue.addListener(this);

    addAndMakeVisible(&loudnessBarComponent);

    metersInitialize();

    start1 = juce::Time::currentTimeMillis();

    mouseDragEnd(0);
    mouseDragEnd(1);

    playerSelectionChanged = new juce::ChangeBroadcaster();
    playlistLoadedBroadcaster = new juce::ChangeBroadcaster();

    newMeter.reset(new Meter(Meter::Mode::Stereo));
    addAndMakeVisible(newMeter.get());
    newMeter->setSkewFactor(1.5f);
    if (soundPlayerMode == SoundPlayer::Mode::KeyMap)
        keyMappedSoundboard->setBounds(getLocalBounds());


}

SoundPlayer::~SoundPlayer()
{
    mouseDragEnd(0);
    removeMouseListener(this);
    myMixer.removeAllInputs();
    myCueMixer.removeAllInputs();

    myPlaylists.clear(true);
    delete playerSelectionChanged;
    delete playlistLoadedBroadcaster;
    Settings::audioOutputModeValue.removeListener(this);
}

void SoundPlayer::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    for (int i = 0; i < myPlaylists.size(); i++)
    {
        myPlaylists[i]->getNextAudioBlock(bufferToFill);
    }
}

void SoundPlayer::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    myMixer.prepareToPlay(samplesPerBlockExpected, sampleRate);
    myCueMixer.prepareToPlay(samplesPerBlockExpected, sampleRate);

    actualSampleRate = sampleRate;
    actualSamplesPerBlockExpected = samplesPerBlockExpected;


    myPlaylists[0]->prepareToPlay(samplesPerBlockExpected, sampleRate);
    myPlaylists[1]->prepareToPlay(samplesPerBlockExpected, sampleRate);

    loudnessMeter.prepareToPlay(actualSampleRate, 2, actualSamplesPerBlockExpected, 20);

    meterSource.resize(2, sampleRate * 0.1 / samplesPerBlockExpected);
    cuemeterSource.resize(2, sampleRate * 0.1 / samplesPerBlockExpected);

    newMeter->prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void SoundPlayer::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
}

void SoundPlayer::resized()
{
    if (soundPlayerMode != SoundPlayer::Mode::KeyMap)
    {
        //setSize(getParentWidth(), getParentHeight());
        playlistViewport.setBounds(0, playersStartHeightPosition,
            (getParentWidth() / 2) - 50 - dragZoneWidth, (getHeight() - playersStartHeightPosition - bottomButtonsHeight));
        if (myPlaylists[0] != nullptr)
        {
            myPlaylists[0]->setSize(playlistViewport.getWidth() - 2, playlistViewport.getHeight() - 2);
        }
        if (!isEightPlayerMode)
            playlistbisViewport.setBounds(getParentWidth() / 2 + 50, playersStartHeightPosition,
                getParentWidth() / 2 - 50, (getHeight() - playersStartHeightPosition - bottomButtonsHeight));
        else
            playlistbisViewport.setBounds(getParentWidth() / 2 + 50, playersStartHeightPosition,
                getParentWidth() / 2 - 50, (getHeight() - playersStartHeightPosition - bottomButtonsHeight));
        if (myPlaylists[1] != nullptr)
        {
            myPlaylists[1]->setSize(playlistbisViewport.getWidth(), playlistbisViewport.getHeight() - 2);
            if (playlistViewport.isVerticalScrollBarShown())
            {
                myPlaylists[1]->scrollbarShown = true;
                myPlaylists[1]->setSize(playlistbisViewport.getWidth() - 20, playlistbisViewport.getHeight() - 2);
            }
            else
                myPlaylists[1]->scrollbarShown = true;
        }


        playersResetPosition.setBounds(playlistViewport.getWidth() - 200,
            0, upDownButtonsWidth, upDownButtonsHeight);
        playersPreviousPosition.setBounds(playlistViewport.getWidth() - 300,
            0, upDownButtonsWidth, upDownButtonsHeight);
        playersNextPosition.setBounds(playlistViewport.getWidth() - 100,
            0, upDownButtonsWidth, upDownButtonsHeight);

        addPlayerPlaylist.setBounds(playlistViewport.getWidth() - 200,
            playlistViewport.getHeight() + playersStartHeightPosition + 5, 100, bottomButtonsHeight);
        removePlayerPlaylist.setBounds(playlistViewport.getWidth() - 100,
            playlistViewport.getHeight() + playersStartHeightPosition + 5, 100, bottomButtonsHeight);

        addPlayerCart.setBounds(playlistbisViewport.getX(),
            playlistbisViewport.getHeight() + playersStartHeightPosition + 5, 100, bottomButtonsHeight);
        removePlayerCart.setBounds(playlistbisViewport.getX() + 100,
            playlistbisViewport.getHeight() + playersStartHeightPosition + 5, 100, bottomButtonsHeight);

        int cueMeterXStart = playlistViewport.getWidth();

        if (Settings::audioOutputMode == 1 || Settings::audioOutputMode == 3)
        {
            levelMeterHeight = std::min(getHeight() - playersStartHeightPosition - cuelevelMeterMinimumHeight,
                levelMeterMaximumHeight);
            meter.setBounds(cueMeterXStart, getHeight() - levelMeterHeight, 80,
                std::min(getHeight() - playersStartHeightPosition, levelMeterHeight));
            loudnessBarComponent.setBounds(meter.getBounds().getTopRight().getX() + 7,
                getHeight() - levelMeterHeight, 25, levelMeterHeight);
            cuelevelMeterHeight = std::min(getHeight() - playersStartHeightPosition - levelMeterHeight,
                cuelevelMeterMaximumHeight);
            int cueMeterYStart = meter.getPosition().getY() - cuelevelMeterHeight;
            cuemeter.setBounds(cueMeterXStart, cueMeterYStart, 80, cuelevelMeterHeight);
        }
        else if (Settings::audioOutputMode == 2)
        {
            levelMeterHeight = std::min(getHeight() - playersStartHeightPosition, levelMeterMaximumHeight);
            meter.setBounds(playlistViewport.getWidth(), getHeight() - levelMeterHeight,
                80, std::min(getHeight() - playersStartHeightPosition, levelMeterHeight));
            loudnessBarComponent.setBounds(meter.getBounds().getTopRight().getX() + 7,
                getHeight() - levelMeterHeight, 25, levelMeterHeight);

            //newMeter->setBounds(meter.getBounds());
        }
    }
    else
    {
        levelMeterHeight = getHeight() - 15;
        meter.setBounds(getWidth() - meterWidth - loudnessBarWidth - 7, getHeight() - levelMeterHeight,
            meterWidth, std::min(getHeight() - playersStartHeightPosition, levelMeterHeight));
        loudnessBarComponent.setBounds(meter.getBounds().getTopRight().getX() + 5,
            getHeight() - levelMeterHeight, loudnessBarWidth, levelMeterHeight);
        keyMappedSoundboard->setBounds(0, 15, getWidth() - (getWidth() - meter.getX()), getHeight());
    }


}


void SoundPlayer::timerCallback()
{
    if (Settings::audioOutputMode == 2 || Settings::audioOutputMode == 3)
    shortTermLoudness = loudnessMeter.getMomentaryLoudness();
    else if (Settings::audioOutputMode == 1)
        shortTermLoudness = loudnessMeter.getMomentaryLoudnessForIndividualChannels()[0];
    if (shortTermLoudness > 1)
        shortTermLoudness = 1;
    loudnessBarComponent.setLoudness(shortTermLoudness);


    //OSC SEND
    if (oscConnected)
    {
        if (myPlaylists[0]->players[myPlaylists[0]->fader1Player] != nullptr)
        {
            OSCsend(1, myPlaylists[0]->players[myPlaylists[0]->fader1Player]->remainingTimeString);
            sender.send("/1/gain1", juce::String(myPlaylists[0]->players[myPlaylists[0]->fader1Player]->volumeLabel.getText() + "dB"));
            delta1 = juce::Time::currentTimeMillis() - start1;
            if (delta1 > deltaTime)
            {

                juce::NormalisableRange<float>valueToSendNorm(0., juce::Decibels::decibelsToGain(Settings::maxFaderValueGlobal), 0.0001, Settings::skewFactorGlobal);
                sender.send("/1/fader1", valueToSendNorm.convertTo0to1(myPlaylists[0]->players[myPlaylists[0]->fader1Player]->volumeSlider.getValue()));

            }
        }
        else
        {
            OSCsend(1, "0:0");
        }

        if (myPlaylists[0]->players[myPlaylists[0]->fader2Player] != nullptr)
        {

            OSCsend(2, myPlaylists[0]->players[myPlaylists[0]->fader2Player]->remainingTimeString);
            sender.send("/1/gain2", juce::String(myPlaylists[0]->players[myPlaylists[0]->fader2Player]->volumeLabel.getText() + "dB"));
            delta2 = juce::Time::currentTimeMillis() - start2;
            if (delta2 > deltaTime)
            {
                juce::NormalisableRange<float>valueToSendNorm(0., juce::Decibels::decibelsToGain(Settings::maxFaderValueGlobal), 0.0001, Settings::skewFactorGlobal);
                sender.send("/1/fader2", valueToSendNorm.convertTo0to1(myPlaylists[0]->players[myPlaylists[0]->fader2Player]->volumeSlider.getValue()));
            }
        }
        else
        {
            OSCsend(2, "0:0");
        }

        if (myPlaylists[1]->players[myPlaylists[1]->fader1Player] != nullptr)
        {
            OSCsend(3, myPlaylists[1]->players[myPlaylists[1]->fader1Player]->remainingTimeString);
            sender.send("/1/gain3", juce::String(myPlaylists[1]->players[myPlaylists[1]->fader1Player]->volumeLabel.getText() + "dB"));
            delta3 = juce::Time::currentTimeMillis() - start3;
            if (delta3 > deltaTime)
            {
                juce::NormalisableRange<float>valueToSendNorm(0., juce::Decibels::decibelsToGain(Settings::maxFaderValueGlobal), 0.0001, Settings::skewFactorGlobal);
                sender.send("/1/fader3", valueToSendNorm.convertTo0to1(myPlaylists[1]->players[myPlaylists[1]->fader1Player]->volumeSlider.getValue()));
            }
        }
        else
        {
            OSCsend(3, "0:0");
        }

        if (myPlaylists[1]->players[myPlaylists[1]->fader2Player] != nullptr)
        {
            OSCsend(4, myPlaylists[1]->players[myPlaylists[1]->fader2Player]->remainingTimeString);
            sender.send("/1/gain4", juce::String(myPlaylists[1]->players[myPlaylists[1]->fader2Player]->volumeLabel.getText() + "dB"));
            delta4 = juce::Time::currentTimeMillis() - start4;
            if (delta4 > deltaTime)
            {
                juce::NormalisableRange<float>valueToSendNorm(0., juce::Decibels::decibelsToGain(Settings::maxFaderValueGlobal), 0.0001, Settings::skewFactorGlobal);
                sender.send("/1/fader4", valueToSendNorm.convertTo0to1(myPlaylists[1]->players[myPlaylists[1]->fader2Player]->volumeSlider.getValue()));
            }
        }
        else
        {
            OSCsend(4, "0:0");
        }


        juce::NormalisableRange<float>valueToSendNorm(0., 1., 0.001, 0.2);
        OSCsend(5, valueToSendNorm.convertTo0to1(meterSource.getRMSLevel(1)));
    }
}

void SoundPlayer::handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message)
{
    midiMessageValue = message.getControllerValue();
    midiMessageNumber = message.getControllerNumber();

    if (myPlaylists[0] != nullptr)
    {
        myPlaylists[0]->handleIncomingMidiMessage(source, message);
    }
    if (myPlaylists[1] != nullptr)
    {
        myPlaylists[1]->handleIncomingMidiMessage(source, message);
    }

    if (midiMessageNumber == 43 && midiMessageValue == 127)
    {
        myPlaylists[0]->playersPreviousPositionClicked();
    }
    else if (midiMessageNumber == 44 && midiMessageValue == 127)
    {
        myPlaylists[0]->playersNextPositionClicked();
    }
    else if (midiMessageNumber == 42 && midiMessageValue == 127)
    {
        myPlaylists[0]->playersResetPositionClicked();
    }
    else if (midiMessageNumber == 41 && midiMessageValue == 127)
    {
        myPlaylists[0]->spaceBarPressed();
    }

    //const juce::ScopedValueSetter<bool> scopedInputFlag(isAddingFromMidiInput, true);
}

void SoundPlayer::handleIncomingMidiMessageEightPlayers(juce::MidiInput* source, const juce::MidiMessage& message)
{
    midiMessageValue = message.getControllerValue();
    midiMessageNumber = message.getControllerNumber();
    if (myPlaylists[0] != nullptr && midiMessageNumber < 4 && midiMessageNumber >= 0)
    {
        myPlaylists[0]->handleIncomingMidiMessageEightPlayers(midiMessageValue, midiMessageNumber);
    }
    else if (myPlaylists[1] != nullptr && midiMessageNumber >= 4 && midiMessageNumber < 8)
    {
        myPlaylists[1]->handleIncomingMidiMessageEightPlayers(midiMessageValue, midiMessageNumber - 4);
    }
    else if (myPlaylists[0] != nullptr && midiMessageNumber >= 16 && midiMessageNumber <= 19)
    {
        myPlaylists[0]->handleTrimMidiMessage(midiMessageValue, midiMessageNumber - 16);
    }
    else if (myPlaylists[1] != nullptr && midiMessageNumber >= 20 && midiMessageNumber < 24)
    {
        myPlaylists[1]->handleTrimMidiMessage(midiMessageValue, midiMessageNumber - 20);
    }
}

bool SoundPlayer::keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent, KeyMapper* keyMapper)
{
    if (myPlaylists[Settings::editedPlaylist] != nullptr)
    {
        int keyCode = key.getKeyCode();
        auto* player = myPlaylists[Settings::editedPlaylist]->players[Settings::editedPlayer];

        if (player != nullptr)
        {
            if (keyMapper->getKeyMapping(7) == keyCode)
                    player->setStart();
            else if (keyMapper->getKeyMapping(8) == keyCode)
                    player->deleteStart();
            else if (keyMapper->getKeyMapping(9) == keyCode)
                    player->setStop();
            else if (keyMapper->getKeyMapping(10) == keyCode)
                    player->deleteStop();
            else if (keyMapper->getKeyMapping(4) == keyCode)
            {
                player->cueButtonClicked();
                return true;
            }
            else if (keyMapper->getKeyMapping(5) == keyCode)
            {
                player->cueTransport.setPosition(player->startTime);
                player->cueButtonClicked();
                return true;
            }
            else if (keyMapper->getKeyMapping(6) == keyCode)
            {
                player->cueTransport.setPosition(player->stopTime - 6);
                player->cueButtonClicked();
                return true;
            }
        }
    }
    if (key == juce::KeyPress::rightKey)
        copyPlayingSound();
    return false;
}

void SoundPlayer::positionViewport(int player)
{
    playlistbisViewport.setViewPosition(0, (player - 1) * 105);
}
void SoundPlayer::valueChanged(juce::Value& value)
{
    if (value.refersToSameSourceAs(myPlaylists[0]->minimumPlayer))
    {
        playlistViewport.setViewPosition(0, (((myPlaylists[0]->minimumPlayer).toString().getIntValue()) - 1) * 105);
    }
    else if (value.refersToSameSourceAs(Settings::audioOutputModeValue))
    {
        //channelsMapping();
        
        metersInitialize();
        resized();
    }
    else if (value.refersToSameSourceAs(myPlaylists[0]->fader1Value))
    {
        OSCsend(1, value.toString().getFloatValue());
    }
    else if (value.refersToSameSourceAs(myPlaylists[0]->playerStoppedID)) //playlist 1
    {
        if (value.toString().getIntValue() == myPlaylists[0]->fader1Player)
        {

            OSCsend(1, 0.);
            if (fader1OSCLaunched == true)
            {
                myPlaylists[0]->handleFader1OSC(0);
                fader1OSCLaunched = false;
            }
        }
        else if (value.toString().getIntValue() == myPlaylists[0]->fader2Player)
        {
            OSCsend(2, 0.);
            if (fader2OSCLaunched == true)
            {
                myPlaylists[0]->handleFader2OSC(0);
                fader2OSCLaunched = false;
            }
        }
    }
    else if (value.refersToSameSourceAs(myPlaylists[1]->playerStoppedID)) //playlist 1
    {
        if (value.toString().getIntValue() == myPlaylists[1]->fader1Player)
        {
            //DBG(value.toString().getIntValue());
            OSCsend(3, 0.);
            //myPlaylists[1]->handleFader3(0);

        }
        else if (value.toString().getIntValue() == myPlaylists[1]->fader2Player)
        {
            OSCsend(4, 0.);
            //myPlaylists[1]->handleFader4(0);
        }
    }
    else if (value.refersToSameSourceAs(myPlaylists[0]->fader1Name))
    {
        OSCsend(5, myPlaylists[0]->fader1Name.toString());
    }
    else if (value.refersToSameSourceAs(myPlaylists[0]->fader2Name))
    {
        OSCsend(6, myPlaylists[0]->fader2Name.toString());
    }
    else if (value.refersToSameSourceAs(myPlaylists[1]->fader1Name))
    {
        OSCsend(7, myPlaylists[1]->fader1Name.toString());
    }
    else if (value.refersToSameSourceAs(myPlaylists[1]->fader2Name))
    {
        OSCsend(8, myPlaylists[1]->fader2Name.toString());
    }
    else if (value.refersToSameSourceAs(myPlaylists[0]->mouseDragX))
    {
        mouseDragX = myPlaylists[0]->mouseDragX.toString().getIntValue();
        drawDragLines();
    }
    else if (value.refersToSameSourceAs(myPlaylists[0]->mouseDragY))
    {
        mouseDragY = myPlaylists[0]->mouseDragY.toString().getIntValue();
        drawDragLines();
    }
    else if (value.refersToSameSourceAs(myPlaylists[0]->mouseDragSource))
    {
        playerDragSource = myPlaylists[0]->mouseDragSource.toString().getIntValue();
        playlistDragSource = 0;
        playerSourceLatch = playerDragSource;
        //playlistSourceLatch = playlistDragSource;
        mouseDragGetInfos(0, playerDragSource);

    }
    else if (value.refersToSameSourceAs(myPlaylists[0]->mouseDraggedUp))
    {
        mouseDragged = myPlaylists[0]->mouseDraggedUp.toString().getIntValue();
        if (myPlaylists[0]->mouseDraggedUp.toString().getIntValue() == 1)
        {
            mouseDragDefinePlayer();
        }
    }
    else if (value.refersToSameSourceAs(myPlaylists[1]->mouseDragX))
    {
        mouseDragX = myPlaylists[1]->mouseDragX.toString().getIntValue();
        drawDragLines();
    }
    else if (value.refersToSameSourceAs(myPlaylists[1]->mouseDragY))
    {
        mouseDragY = myPlaylists[1]->mouseDragY.toString().getIntValue();
        drawDragLines();
    }
    else if (value.refersToSameSourceAs(myPlaylists[1]->mouseDragSource))
    {
        playerDragSource = myPlaylists[1]->mouseDragSource.toString().getIntValue();
        playerSourceLatch = playerDragSource;
        playlistDragSource = 1;
        mouseDragGetInfos(1, playerDragSource);
    }
    else if (value.refersToSameSourceAs(myPlaylists[1]->mouseDraggedUp))
    {
        mouseDragged = myPlaylists[1]->mouseDraggedUp.toString().getIntValue();
        if (myPlaylists[1]->mouseDraggedUp.toString().getIntValue() == 1)
        {
            mouseDragDefinePlayer();
        }
    }
    else if (value.refersToSameSourceAs(myPlaylists[0]->draggedPlayer))
    {
        draggedPlayer = -1;
        draggedPlayer = myPlaylists[0]->draggedPlayer.toString().getIntValue();
        draggedPlaylist = -1;
        draggedPlaylist = 0;
        myPlaylists[1]->draggedPlayer = -1;
        updateDraggedPlayerDisplay(draggedPlayer, 0);
    }
    else if (value.refersToSameSourceAs(myPlaylists[1]->draggedPlayer))
    {
        draggedPlayer = -1;
        draggedPlayer = myPlaylists[1]->draggedPlayer.toString().getIntValue();
        draggedPlaylist = -1;
        draggedPlaylist = 1;
        myPlaylists[0]->draggedPlayer = -1;
        updateDraggedPlayerDisplay(draggedPlayer, 1);
    }

}

void SoundPlayer::OSCInitialize()
{
    if (oscConnected == false)
    {
        if (receiver.connect(juce::int32(Settings::inOscPort)) && sender.connect(Settings::ipAdress, juce::int32(Settings::outOscPort)))
        {
            //oscStatusLabel.setText("OSC Connected", juce::NotificationType::dontSendNotification);
            //oscStatusLabel.setColour(juce::Label::textColourId, juce::Colours::green);
            //connectOSCButton.setButtonText("Disconnect OSC");
        }

        receiver.addListener(this);
        addListener(this, "/1/fader1");
        addListener(this, "/1/fader2");
        addListener(this, "/1/fader3");
        addListener(this, "/1/fader4");
        addListener(this, "/1/push1");
        addListener(this, "/1/push2");
        addListener(this, "/1/push3");
        addListener(this, "/1/push4");
        addListener(this, "/1/up");
        addListener(this, "/1/reset");
        addListener(this, "/1/down");
        addListener(this, "/2/multipush1");

        oscConnected = true;
    }
    else if (oscConnected == true)
    {
        OSCClean();
        if (receiver.disconnect())   // [13]
        {
            //connectOSCButton.setButtonText("Connect OSC");
            //oscStatusLabel.setText("OSC Not Connected", juce::NotificationType::dontSendNotification);
            //oscStatusLabel.setColour(juce::Label::textColourId, juce::Colours::red);
            oscConnected = false;
        }
    }
}
void SoundPlayer::OSCClean()
{
    if (oscConnected == true)
    {
        OSCsend(1, "");
        OSCsend(2, "");
        OSCsend(3, "");
        OSCsend(4, "");
        OSCsend(5, "");
        OSCsend(6, "");
        OSCsend(7, "");
        OSCsend(8, "");
        OSCsend(1, 0.);
        OSCsend(2, 0.);
        OSCsend(3, 0.);
        OSCsend(4, 0.);
    }
}
void SoundPlayer::oscMessageReceived(const juce::OSCMessage& message)
{
    //DBG("OSC message received");

    //DBG(message.getAddressPattern().toString());
    if (message.getAddressPattern().matches("/1/fader1"))
    {
        if (message.size() == 1 && message[0].isFloat32())
        {
            start1 = juce::Time::currentTimeMillis();
            float messagestring = message[0].getFloat32();
            juce::NormalisableRange<float>valueToSendNorm(0., 127., 0.001, Settings::skewFactorGlobal);
            myPlaylists[0]->handleFader1OSC(messagestring);

        }
    }
    else if (message.getAddressPattern().matches("/1/fader2"))
    {
        if (message.size() == 1 && message[0].isFloat32())
        {
            start2 = juce::Time::currentTimeMillis();
            float messagestring = message[0].getFloat32();
            juce::NormalisableRange<float>valueToSendNorm(0., 127., 0.001, Settings::skewFactorGlobal);
            myPlaylists[0]->handleFader2OSC(messagestring);
        }
    }
    else if (message.getAddressPattern().matches("/1/fader3"))
    {
        if (message.size() == 1 && message[0].isFloat32())
        {
            start3 = juce::Time::currentTimeMillis();
            float messagestring = message[0].getFloat32();
            juce::NormalisableRange<float>valueToSendNorm(0., 127., 0.001, Settings::skewFactorGlobal);
            myPlaylists[1]->handleFader3OSC(messagestring);
        }
    }
    else if (message.getAddressPattern().matches("/1/fader4"))
    {
        if (message.size() == 1 && message[0].isFloat32())
        {
            start4 = juce::Time::currentTimeMillis();
            float messagestring = message[0].getFloat32();
            juce::NormalisableRange<float>valueToSendNorm(0., 127., 0.001, Settings::skewFactorGlobal);
            myPlaylists[1]->handleFader4OSC(messagestring);
        }
    }
    else if (message.getAddressPattern().matches("/1/push1"))
    {
        if (message.size() == 1 && message[0].isFloat32())
        {
            float messagestring = message[0].getFloat32();
            if (messagestring == 1)
            {
                if (myPlaylists[0]->players[myPlaylists[0]->fader1Player] != nullptr)
                {
                    if (myPlaylists[0]->players[myPlaylists[0]->fader1Player]->fileLoaded == true)
                    {
                        if (myPlaylists[0]->fader1IsPlaying == false)
                        {
                            juce::NormalisableRange<float>valueToSendNorm(0., juce::Decibels::decibelsToGain(Settings::maxFaderValueGlobal), 0.001, Settings::skewFactorGlobal);
                            myPlaylists[0]->handleFader1OSC(0);
                            myPlaylists[0]->handleFader1OSC(valueToSendNorm.convertTo0to1(1.));
                            //sender.send("/1/fader1", valueToSendNorm.convertTo0to1(1.));
                            //sender.send("/1/fader1", valueToSendNorm.convertTo0to1(1.));
                            fader1OSCLaunched = true;
                        }
                    }
                }
            }

        }
    }
    else if (message.getAddressPattern().matches("/1/push2"))
    {
        if (message.size() == 1 && message[0].isFloat32())
        {
            float messagestring = message[0].getFloat32();
            if (messagestring == 1)
            {
                if (myPlaylists[0]->players[myPlaylists[0]->fader2Player] != nullptr)
                {
                    if (myPlaylists[0]->players[myPlaylists[0]->fader2Player]->fileLoaded == true)
                    {
                        if (myPlaylists[0]->fader2IsPlaying == false)
                        {
                            juce::NormalisableRange<float>valueToSendNorm(0., juce::Decibels::decibelsToGain(Settings::maxFaderValueGlobal), 0.001, Settings::skewFactorGlobal);
                            myPlaylists[0]->handleFader2OSC(0);
                            myPlaylists[0]->handleFader2OSC(valueToSendNorm.convertTo0to1(1.));
                            //sender.send("/1/fader2", valueToSendNorm.convertTo0to1(1.));
                            //sender.send("/1/fader2", valueToSendNorm.convertTo0to1(1.));
                            fader2OSCLaunched = true;

                        }
                    }
                }
            }

        }
    }
    else if (message.getAddressPattern().matches("/1/push3"))
    {
        if (message.size() == 1 && message[0].isFloat32())
        {
            float messagestring = message[0].getFloat32();
            if (messagestring == 1)
            {
                if (myPlaylists[1]->players[myPlaylists[1]->fader1Player] != nullptr)
                {
                    if (myPlaylists[1]->players[myPlaylists[1]->fader1Player]->fileLoaded == true)
                    {
                        if (myPlaylists[1]->fader1IsPlaying == false)
                        {
                            juce::NormalisableRange<float>valueToSendNorm(0., juce::Decibels::decibelsToGain(Settings::maxFaderValueGlobal), 0.001, Settings::skewFactorGlobal);
                            myPlaylists[1]->handleFader3OSC(0);
                            myPlaylists[1]->handleFader3OSC(valueToSendNorm.convertTo0to1(1.));
                            sender.send("/1/fader3", valueToSendNorm.convertTo0to1(1.));
                            sender.send("/1/fader3", valueToSendNorm.convertTo0to1(1.));
                        }
                    }
                }
            }
        }
    }
    else if (message.getAddressPattern().matches("/1/push4"))
    {
        if (message.size() == 1 && message[0].isFloat32())
        {
            float messagestring = message[0].getFloat32();
            if (messagestring == 1)
            {
                if (myPlaylists[1]->players[myPlaylists[1]->fader2Player] != nullptr)
                {
                    if (myPlaylists[1]->players[myPlaylists[1]->fader2Player]->fileLoaded == true)
                    {
                        if (myPlaylists[1]->fader2IsPlaying == false)
                        {
                            juce::NormalisableRange<float>valueToSendNorm(0., juce::Decibels::decibelsToGain(Settings::maxFaderValueGlobal), 0.001, Settings::skewFactorGlobal);
                            myPlaylists[1]->handleFader4OSC(0);
                            myPlaylists[1]->handleFader4OSC(valueToSendNorm.convertTo0to1(1.));
                            sender.send("/1/fader4", valueToSendNorm.convertTo0to1(1.));
                            sender.send("/1/fader4", valueToSendNorm.convertTo0to1(1.));
                        }
                    }
                }
            }
        }
    }
    else if (message.getAddressPattern().matches("/1/up"))
    {
        if (message.size() == 1 && message[0].isFloat32())
        {
            float messagestring = message[0].getFloat32();
            if (messagestring == 1)
            {
                myPlaylists[0]->playersPreviousPositionClicked();
            }

        }
    }
    else if (message.getAddressPattern().matches("/1/reset"))
    {
        if (message.size() == 1 && message[0].isFloat32())
        {
            float messagestring = message[0].getFloat32();
            if (messagestring == 1)
            {
                myPlaylists[0]->playersResetPositionClicked();
            }

        }
    }
    else if (message.getAddressPattern().matches("/1/down"))
    {
        if (message.size() == 1 && message[0].isFloat32())
        {
            float messagestring = message[0].getFloat32();
            if (messagestring == 1)
            {
                myPlaylists[0]->playersNextPositionClicked();
            }

        }
    }
}

void SoundPlayer::OSCsend(int destination, float value)
{
    if (oscConnected == true)
    {
        if (destination == 1)
        {

            float valueToSend = value / 127;
            juce::NormalisableRange<float>valueToSendNorm(0.0, 1, 0.001, 0.5, false);
            if (!sender.send("/1/fader1", valueToSendNorm.convertTo0to1(valueToSend)))
            {
            }
        }
        else if (destination == 2)
        {
            float valueToSend = value / 127;
            juce::NormalisableRange<float>valueToSendNorm(0.0, 1, 0.001, 0.5, false);
            if (!sender.send("/1/fader2", valueToSendNorm.convertTo0to1(valueToSend)))
            {
            }
        }
        else if (destination == 3)
        {
            float valueToSend = value / 127;
            juce::NormalisableRange<float>valueToSendNorm(0.0, 1, 0.001, 0.5, false);
            if (!sender.send("/1/fader3", valueToSendNorm.convertTo0to1(valueToSend)))
            {
            }
        }
        else if (destination == 4)
        {
            float valueToSend = value / 127;
            juce::NormalisableRange<float>valueToSendNorm(0.0, 1, 0.001, 0.5, false);
            if (!sender.send("/1/fader4", valueToSendNorm.convertTo0to1(valueToSend)))
            {
            }
        }
        else if (destination == 5)
        {
            if (!sender.send("/1/meter", value))
            {

            }
        }
    }
}



void SoundPlayer::OSCsend(int destination, juce::String string)
{
    if (oscConnected == true)
    {
        if (destination == 1)
        {
            if (!sender.send("/1/remaining1", string))
            {
            }
        }
        else if (destination == 2)
        {
            if (!sender.send("/1/remaining2", string))
            {
            }
        }
        else if (destination == 3)
        {
            if (!sender.send("/1/remaining3", string))
            {
            }
        }
        else if (destination == 4)
        {
            if (!sender.send("/1/remaining4", string))
            {
            }
        }
        else if (destination == 5)
        {
            if (!sender.send("/1/soundname1", string))
            {
            }
        }
        else if (destination == 6)
        {
            if (!sender.send("/1/soundname2", string))
            {
            }
        }
        else if (destination == 7)
        {
            if (!sender.send("/1/soundname3", string))
            {
            }
        }
        else if (destination == 8)
        {
            if (!sender.send("/1/soundname4", string))
            {
            }
        }
    }
}



void SoundPlayer::mouseDragGetInfos(int playlistSource, int playerID)
{
    auto* player = myPlaylists[playlistSource]->players[playerID];
    if (player != nullptr)
    {
        draggedFilterParameters = player->getFilterProcessor().getGlobalFilterParameters();
        draggedFxBypassed = player->getBypassed();
        draggedNormalized = player->getHasBeenNormalized();
        draggedPath = player->getFilePath();
        draggedTrim = player->getTrimVolume();
        draggedName = player->getName();
        draggedHpfEnabled = player->isHpfEnabled();
        draggedStartTimeSet = player->startTimeSet;
        draggedStopTimeSet = player->stopTimeSet;
        draggedStartTime = player->getStart();
        draggedStopTime = player->getStop();
        mouseDragged = true;
    }
}

void SoundPlayer::drawDragLines()
{
    if (soundPlayerMode == SoundPlayer::Mode::OnePlaylistOneCart)
    {
        if (myPlaylists[playlistDragSource] != nullptr)
        {
            myPlaylists[playlistDragSource]->fileDragPlayerSource = playerDragSource;
            myPlaylists[playlistDragSource]->dragPaintSourceRectangle = true;
            for (auto i = 0; i < myPlaylists[0]->players.size(); i++)
            {
                if ((getMouseXYRelative().getX() < playlistViewport.getWidth())
                    && (getMouseXYRelative().getX() > 0)
                    && (getMouseXYRelative().getY() > (playlistViewport.getPosition().getY() - playlistViewport.getViewPositionY() + (105 * i) + dragZoneHeight)
                        && (getMouseXYRelative().getY() < (playlistViewport.getPosition().getY() - playlistViewport.getViewPositionY() + (105 * (i + 1) - dragZoneHeight)))))
                {
                    if (playlistDragSource == 1)
                    {
                        playerMouseDragUp = i;
                        playlistDragDestination = 0;
                        myPlaylists[0]->fileDragPlayerDestination = playerMouseDragUp;
                        destinationPlayerFound = true;
                        myPlaylists[0]->fileDragPaintRectangle = false;
                    }
                    myPlaylists[0]->fileDragPaintLine = false;
                    myPlaylists[1]->fileDragPaintRectangle = false;
                    myPlaylists[1]->fileDragPaintLine = false;
                    insertTop = false;
                    myPlaylists[0]->repaint();

                }
                else if (getMouseXYRelative().getY() > (playlistViewport.getPosition().getY() - playlistViewport.getViewPositionY() + (105 * i) + 100 - dragZoneHeight)
                    && getMouseXYRelative().getY() < (playlistViewport.getPosition().getY() - playlistViewport.getViewPositionY() + (105 * i) + 100 + dragZoneHeight)
                    && getMouseXYRelative().getX() < playlistViewport.getWidth()
                    && (getMouseXYRelative().getX() > 0))
                {
                    playerMouseDragUp = i;
                    playlistDragDestination = 0;
                    if (soundPlayerMode != Mode::EightFaders)
                        myPlaylists[0]->fileDragPaintLine = true;
                    myPlaylists[0]->fileDragPaintRectangle = false;
                    myPlaylists[1]->fileDragPaintRectangle = false;
                    myPlaylists[1]->fileDragPaintLine = false;
                    myPlaylists[0]->fileDragPlayerDestination = playerMouseDragUp;
                    myPlaylists[0]->repaint();
                    if (soundPlayerMode != Mode::EightFaders)
                        destinationPlayerFound = true;
                    insertTop = false;
                }
                else if (getMouseXYRelative().getY() < (playlistViewport.getPosition().getY() - playlistViewport.getViewPositionY() + dragZoneHeight)
                    && getMouseXYRelative().getY() > (playlistViewport.getPosition().getY() - playlistViewport.getViewPositionY() - dragZoneHeight)
                    && getMouseXYRelative().getX() < playlistViewport.getWidth()
                    && (getMouseXYRelative().getX() > 0))
                {
                    if (soundPlayerMode != Mode::EightFaders)
                        insertTop = true;
                    playerMouseDragUp = -1;
                    playlistDragDestination = 0;
                    if (!isEightPlayerMode)
                        myPlaylists[0]->fileDragPaintLine = true;
                    myPlaylists[0]->fileDragPaintRectangle = false;
                    myPlaylists[1]->fileDragPaintRectangle = false;
                    myPlaylists[1]->fileDragPaintLine = false;
                    myPlaylists[0]->fileDragPlayerDestination = playerMouseDragUp;
                    myPlaylists[0]->repaint();
                    if (soundPlayerMode != Mode::EightFaders)
                        destinationPlayerFound = true;
                }
                else if (getMouseXYRelative().getY() < playlistViewport.getPosition().getY()
                    || getMouseXYRelative().getY() > myPlaylists[0]->players.size() * 105
                    || getMouseXYRelative().getX() > playlistViewport.getWidth()
                    || (getMouseXYRelative().getX() < 0))
                {
                    myPlaylists[0]->fileDragPaintRectangle = false;
                    myPlaylists[0]->fileDragPaintLine = false;
                    myPlaylists[1]->fileDragPaintRectangle = false;
                    myPlaylists[1]->fileDragPaintLine = false;
                    destinationPlayerFound = false;
                    playerMouseDragUp = -1;
                    playlistDragDestination = -1;
                    myPlaylists[0]->repaint();
                    insertTop = false;
                }
                myPlaylists[1]->repaint();
            }
            if (destinationPlayerFound == false)
            {
                for (auto i = 0; i < myPlaylists[1]->players.size(); i++)
                {
                    if ((getMouseXYRelative().getX() > playlistbisViewport.getPosition().getX())
                        && (getMouseXYRelative().getX() < getParentWidth())
                        && (getMouseXYRelative().getY() > (playlistbisViewport.getPosition().getY() - playlistbisViewport.getViewPositionY() + (105 * i) + dragZoneHeight)
                            && (getMouseXYRelative().getY() < (playlistbisViewport.getPosition().getY() - playlistbisViewport.getViewPositionY() + (105 * (i + 1) - dragZoneHeight)))))
                    {
                        playerMouseDragUp = i;
                        playlistDragDestination = 1;
                        myPlaylists[1]->fileDragPaintRectangle = false;
                        myPlaylists[1]->fileDragPaintLine = false;
                        myPlaylists[0]->fileDragPaintRectangle = false;
                        myPlaylists[0]->fileDragPaintLine = false;
                        myPlaylists[1]->fileDragPlayerDestination = playerMouseDragUp;
                        myPlaylists[1]->repaint();
                        destinationPlayerFound = true;
                        insertTop = false;
                    }
                    else if (getMouseXYRelative().getY() > (playlistbisViewport.getPosition().getY() - playlistbisViewport.getViewPositionY() + (105 * i) + 100 - dragZoneHeight)
                        && getMouseXYRelative().getY() < (playlistbisViewport.getPosition().getY() - playlistbisViewport.getViewPositionY() + (105 * i) + 100 + dragZoneHeight)
                        && getMouseXYRelative().getX() > playlistbisViewport.getPosition().getX()
                        && (getMouseXYRelative().getX() < getParentWidth()))
                    {
                        playerMouseDragUp = i;
                        playlistDragDestination = 1;
                        if (!isEightPlayerMode)
                            myPlaylists[1]->fileDragPaintLine = true;
                        myPlaylists[1]->fileDragPaintRectangle = false;
                        myPlaylists[0]->fileDragPaintRectangle = false;
                        myPlaylists[0]->fileDragPaintLine = false;
                        myPlaylists[1]->fileDragPlayerDestination = playerMouseDragUp;
                        myPlaylists[1]->repaint();
                        if (!isEightPlayerMode)
                            destinationPlayerFound = true;
                        insertTop = false;
                    }
                    else if (getMouseXYRelative().getY() < (playlistbisViewport.getPosition().getY() - playlistbisViewport.getViewPositionY() + dragZoneHeight)
                        && getMouseXYRelative().getY() > (playlistbisViewport.getPosition().getY() - playlistbisViewport.getViewPositionY() - dragZoneHeight)
                        && getMouseXYRelative().getX() > playlistbisViewport.getPosition().getX()
                        && (getMouseXYRelative().getX() < getParentWidth()))
                    {
                        if (!isEightPlayerMode)
                            insertTop = true;
                        playerMouseDragUp = -1;
                        playlistDragDestination = 1;
                        if (!isEightPlayerMode)
                            myPlaylists[1]->fileDragPaintLine = true;
                        myPlaylists[1]->fileDragPaintRectangle = false;
                        myPlaylists[0]->fileDragPaintRectangle = false;
                        myPlaylists[0]->fileDragPaintLine = false;
                        myPlaylists[1]->fileDragPlayerDestination = playerMouseDragUp;
                        myPlaylists[1]->repaint();
                        destinationPlayerFound = true;
                    }
                    else if (getMouseXYRelative().getY() < playlistbisViewport.getPosition().getY()
                        || getMouseXYRelative().getY() > myPlaylists[1]->players.size() * 105
                        || getMouseXYRelative().getX() < playlistbisViewport.getPosition().getX()
                        || (getMouseXYRelative().getX() > getParentWidth()))
                    {
                        myPlaylists[1]->fileDragPaintRectangle = false;
                        myPlaylists[1]->fileDragPaintLine = false;
                        myPlaylists[0]->fileDragPaintRectangle = false;
                        myPlaylists[0]->fileDragPaintLine = false;
                        insertTop = false;
                        destinationPlayerFound = false;
                        playerMouseDragUp = -1;
                        playlistDragDestination = -1;
                        myPlaylists[1]->repaint();
                    }
                    myPlaylists[0]->repaint();
                }
            }
        }
    }
    else if (soundPlayerMode == Mode::EightFaders)
    {
        int a = 1;
        int destinationPlaylist = -1;
        int destinationPlayer = -1;
        auto* sourcePlayer = myPlaylists[playlistDragSource]->players[playerDragSource];
        if (sourcePlayer != nullptr)
        {
            if (sourcePlayer->isFileLoaded())
            {
                for (int p = 0; p < myPlaylists.size(); p++)
                {
                    for (int i = 0; i < myPlaylists[p]->players.size(); i++)
                    {
                        auto* player = myPlaylists[p]->players[i];
                        juce::Point<int> mouseGlobal = localPointToGlobal(getMouseXYRelative());
                        if (player->getScreenBounds().contains(mouseGlobal)
                            && !player->isFileLoaded())
                        {
                            destinationPlaylist = p;
                            playlistDragDestination = p;
                            destinationPlayer = i;
                            playerMouseDragUp = i;
                            myPlaylists[p]->fileDragPaintRectangle = true;
                            myPlaylists[p]->fileDragPlayerDestination = destinationPlayer;
                            myPlaylists[0]->repaint();
                            myPlaylists[1]->repaint();
                            break;
                        }
                        else
                        {
                            myPlaylists[p]->fileDragPaintRectangle = false;
                            myPlaylists[p]->fileDragPlayerDestination = -1;
                            //playlistDragDestination = -1;
                            myPlaylists[0]->repaint();
                            myPlaylists[1]->repaint();
                        }
                    }
                }
            }
        }
    }
}

void SoundPlayer::mouseDragDefinePlayer()
{
    if (soundPlayerMode == Mode::OnePlaylistOneCart)
    {
        if (playlistDragDestination != -1)
            mouseDragSetInfos(playlistDragDestination, playerMouseDragUp);
        else
        {
            mouseDragEnd(0);
            mouseDragEnd(1);
        }
    }
    else if (soundPlayerMode == Mode::EightFaders)
    {
        if (playlistDragDestination != -1)
        {
            auto* playerSource = myPlaylists[playlistDragSource]->players[playerDragSource];
            auto* playerDest = myPlaylists[playlistDragDestination]->players[playerMouseDragUp];
            if (playerSource != nullptr && playerDest != nullptr)
            {
                if (playerSource->isFileLoaded()
                    && !playerDest->isFileLoaded()
                    && myPlaylists[playlistDragDestination]->fileDragPaintRectangle == true)
                {
                    mouseDragGetInfos(playlistDragSource, playerDragSource);
                    setSoundInfos(playlistDragDestination, playerMouseDragUp);
                    myPlaylists[playlistDragSource]->players[playerDragSource]->deleteFile();
                    mouseDragEnd(playlistDragDestination);
                }
            }
        }
    }

}

bool SoundPlayer::isDraggable(int playlistSource, int playerSource, int playlistDestination, int playerDestination)
{
    return true;
}

void SoundPlayer::mouseDragSetInfos(int playlistDestination, int playerIdDestination)
{
    auto* destinationPlaylist = myPlaylists[playlistDestination];
    auto* sourcePlaylist = myPlaylists[playlistDragSource];
    auto sourcePlayer = myPlaylists[playlistDragSource]->players[playerDragSource];
    if (destinationPlaylist != nullptr
        && myPlaylists[playlistDragSource] != nullptr)
    {
        if (insertTop == true)
        {
            if (isDraggable(playlistDragSource, playerDragSource, playlistDestination, playerIdDestination))
            {
                if (myPlaylists[playlistDestination] == myPlaylists[playlistDragSource])
                {
                    reassignFaders(playerIdDestination, playerDragSource, playlistDestination);
                    destinationPlaylist->players.move(playerDragSource, 0);
                    destinationPlaylist->rearrangePlayers();
                }
                else
                {
                    myPlaylists[playlistDestination]->fader1Player++;
                    myPlaylists[playlistDestination]->fader2Player++;

                    myPlaylists[playlistDestination]->addPlayer(playerIdDestination);
                    playerIdDestination++;
                    setSoundInfos(playlistDestination, playerIdDestination);
                }
                //clearDragInfos();
            }
        }
        if (destinationPlaylist->players[playerIdDestination] != myPlaylists[playlistDragSource]->players[playerDragSource])
        {
            if (isDraggable(playlistDragSource, playerDragSource, playlistDestination, playerIdDestination))
            {
                if (destinationPlaylist->players[playerIdDestination] != nullptr && mouseDragged)
                {
                    if (destinationPlaylist == myPlaylists[playlistDragSource]) //IF BOTH PLAYERS ARE ON THE SAME PLAYLIST
                    {
                        if (destinationPlaylist->fileDragPaintLine)
                        {
                            if (playlistDestination == 0)
                            {
                                DBG("player destination : " << playerIdDestination);
                                reassignFaders(playerIdDestination, playerDragSource, playlistDestination);
                                if (insertTop)
                                    destinationPlaylist->players.move(playerDragSource, 0);
                                if (playerIdDestination > playerDragSource)
                                    destinationPlaylist->players.move(playerDragSource, playerIdDestination);
                                else if (playerIdDestination < playerDragSource)
                                  destinationPlaylist->players.move(playerDragSource, playerIdDestination + 1);

                                destinationPlaylist->rearrangePlayers();
                            }
                            else if (playlistDestination == 1)
                            {
                                myPlaylists[playlistDestination]->players.move(playerDragSource, playerIdDestination + 1);
                                destinationPlaylist->assignLeftFaderButtons.move(playerDragSource, playerIdDestination + 1);
                                destinationPlaylist->assignRightFaderButtons.move(playerDragSource, playerIdDestination + 1);
                                myPlaylists[playlistDestination]->rearrangePlayers();
                            }

                        }

                    }
                    else if (myPlaylists[playlistDestination] != myPlaylists[playlistDragSource]) //IF PLAYERS ARE ON DIFFERENTS PLAYLISTS
                    {
                        if (myPlaylists[playlistDestination]->fileDragPaintLine)
                        {
                            if (playlistDestination == 0)
                            {
                                if (playerIdDestination < myPlaylists[playlistDestination]->fader1Player && myPlaylists[playlistDestination]->fader1IsPlaying)
                                {
                                    myPlaylists[playlistDestination]->fader1Player++;
                                }
                                if (playerIdDestination < myPlaylists[playlistDestination]->fader2Player && myPlaylists[playlistDestination]->fader2IsPlaying)
                                {
                                    myPlaylists[playlistDestination]->fader2Player++;
                                }
                                if (myPlaylists[0]->fader2Player == myPlaylists[0]->fader1Player && !myPlaylists[0]->fader2IsPlaying && myPlaylists[0]->fader1IsPlaying)
                                    myPlaylists[0]->fader2Player++;
                                if (myPlaylists[0]->fader1Player == myPlaylists[0]->fader2Player && !myPlaylists[0]->fader1IsPlaying && myPlaylists[0]->fader2IsPlaying)
                                    myPlaylists[0]->fader1Player++;

                                myPlaylists[playlistDestination]->addPlayer(playerIdDestination);
                                playerIdDestination++;
                                setSoundInfos(playlistDestination, playerIdDestination);
                            }
                            else if (playlistDestination == 1)
                            {

                                if (playerIdDestination < myPlaylists[1]->fader1Player)
                                    myPlaylists[1]->fader1Player++;
                                if (playerIdDestination < myPlaylists[1]->fader2Player)
                                    myPlaylists[1]->fader2Player++;
                                myPlaylists[playlistDestination]->addPlayer(playerIdDestination);
                                playerIdDestination++;
                                setSoundInfos(playlistDestination, playerIdDestination);
                            }
                        }

                        if (myPlaylists[playlistDragSource]->mouseCtrlModifier == true)
                            checkAndRemovePlayer(playlistDragSource, playerDragSource);
                    }
                }
            }
        }
    }
    mouseDragEnd(playlistDestination);
}

void SoundPlayer::setSoundInfos(int playlistDestination, int playerIdDestination)
{
    playerSelectionChanged->sendChangeMessage();
    auto* player = myPlaylists[playlistDestination]->players[playerIdDestination];
    player->setHasBeenNormalized(draggedNormalized);
    player->loadFile(draggedPath, false);
    player->setTrimVolume(draggedTrim);
    player->setName(draggedName);
    player->enableHPF(draggedHpfEnabled);
    if (draggedStartTimeSet)
        player->setStartTime(draggedStartTime);
    if (draggedStopTimeSet)
        player->setStopTime(draggedStopTime);
    player->setFilterParameters(draggedFilterParameters);
    player->bypassFX(draggedFxBypassed, false);
    clearDragInfos();
}

void SoundPlayer::clearDragInfos()
{
    draggedPath = "";
    draggedName = "";
    draggedTrim = 0.0;
    draggedHpfEnabled = false;
    draggedStartTimeSet = false;
    draggedStopTimeSet = false;
    draggedStartTime = 0.0;
    draggedStopTime = 0.0;
    draggedNormalized = false;
    mouseDragged = false;
    draggedFxBypassed = false;
    draggedFilterParameters = FilterProcessor::makeDefaultFilter();
    myPlaylists[playlistDragSource]->mouseDragSource.setValue(-1);
}

void SoundPlayer::mouseDragEnd(int playlistDestination)
{
    if (myPlaylists[playlistDragSource] != nullptr && myPlaylists[playlistDestination] != nullptr)
    {
        myPlaylists[playlistDragSource]->mouseCtrlModifier = false;
        myPlaylists[0]->fileDragPaintRectangle = false;
        myPlaylists[0]->fileDragPaintLine = false;
        myPlaylists[1]->fileDragPaintRectangle = false;
        myPlaylists[1]->fileDragPaintLine = false;
        myPlaylists[playlistDragSource]->dragPaintSourceRectangle = false;
        myPlaylists[playlistDestination]->fileDragPlayerDestination = -1;
        myPlaylists[playlistDragSource]->repaint();
        myPlaylists[playlistDestination]->repaint();
        insertTop = false;
        myPlaylists[playlistDragSource]->mouseAltModifier = false;
        myPlaylists[playlistDragSource]->mouseDraggedStartPositionOK = false;
    }
}

void SoundPlayer::checkAndRemovePlayer(int playlist, int player)
{
    if (myPlaylists[playlist] != nullptr)
    {
        if (myPlaylists[playlist]->players.size() > 1)
            myPlaylists[playlist]->removePlayer(player);
        else
            myPlaylists[playlist]->players[player]->deleteFile();
    }
}


void SoundPlayer::reassignFaders(int playerIdDestination, int playerDragSource, int playlistDestination)
{
    auto* destinationPlaylist = myPlaylists[playlistDestination];
    if (playerIdDestination > playerDragSource)
    {
        if (playerDragSource < destinationPlaylist->fader1Player
            && playerIdDestination >= destinationPlaylist->fader1Player)
            destinationPlaylist->fader1Player--;
        if (playerDragSource < destinationPlaylist->fader2Player
            && playerIdDestination >= destinationPlaylist->fader2Player)
            destinationPlaylist->fader2Player--;
    }
    else if (playerIdDestination < playerDragSource)
    {

        if (playerDragSource == destinationPlaylist->fader1Player
            && destinationPlaylist->fader1IsPlaying)
        {
            destinationPlaylist->fader1Player = playerIdDestination + 1;
        }
        else if (playerDragSource == destinationPlaylist->fader2Player
            && destinationPlaylist->fader2IsPlaying)
        {
            destinationPlaylist->fader2Player = playerIdDestination + 1;
        }

        if (playerIdDestination < destinationPlaylist->fader1Player
            && playerDragSource < destinationPlaylist->fader1Player)
        {
        }
        else if (playerIdDestination < destinationPlaylist->fader1Player)
        {
            destinationPlaylist->fader1Player++;
            if (!destinationPlaylist->fader1IsPlaying && playerIdDestination >= destinationPlaylist->fader2Player)
                destinationPlaylist->fader1Player--;

        }
        if (playerIdDestination < destinationPlaylist->fader2Player
            && playerDragSource < destinationPlaylist->fader2Player)
        {
        }
        else if (playerIdDestination < destinationPlaylist->fader2Player)
        {
            destinationPlaylist->fader2Player++;
            if (!destinationPlaylist->fader2IsPlaying && playerIdDestination >= destinationPlaylist->fader1Player)
                destinationPlaylist->fader2Player--;
        }
    }

}

void SoundPlayer::copyPlayingSound()
{
    if (myPlaylists[0]->fader1IsPlaying)
    {
        if (myPlaylists[0]->players[myPlaylists[0]->fader1Player] != nullptr)
        {
            myPlaylists[1]->addPlayer(myPlaylists[1]->players.size() - 1);
            myPlaylists[1]->players.getLast()->loadFile(myPlaylists[0]->players[myPlaylists[0]->fader1Player]->getFilePath(), false);
            myPlaylists[1]->players.getLast()->setTrimVolume(myPlaylists[0]->players[myPlaylists[0]->fader1Player]->getTrimVolume());
            myPlaylists[1]->players.getLast()->setName(myPlaylists[0]->players[myPlaylists[0]->fader1Player]->getName());
            myPlaylists[1]->players.getLast()->enableHPF(myPlaylists[0]->players[myPlaylists[0]->fader1Player]->isHpfEnabled());
            if (myPlaylists[0]->players[myPlaylists[0]->fader1Player]->startTimeSet)
                myPlaylists[1]->players.getLast()->setStartTime(myPlaylists[0]->players[myPlaylists[0]->fader1Player]->getStart());
            if (myPlaylists[0]->players[myPlaylists[0]->fader1Player]->stopTimeSet)
                myPlaylists[1]->players.getLast()->setStopTime(myPlaylists[0]->players[myPlaylists[0]->fader1Player]->getStop());
        }
    }
    if (myPlaylists[0]->fader2IsPlaying)
    {
        if (myPlaylists[0]->players[myPlaylists[0]->fader2Player] != nullptr)
        {
            myPlaylists[1]->addPlayer(myPlaylists[1]->players.size() - 1);
            myPlaylists[1]->players.getLast()->loadFile(myPlaylists[0]->players[myPlaylists[0]->fader2Player]->getFilePath(), false);
            myPlaylists[1]->players.getLast()->setTrimVolume(myPlaylists[0]->players[myPlaylists[0]->fader2Player]->getTrimVolume());
            myPlaylists[1]->players.getLast()->setName(myPlaylists[0]->players[myPlaylists[0]->fader2Player]->getName());
            myPlaylists[1]->players.getLast()->enableHPF(myPlaylists[0]->players[myPlaylists[0]->fader2Player]->isHpfEnabled());
            if (myPlaylists[0]->players[myPlaylists[0]->fader2Player]->startTimeSet)
                myPlaylists[1]->players.getLast()->setStartTime(myPlaylists[0]->players[myPlaylists[0]->fader2Player]->getStart());
            if (myPlaylists[0]->players[myPlaylists[0]->fader2Player]->stopTimeSet)
                myPlaylists[1]->players.getLast()->setStopTime(myPlaylists[0]->players[myPlaylists[0]->fader2Player]->getStop());
        }
    }
    if (myPlaylists[0]->spaceBarIsPlaying)
    {
        if (myPlaylists[0]->players[myPlaylists[0]->spaceBarPlayerId] != nullptr)
        {
            myPlaylists[1]->addPlayer(myPlaylists[1]->players.size() - 1);
            myPlaylists[1]->players.getLast()->loadFile(myPlaylists[0]->players[myPlaylists[0]->spaceBarPlayerId]->getFilePath(), false);
            myPlaylists[1]->players.getLast()->setTrimVolume(myPlaylists[0]->players[myPlaylists[0]->spaceBarPlayerId]->getTrimVolume());
            myPlaylists[1]->players.getLast()->setName(myPlaylists[0]->players[myPlaylists[0]->spaceBarPlayerId]->getName());
            myPlaylists[1]->players.getLast()->enableHPF(myPlaylists[0]->players[myPlaylists[0]->spaceBarPlayerId]->isHpfEnabled());
            if (myPlaylists[0]->players[myPlaylists[0]->spaceBarPlayerId]->startTimeSet)
                myPlaylists[1]->players.getLast()->setStartTime(myPlaylists[0]->players[myPlaylists[0]->spaceBarPlayerId]->getStart());
            if (myPlaylists[0]->players[myPlaylists[0]->spaceBarPlayerId]->stopTimeSet)
                myPlaylists[1]->players.getLast()->setStopTime(myPlaylists[0]->players[myPlaylists[0]->spaceBarPlayerId]->getStop());
        }
    }
}

void SoundPlayer::metersInitialize()
{
    lnf.setColour(foleys::LevelMeter::lmBackgroundColour, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    lnf.setColour(foleys::LevelMeter::lmMeterGradientLowColour, juce::Colours::green);
    lnf.setColour(foleys::LevelMeter::lmMeterGradientMaxColour, juce::Colours::red);
    lnf.setColour(foleys::LevelMeter::lmMeterGradientMidColour, juce::Colours::orange);
    int audioMode = Settings::audioOutputMode;

    if (audioMode == 1)
    {
        meter.setLookAndFeel(&lnf);
        meter.setMeterSource(&meterSource);
        addAndMakeVisible(meter);
        meter.setMeterFlags(foleys::LevelMeter::MeterFlags::SingleChannel);
        meter.setSelectedChannel(0);

        cuemeter.setLookAndFeel(&lnf);
        cuemeter.setMeterSource(&cuemeterSource);
        addAndMakeVisible(cuemeter);
        cuemeter.setMeterFlags(foleys::LevelMeter::MeterFlags::SingleChannel);
        cuemeter.setSelectedChannel(1);
    }
    else if (audioMode == 2)
    {
        meter.setLookAndFeel(&lnf);
        meter.setMeterSource(&meterSource);
        addAndMakeVisible(meter);
        meter.setMeterFlags(foleys::LevelMeter::MeterFlags::Default);
        removeChildComponent(&cuemeter);
    }

    else if (audioMode == 3)
    {
        meter.setLookAndFeel(&lnf);
        meter.setMeterSource(&meterSource);
        addAndMakeVisible(meter);
        meter.setMeterFlags(foleys::LevelMeter::MeterFlags::Default);

        cuemeter.setLookAndFeel(&lnf);
        cuemeter.setMeterSource(&cuemeterSource);
        addAndMakeVisible(cuemeter);
        cuemeter.setMeterFlags(foleys::LevelMeter::MeterFlags::Default);
    }


    int cueMeterXStart = playlistViewport.getWidth();
    if (Settings::audioOutputMode == 1 || Settings::audioOutputMode == 3)
    {
        levelMeterHeight = std::min(getHeight() - playersStartHeightPosition - cuelevelMeterMinimumHeight, levelMeterMaximumHeight);
        meter.setBounds(cueMeterXStart, getHeight() - levelMeterHeight, 80, std::min(getHeight() - playersStartHeightPosition, levelMeterHeight));
        cuelevelMeterHeight = std::min(getHeight() - playersStartHeightPosition - levelMeterHeight, cuelevelMeterMaximumHeight);
        int cueMeterYStart = meter.getPosition().getY() - cuelevelMeterHeight;
        cuemeter.setBounds(cueMeterXStart, cueMeterYStart, 80, cuelevelMeterHeight);
    }
    else if (Settings::audioOutputMode == 2)
    {
        levelMeterHeight = std::min(getHeight() - playersStartHeightPosition, levelMeterMaximumHeight);
        meter.setBounds(playlistViewport.getWidth(), getHeight() - levelMeterHeight, 80, std::min(getHeight() - playersStartHeightPosition, levelMeterHeight));
    }
}


void SoundPlayer::savePlaylist()
{
    juce::XmlElement* playlist = new juce::XmlElement("PLAYLIST");
    for (int i = 0; i < myPlaylists[0]->players.size(); ++i)
    {
        if (myPlaylists[0]->players[i]->fileLoaded == true)
        {
            auto* xmlPlayer = new juce::XmlElement("PLAYER");
            createPlayerXmlElement(i, 0, xmlPlayer);
            if (xmlPlayer != nullptr)
                playlist->addChildElement(xmlPlayer);
        }
    }

    juce::XmlElement* cart = new juce::XmlElement("CART");
    for (int i = 0; i < myPlaylists[1]->players.size(); ++i)
    {
        if (myPlaylists[1]->players[i]->fileLoaded == true)
        {
            auto* xmlPlayer = new juce::XmlElement("PLAYER");
            createPlayerXmlElement(i, 1, xmlPlayer);
            if (xmlPlayer != nullptr)
                cart->addChildElement(xmlPlayer);
        }
    }


    juce::XmlElement multiPlayer("MULTIPLAYER");
    multiPlayer.addChildElement(playlist);
    multiPlayer.addChildElement(cart);
    auto xmlString = multiPlayer.toString();

    juce::FileChooser chooser("Choose an XML file to save", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*.xml");
    if (chooser.browseForFileToSave(true))
    {
        juce::File myPlaylistSave;
        myPlaylistSave = chooser.getResult();
        multiPlayer.writeTo(myPlaylistSave);
    }
}

juce::XmlElement* SoundPlayer::createPlayerXmlElement(int playerID, int playlistID, juce::XmlElement* e)
{
    if (myPlaylists[playlistID]->players[playerID] != nullptr)
    {
        auto* soundPlayer = myPlaylists[playlistID]->players[playerID];

        e->setAttribute("ID", playerID);
        e->setAttribute("path", soundPlayer->getFilePath());
        e->setAttribute("trimvolume", soundPlayer->getTrimVolume());
        e->setAttribute("islooping", soundPlayer->getIsLooping());
        e->setAttribute("Name", juce::String(soundPlayer->getName()));
        e->setAttribute("isHpfEnabled", soundPlayer->isHpfEnabled());
        bool isStartTimeSet = soundPlayer->startTimeSet;
        e->setAttribute("isStartTimeSet", isStartTimeSet);
        if (isStartTimeSet)
            e->setAttribute("startTime", soundPlayer->getStart());
        bool isStopTimeSet = soundPlayer->stopTimeSet;
        e->setAttribute("isStopTimeSet", isStopTimeSet);
        if (isStopTimeSet)
            e->setAttribute("stopTime", soundPlayer->getStop());

        //FILTER
        juce::StringArray params = soundPlayer->getFilterProcessor().getFilterParametersAsArray();
        for (int i = 0; i < params.size(); i++)
        {
            juce::String identifier = "filterParams" + juce::String(i);
            if (!params[i].isEmpty())
                e->setAttribute(identifier, params[i]);
        }
        e->setAttribute("isBypassed", soundPlayer->getBypassed());

        //ENVELOPPE
        e->setAttribute("enveloppeEnabled", soundPlayer->isEnveloppeEnabled());

        juce::Array<float> enveloppeXArray = soundPlayer->getEnveloppeXArray();
        for (int i = 0; i < enveloppeXArray.size(); i++)
        {
            juce::String identifier = "xArray" + juce::String(i);
            e->setAttribute(identifier, enveloppeXArray[i]);
        }
        juce::Array<float> enveloppeYArray = soundPlayer->getEnveloppeYArray();
        for (int i = 0; i < enveloppeYArray.size(); i++)
        {
            juce::String identifier = "yArray" + juce::String(i);
            e->setAttribute(identifier, enveloppeYArray[i]);
        }

        int enveloppePointsNumber = enveloppeXArray.size();
        e->setAttribute("envPointsNumber", enveloppePointsNumber);
        return e;

    }
    else
        return nullptr;
}



void SoundPlayer::loadPlaylist()
{
    if ((myPlaylists[0]->fader1IsPlaying == true)
        || myPlaylists[0]->fader2IsPlaying == true
        || myPlaylists[1]->fader1IsPlaying == true
        || myPlaylists[1]->fader2IsPlaying == true
        || myPlaylists[0]->spaceBarIsPlaying == true
        )
    {
        std::unique_ptr<juce::AlertWindow> dialogFilePlaying;
        dialogFilePlaying->showMessageBox(juce::AlertWindow::AlertIconType::WarningIcon, "Warning ! ", "Stop playback before loading a new playlist, and make sure all faders are down.");
    }
    else
    {
        juce::FileChooser chooser("Choose an XML File to load", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*.xml");

        if (chooser.browseForFileToOpen())
        {
            myFile = chooser.getResult();


            if (auto xml = parseXML(myFile))
            {
                myPlaylists[0]->fader1Player = -1;
                myPlaylists[0]->fader2Player = -1;
                myPlaylists[1]->fader1Player = -1;
                myPlaylists[1]->fader2Player = -1;
                myPlaylists[0]->spaceBarPlayerId = -1;
                if (soundPlayerMode != Mode::KeyMap)
                {
                    while (myPlaylists[0]->players.size() > 1)
                    {
                        myPlaylists[0]->removePlayer(0);
                        if (myPlaylists[0]->players[0] != nullptr)
                        {
                            myPlaylists[0]->players[0]->deleteFile();
                        }
                    }
                    while (myPlaylists[1]->players.size() > 1)
                        myPlaylists[1]->removePlayer(0);
                    if (myPlaylists[1]->players[0] != nullptr)
                    {
                        myPlaylists[1]->players[0]->deleteFile();
                    }
                }

                //for (auto i = 0; i < (myPlaylists[1]->players.size() + 1); i++)
                    //myPlaylists[1]->removePlayer(i);

                double numberOfPlayersToLoad = 0;
                if (xml->hasTagName("MULTIPLAYER"))
                {
                    forEachXmlChildElement(*xml, n)
                    {
                        if (n->hasTagName("PLAYLIST"))
                        {
                            auto iLoadedPlayer = 0;
                            forEachXmlChildElement(*n, e)
                            {
                                if (e->hasTagName("PLAYER"))
                                {
                                    numberOfPlayersToLoad++;
                                }
                            }
                        }
                        else if (n->hasTagName("CART"))
                        {
                            auto iLoadedPlayer = 0;
                            forEachXmlChildElement(*n, e)
                            {
                                if (e->hasTagName("PLAYER"))
                                {
                                    numberOfPlayersToLoad++;
                                }
                            }
                        }
                    }
                }


                double loaddedPlayers = 0;
                std::unique_ptr<juce::Label> progressLabel = std::make_unique<juce::Label>();
                addAndMakeVisible(*progressLabel);
                progressLabel->setBounds(0, 600, 200, 50);
                progressLabel->toFront(true);


                if (xml->hasTagName("MULTIPLAYER"))
                {
                    forEachXmlChildElement(*xml, n)
                    {
                        if (n->hasTagName("PLAYLIST"))
                        {
                            auto iLoadedPlayer = 0;
                            forEachXmlChildElement(*n, e)
                            {
                                if (e->hasTagName("PLAYER"))
                                {
                                    loadXMLElement(e, iLoadedPlayer, 0);
                                    iLoadedPlayer++;
                                    loaddedPlayers++;
                                    loadingProgress = loaddedPlayers / numberOfPlayersToLoad;

                                    progressLabel->setText(juce::String(loadingProgress), juce::NotificationType::dontSendNotification);
                                }
                            }
                        }
                        else if (n->hasTagName("CART"))
                        {
                            auto iLoadedPlayer = 0;
                            forEachXmlChildElement(*n, e)
                            {
                                if (e->hasTagName("PLAYER"))
                                {
                                    loadXMLElement(e, iLoadedPlayer, 1);

                                    iLoadedPlayer++;
                                    myPlaylists[1]->assignLeftFader(0);
                                    myPlaylists[1]->assignRightFader(1);
                                    loaddedPlayers++;
                                    loadingProgress = loaddedPlayers / numberOfPlayersToLoad;
                                }
                            }
                        }
                    }
                }
            }
            myPlaylists[0]->playersResetPositionClicked();
            myPlaylists[0]->fader1Player = 0;
            myPlaylists[0]->fader2Player = 0;
            myPlaylists[0]->spaceBarPlayerId = 0;
            myPlaylists[1]->assignLeftFader(2);
            myPlaylists[1]->assignRightFader(3);
            myPlaylists[1]->assignLeftFader(0);
            myPlaylists[1]->assignRightFader(1);
            myPlaylists[0]->resized();
            myPlaylists[0]->resized();

            playlistLoadedBroadcaster->sendChangeMessage();
            /*while (myPlaylists[0]->players.size() < 4)
                myPlaylists[0]->addPlayer(myPlaylists[0]->players.size() - 1);
            while (myPlaylists[1]->players.size() < 4)
                myPlaylists[1]->addPlayer(myPlaylists[1]->players.size() - 1);*/
            Settings::draggedPlayer = -1;
        }
    }
}

void SoundPlayer::loadXMLElement(juce::XmlElement* e, int player, int playlistID)
{
    int playerID = e->getIntAttribute("ID");
    juce::String filePath = juce::String(e->getStringAttribute("path"));
    int trimvolume = e->getDoubleAttribute("trimvolume");
    bool islooping = e->getBoolAttribute("islooping");
    std::string name = (e->getStringAttribute("Name")).toStdString();
    bool hpfEnabled = e->getBoolAttribute("isHpfEnabled");
    bool isStartTimeSet = e->getBoolAttribute("isStartTimeSet");
    float startTime = e->getDoubleAttribute("startTime");
    bool isStopTimeSet = e->getBoolAttribute("isStopTimeSet");
    float stopTime = e->getDoubleAttribute("stopTime");

    if (soundPlayerMode == Mode::KeyMap)
    {
        if (myPlaylists[playlistID]->players[playerID] == nullptr)
        {
            myPlaylists[playlistID]->addPlayer(playerID - 1);
        }
    }
    else

    {
        if (myPlaylists[playlistID]->players[player] == nullptr)
        {
            myPlaylists[playlistID]->addPlayer(player - 1);
        }
        playerID = player;
    }

    auto* playerToLoad = myPlaylists[playlistID]->players[playerID];

    playerToLoad->verifyAudioFileFormat(filePath);
    playerToLoad->setTrimVolume(trimvolume);
    playerToLoad->setIsLooping(islooping);
    playerToLoad->setName(name);
    playerToLoad->enableHPF(hpfEnabled);
    if (isStartTimeSet)
        playerToLoad->setStartTime(startTime);
    if (isStopTimeSet)
        playerToLoad->setStopTime(stopTime);

    //EQ
    juce::StringArray filterParams;
    for (int i = 0; i < 16; i++)
    {
        filterParams.add(e->getStringAttribute("filterParams" + juce::String(i)));
    }
    playerToLoad->getFilterProcessor().setFilterParametersAsArray(filterParams);
    playerToLoad->bypassFX(e->getBoolAttribute("isBypassed"));

    //Enveloppe
    playerToLoad->setEnveloppeEnabled(e->getBoolAttribute("enveloppeEnabled"));
    int enveloppePointsNumber = e->getStringAttribute("envPointsNumber").getIntValue();
    juce::Array<float> xArray;
    juce::Array<float> yArray;
    for (auto i = 0; i < enveloppePointsNumber; i++)
    {
        xArray.add(e->getStringAttribute("xArray" + juce::String(i)).getFloatValue());
        yArray.add(e->getStringAttribute("yArray" + juce::String(i)).getFloatValue());
    }
    playerToLoad->setEnveloppePath(Player::createEnveloppePathFromArrays(xArray, yArray));
}

void SoundPlayer::setTimerTime(int timertime)
{
    juce::Timer::startTimer(timertime);
    myPlaylists[0]->setTimerTime(timertime);
    myPlaylists[1]->setTimerTime(timertime);
}

void SoundPlayer::setEightPlayersMode(bool isEightPlayers)
{
    isEightPlayerMode = isEightPlayers;
    if (isEightPlayers)
        soundPlayerMode = Mode::EightFaders;
}

void SoundPlayer::updateDraggedPlayerDisplay(int playerDragged, int playlistDragged)
{
    //reset dragged player display on all players
    if (playerDragged != -1)
    {
        for (int i = 0; i < myPlaylists[0]->players.size(); i++)
        {
            myPlaylists[0]->players[i]->setActivePlayer(false);
        }
        for (int i = 0; i < myPlaylists[1]->players.size(); i++)
        {
            myPlaylists[1]->players[i]->setActivePlayer(false);
        }
        //set dragged player display on dragged player
        if (myPlaylists[playlistDragged]->players[Settings::draggedPlayer] != nullptr)
        {
            myPlaylists[playlistDragged]->players[Settings::draggedPlayer]->setActivePlayer(true);
        }
    }
}

void SoundPlayer::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == myPlaylists[0]->cuePlaylistBroadcaster)
    {
        draggedPlaylist = -1;
        draggedPlaylist = 0;
        draggedPlayer = Settings::draggedPlayer;
        draggedPlayer = myPlaylists[0]->draggedPlayer.toString().getIntValue();
    }
    else if (source == myPlaylists[1]->cuePlaylistBroadcaster)
    {
        draggedPlaylist = -1;
        draggedPlaylist = 1;
        draggedPlayer = Settings::draggedPlayer;
    }
}

void SoundPlayer::actionListenerCallback(const juce::String& message)
{

}

void SoundPlayer::initializeKeyMapPlayer()
{
    for (int i = 0; myPlaylists[0]->players.size() < 32; i++)
    {
        //myPlaylists[0]->players.clear();
        myPlaylists[0]->addPlayer(i);
        keyMappedSoundboard->addPlayer(myPlaylists[0]->players[i]);
    }
    keyMappedSoundboard->setShortcutKeys();
    keyMappedSoundboard->resized();
}

SoundPlayer::Mode SoundPlayer::getSoundPlayerMode()
{
    return soundPlayerMode;
}

bool SoundPlayer::isPlaying()
{
    for (auto* playlist : myPlaylists)
    {
        if (playlist->isPlaying())
            return true;
    }
    return false;
}
