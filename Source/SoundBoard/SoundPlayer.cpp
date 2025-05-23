/*
  ==============================================================================

    SoundPlayer.cpp
    Created: 15 Mar 2021 2:18:44pm
    Author:  Lucien

  ==============================================================================
*/

#include <JuceHeader.h>
#include "SoundPlayer.h"

using namespace std;

//==============================================================================
SoundPlayer::SoundPlayer(SoundPlayer::Mode m, Settings* s)
{
    settings = s;
    soundPlayerMode = m;
    if (m == SoundPlayer::Mode::EightFaders)
        isEightPlayerMode = true;;
    setMouseClickGrabsKeyboardFocus(false);

    juce::Timer::startTimer(50);

    if (soundPlayerMode == SoundPlayer::Mode::OnePlaylistOneCart)
        myPlaylists.add(new Playlist(0, settings));
    else if (soundPlayerMode == SoundPlayer::Mode::EightFaders)
        myPlaylists.add(new Playlist(1, settings));
    else
    {
        myPlaylists.add(new Playlist(0, settings));
        myPlaylists.add(new Playlist(1, settings));
        keyMappedSoundboard.reset(new KeyboardMappedSoundboard(settings));
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

    if (soundPlayerMode != SoundPlayer::Mode::KeyMap)
    {
        playlistViewport.setBounds(0, playersStartHeightPosition, 715, (getParentHeight() - playersStartHeightPosition));
        playlistViewport.setViewedComponent(myPlaylists[0], false);
        playlistViewport.setWantsKeyboardFocus(false);
        addAndMakeVisible(playlistViewport);
        playlistViewport.setMouseClickGrabsKeyboardFocus(false);
        playlistViewport.getVerticalScrollBar().setMouseClickGrabsKeyboardFocus(false);
        if (!isEightPlayerMode)
        {
            addAndMakeVisible(addPlayerPlaylist);
            addPlayerPlaylist.setButtonText("Add Player");
            addPlayerPlaylist.onClick = [this] { myPlaylists[0]->addPlayer(myPlaylists[0]->players.size() - 1); };
            addAndMakeVisible(removePlayerPlaylist);
            removePlayerPlaylist.setButtonText("Remove Player");
            removePlayerPlaylist.onClick = [this] { myPlaylists[0]->removeButtonClicked();    };
        }

        myPlaylists.add(new Playlist(1, settings));

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
        playlistbisViewport.setBounds(getParentWidth() / 2, playersStartHeightPosition,
            getParentWidth() / 2, (getParentHeight() - playersStartHeightPosition));
        playlistbisViewport.setViewedComponent(myPlaylists[1], false);
        playlistbisViewport.setWantsKeyboardFocus(false);
        playlistbisViewport.getVerticalScrollBar().setMouseClickGrabsKeyboardFocus(false);

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
        }
    }

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

    meter.removeMouseListener(this);
    meter.setMouseClickGrabsKeyboardFocus(false);
    loudnessBarComponent.setMouseClickGrabsKeyboardFocus(false);
    addAndMakeVisible(&mainMeter);
    mainMeter.setMouseClickGrabsKeyboardFocus(false);

    addAndMakeVisible(&timeLabel);
    timeLabel.setFont(juce::Font(30.00f, juce::Font::plain).withTypefaceStyle("Regular"));
    timeLabel.setMouseClickGrabsKeyboardFocus(false);
    timeLabel.setJustificationType(juce::Justification::centred);

    addChildComponent(&mainStopWatch);
    mainStopWatch.setMouseClickGrabsKeyboardFocus(false);

    for (int i = 0; i < getNumChildComponents(); i++)
    {
        getChildComponent(i)->setMouseClickGrabsKeyboardFocus(false);
    }

    if (Settings::OSCEnabled)
        OSCInitialize();
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

void SoundPlayer::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill, const juce::AudioSourceChannelInfo& cueBuffer)
{
    for (auto playlist : myPlaylists)
    {
        playlist->getNextAudioBlock(bufferToFill, cueBuffer);
    }
}

void SoundPlayer::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    actualSampleRate = sampleRate;
    actualSamplesPerBlockExpected = samplesPerBlockExpected;

    myPlaylists[0]->prepareToPlay(samplesPerBlockExpected, sampleRate);
    myPlaylists[1]->prepareToPlay(samplesPerBlockExpected, sampleRate);

    if (keyMappedSoundboard != nullptr)
        keyMappedSoundboard->prepareToPlay(samplesPerBlockExpected, sampleRate);

    loudnessMeter.prepareToPlay(actualSampleRate, 2, actualSamplesPerBlockExpected, 20);
    meterSource.resize(2, sampleRate * 0.1 / samplesPerBlockExpected);
    cuemeterSource.resize(2, sampleRate * 0.1 / samplesPerBlockExpected);

    mainMeter.prepareToPlay(samplesPerBlockExpected, sampleRate);
    newMeter->prepareToPlay(samplesPerBlockExpected, sampleRate);  
}

void SoundPlayer::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void SoundPlayer::resized()
{
    if (Settings::showTimer)
    {
        stopWatchHeight = 25;
        mainStopWatch.setVisible(true);
    }
    else
    {
        stopWatchHeight = 0;
        mainStopWatch.setVisible(false);
    }

    if (Settings::showClock)
    {
        timeLabelHeight = 35;
        timeLabel.setVisible(true);
    }
    else
    {
        timeLabelHeight = 0;
        timeLabel.setVisible(false);
    }

    if (soundPlayerMode != SoundPlayer::Mode::KeyMap)
    {
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
            int availableHeight = getHeight() - timeLabelHeight - stopWatchHeight;
            levelMeterHeight = min(getHeight() - playersStartHeightPosition - cuelevelMeterMinimumHeight - timeLabelHeight - stopWatchHeight,
                levelMeterMaximumHeight);
            levelMeterHeight = availableHeight / 3 * 2;
            cueLevelMeterHeight = availableHeight / 3;
            meter.setBounds(cueMeterXStart, getHeight() - levelMeterHeight, 80,
                min(getHeight() - playersStartHeightPosition, levelMeterHeight));
            loudnessBarComponent.setBounds(meter.getBounds().getTopRight().getX() + 7,
                getHeight() - levelMeterHeight, 25, levelMeterHeight);
            
            int cueMeterYStart = meter.getPosition().getY() - cueLevelMeterHeight;
            cuemeter.setBounds(cueMeterXStart, cueMeterYStart, 80, cueLevelMeterHeight);

            timeLabel.setBounds(playlistViewport.getRight(), 0, timeLabelWidth, timeLabelHeight);
            mainStopWatch.setBounds(timeLabel.getX(), timeLabel.getBottom(), timeLabelWidth, stopWatchHeight);
        }
        else if (Settings::audioOutputMode == 2)
        {
            levelMeterHeight = min(getHeight() - playersStartHeightPosition - timeLabelHeight - stopWatchHeight, levelMeterMaximumHeight);

            meter.setBounds(playlistViewport.getWidth(), getHeight() - levelMeterHeight,
                80, min(getHeight() - playersStartHeightPosition, levelMeterHeight));        

            loudnessBarComponent.setBounds(meter.getBounds().getTopRight().getX() + 7,
                getHeight() - levelMeterHeight, 25, levelMeterHeight);

            timeLabel.setBounds(playlistViewport.getRight(), 0, timeLabelWidth, timeLabelHeight);
            mainStopWatch.setBounds(timeLabel.getX(), timeLabel.getBottom(), timeLabelWidth, stopWatchHeight);

        }
    }
    else
    {
        levelMeterHeight = getHeight() - timeLabelHeight - stopWatchHeight;
        meter.setBounds(getWidth() - meterWidth - loudnessBarWidth - 7, getHeight() - levelMeterHeight,
            meterWidth, min(getHeight() - playersStartHeightPosition, levelMeterHeight));
        loudnessBarComponent.setBounds(meter.getBounds().getTopRight().getX() + 5,
            getHeight() - levelMeterHeight, loudnessBarWidth, levelMeterHeight);
        keyMappedSoundboard->setBounds(0, 15, getWidth() - (getWidth() - meter.getX()), getHeight());
        timeLabel.setBounds(meter.getX(), 0, getWidth() - meter.getX(), timeLabelHeight);
        mainStopWatch.setBounds(meter.getX(), timeLabel.getBottom(), getWidth() - meter.getX(), stopWatchHeight);
    }
}


void SoundPlayer::initializeKeyMapPlayer()
{
    for (int i = 0; myPlaylists[0]->players.size() < 32; i++)
    {
        myPlaylists[0]->addPlayer(i);
        auto addedPlayer = keyMappedSoundboard->addPlayer(myPlaylists[0]->players[i]);
        addedPlayer->playerDraggedBroadcaster->addChangeListener(keyMappedSoundboard.get());
        addedPlayer->shortcutChangedBroadcaster->addChangeListener(keyMappedSoundboard.get());
        addedPlayer->addMouseListener(keyMappedSoundboard.get(), true);
    }
    keyMappedSoundboard->setShortcutKeys();
    keyMappedSoundboard->resized();

    int OSCindex = 1;
    for (auto player : myPlaylists[0]->players)
    {
        player->setPlayerColour(juce::Colour(40, 134, 189));
        player->setIsCart(true);
        player->setOSCIndex(OSCindex);
        OSCindex++;
    }
    auto secondPlaylist = myPlaylists[1];
    if (secondPlaylist != nullptr)
    {
        for (auto player : secondPlaylist->players)
            player->setOSCIndex(-1);
    }
    keyMappedSoundboard->prepareToPlay(actualSamplesPerBlockExpected, actualSampleRate);
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

Player* SoundPlayer::getActivePlayer()
{
    if (myPlaylists[Settings::editedPlaylist]->players[Settings::editedPlayer] != nullptr)
    {
        auto* r = myPlaylists[Settings::editedPlaylist]->players[Settings::editedPlayer];
        return r;
    }
    else
        return nullptr;
}

juce::ApplicationCommandTarget* SoundPlayer::getNextCommandTarget()
{
    return nullptr;
}


void SoundPlayer::getAllCommands(juce::Array<juce::CommandID>& commands)
{
    juce::Array<juce::CommandID> c{ CommandIDs::cuePlay,
                                    CommandIDs::cueStart,
                                    CommandIDs::cueEnd,
                                    CommandIDs::toggleClipEffects,
                                    CommandIDs::toggleClipEnveloppe,
                                    CommandIDs::toggleClipLooping,
                                    CommandIDs::toggleHPF,
                                    CommandIDs::upOneDb,
                                    CommandIDs::downOneDb,
                                    CommandIDs::dummy };
    commands.addArray(c);
}

void SoundPlayer::getCommandInfo(juce::CommandID commandID, juce::ApplicationCommandInfo& result)
{
    switch (commandID)
    {
    case CommandIDs::cuePlay:
        result.setInfo("Cue play at playhead", "Cue Play", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress('c', juce::ModifierKeys::noModifiers);
        break;
    case CommandIDs::cueStart:
        result.setInfo("Cue play from start", "Cue play from start", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress('x', juce::ModifierKeys::noModifiers);
        break;
    case CommandIDs::cueEnd:
        result.setInfo("Cue play last 5 seconds", "Cue play end", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress('v', juce::ModifierKeys::noModifiers);
        break;
    case CommandIDs::toggleClipEffects:
        result.setInfo("Enable / disable clip effects", "Enable / disable clip effects", "Clip", 0);
        result.setTicked(false);
        result.addDefaultKeypress('e', juce::ModifierKeys::commandModifier);
        break;
    case CommandIDs::toggleClipEnveloppe:
        result.setInfo("Enable / disable clip enveloppe", "Enable / disable clip enveloppe", "Clip", 0);
        result.setTicked(false);
        result.addDefaultKeypress('n', juce::ModifierKeys::commandModifier);
        break;
    case CommandIDs::toggleClipLooping:
        result.setInfo("Enable / disable clip looping", "Enable / disable clip looping", "Clip", 0);
        result.setTicked(false);
        result.addDefaultKeypress('l', juce::ModifierKeys::commandModifier);
        break;
    case CommandIDs::toggleHPF:
        result.setInfo("Enable / disable clip high pass filter", "Enable / disable clip high pass filter", "Clip", 0);
        result.setTicked(false);
        result.addDefaultKeypress('h', juce::ModifierKeys::commandModifier);
        break;
    case CommandIDs::upOneDb:
        result.setInfo("Increase clip trim volume by 1dB", "Increase clip trim volume by 1dB", "Clip", 0);
        result.setTicked(false);
        result.addDefaultKeypress(juce::KeyPress::numberPadAdd, juce::ModifierKeys::noModifiers);
        break;
    case CommandIDs::downOneDb:
        result.setInfo("Decrease clip trim volume by 1dB", "Decrease clip trim volume by 1dB", "Clip", 0);
        result.setTicked(false);
        result.addDefaultKeypress(juce::KeyPress::numberPadSubtract, juce::ModifierKeys::noModifiers);
        break;
    case CommandIDs::dummy:
        result.setInfo("dummy", "dummy", "Clip", 0);
        result.setTicked(false);
        break;
    default:
        break;
    }
}

bool SoundPlayer::perform(const InvocationInfo& info)
{
    switch (info.commandID)
    {
    case CommandIDs::toggleClipEffects:
        if (getActivePlayer() != nullptr)
            getActivePlayer()->bypassFX(getActivePlayer()->isFxEnabled(), true);
        break;
    case CommandIDs::toggleClipEnveloppe:
        if (getActivePlayer() != nullptr)
            getActivePlayer()->setEnveloppeEnabled(!getActivePlayer()->isEnveloppeEnabled(), true, true);
        break;
    case CommandIDs::toggleClipLooping:
        if (getActivePlayer() != nullptr)
            getActivePlayer()->setIsLooping(!getActivePlayer()->getIsLooping(), true);
        break;
    case CommandIDs::toggleHPF:
        if (getActivePlayer() != nullptr)
            getActivePlayer()->enableHPF(!getActivePlayer()->isHpfEnabled(), false);
        break;
    case CommandIDs::upOneDb:
        if (getActivePlayer() != nullptr)
            getActivePlayer()->handleMidiTrimMessage(true);
        break;
    case CommandIDs::downOneDb:
        if (getActivePlayer() != nullptr)
            getActivePlayer()->handleMidiTrimMessage(false);
        break;
    case CommandIDs::cuePlay:
        if (getActivePlayer() != nullptr)
            getActivePlayer()->cueButtonClicked();
        break;
    case CommandIDs::cueStart:
        if (getActivePlayer() != nullptr)
        {
            getActivePlayer()->cueTransport.setPosition(0);
            getActivePlayer()->cueButtonClicked();
        }
        break;
    case CommandIDs::cueEnd:
        if (getActivePlayer() != nullptr)
        {
            getActivePlayer()->cueTransport.setPosition(getActivePlayer()->stopTime - 6);
            getActivePlayer()->cueButtonClicked();
        }
        break;
    case CommandIDs::dummy:
        break;
    default:
        return false;
    }
    return true;
}

bool SoundPlayer::handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message, MidiMapper* mapper)
{
    midiMessageValue = message.getControllerValue();
    midiMessageNumber = message.getControllerNumber();

    if (soundPlayerMode == SoundPlayer::Mode::OnePlaylistOneCart)
    {
        if (myPlaylists[0]->handleIncomingMidiMessage(source, message, mapper))
            return true;
        else if (myPlaylists[1]->handleIncomingMidiMessage(source, message, mapper))
            return true;
        else
            return false;
    }
    else if (soundPlayerMode == SoundPlayer::Mode::EightFaders)
    {
        for (int i = 0; i < 4; i++)
        {
            if (midiMessageNumber == mapper->getMidiCCForCommand(200 + i))
            {
                myPlaylists[0]->handleIncomingMidiMessageEightPlayers(source, message, mapper, 200);
                return true;
            }
            else if (midiMessageNumber == mapper->getMidiCCForCommand(204 + i))
            {
                myPlaylists[1]->handleIncomingMidiMessageEightPlayers(source, message, mapper, 204);
                return true;
            }
            else if (midiMessageNumber == mapper->getMidiCCForCommand(208 + i))
            {
                myPlaylists[0]->handleMidiTrimEightPlayers(midiMessageValue, midiMessageNumber, mapper, 208);
                return true;
            }
            else if (midiMessageNumber == mapper->getMidiCCForCommand(212 + i))
            {
                myPlaylists[1]->handleMidiTrimEightPlayers(midiMessageValue, midiMessageNumber, mapper, 212);
                return true;
            }
        }
    }
    else if (soundPlayerMode == SoundPlayer::Mode::KeyMap)
    {
        for (int i = 0; i < 30; i++)
        {
            if (midiMessageNumber == mapper->getMidiCCForCommand(250 + i) && midiMessageValue == 127)
            {
                auto* player = keyMappedSoundboard->mappedPlayers[i];
                if (player != nullptr)
                {
                    player->shortcutKeyPressed();
                }
                return true;
            }
        }
    }
    return false;
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
        cuemeter.setSelectedChannel(0);
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
        levelMeterHeight = min(getHeight() - playersStartHeightPosition - cuelevelMeterMinimumHeight, levelMeterMaximumHeight);
        meter.setBounds(cueMeterXStart, getHeight() - levelMeterHeight, 80, min(getHeight() - playersStartHeightPosition, levelMeterHeight));
        cuelevelMeterHeight = min(getHeight() - playersStartHeightPosition - levelMeterHeight, cuelevelMeterMaximumHeight);
        int cueMeterYStart = meter.getPosition().getY() - cuelevelMeterHeight;
        cuemeter.setBounds(cueMeterXStart, cueMeterYStart, 80, cuelevelMeterHeight);
        if (soundPlayerMode == SoundPlayer::Mode::KeyMap)
            cuemeter.setVisible(false);
        else
            cuemeter.setVisible(true);
    }
    else if (Settings::audioOutputMode == 2)
    {
        levelMeterHeight = min(getHeight() - playersStartHeightPosition, levelMeterMaximumHeight);
        meter.setBounds(playlistViewport.getWidth(), getHeight() - levelMeterHeight, 80, min(getHeight() - playersStartHeightPosition, levelMeterHeight));
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

    juce::FileChooser chooser("Choose an XML file to save", juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("LuPlayer"), "*.xml");
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
        e->setAttribute("hasBeenNormalized", soundPlayer->getHasBeenNormalized());
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
        e->setAttribute("playMode", soundPlayer->getPlayMode());

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

        //Colour
        e->setAttribute("playerColour", soundPlayer->getPlayerColour().toString());
        e->setAttribute("playerColourChanged", soundPlayer->getColourHasChanged());

        e->setAttribute("shortcut", soundPlayer->getShortcut().getTextDescription());
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
        juce::FileChooser chooser("Choose an XML File to load", juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("LuPlayer"), "*.xml");

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

            if (soundPlayerMode == SoundPlayer::Mode::EightFaders)
            {
                while (myPlaylists[0]->players.size() < 4)
                    myPlaylists[0]->addPlayer(myPlaylists[0]->players.size() - 1);
                while (myPlaylists[1]->players.size() < 4)
                    myPlaylists[1]->addPlayer(myPlaylists[1]->players.size() - 1);
            }

            playlistLoadedBroadcaster->sendChangeMessage();

            Settings::draggedPlayer = -1;
        }
    }
}

void SoundPlayer::loadXMLElement(juce::XmlElement* e, int player, int playlistID)
{
    int playerID = e->getIntAttribute("ID");
    bool hasBeenNormalized = e->getBoolAttribute("hasBeenNormalized");
    juce::String filePath = juce::String(e->getStringAttribute("path"));
    double trimvolume = e->getDoubleAttribute("trimvolume");
    bool islooping = e->getBoolAttribute("islooping");
    std::string name = (e->getStringAttribute("Name")).toStdString();
    bool hpfEnabled = e->getBoolAttribute("isHpfEnabled");
    bool isStartTimeSet = e->getBoolAttribute("isStartTimeSet");
    float startTime = e->getDoubleAttribute("startTime");
    bool isStopTimeSet = e->getBoolAttribute("isStopTimeSet");
    float stopTime = e->getDoubleAttribute("stopTime");
    int playMode = e->getIntAttribute("playMode");

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

    playerToLoad->setHasBeenNormalized(hasBeenNormalized);
    playerToLoad->verifyAudioFileFormat(filePath);
    playerToLoad->setIsLooping(islooping);
    playerToLoad->setName(name);
    playerToLoad->enableHPF(hpfEnabled);
    if (isStartTimeSet)
        playerToLoad->setStartTime(startTime);
    if (isStopTimeSet)
        playerToLoad->setStopTime(stopTime);
    playerToLoad->setPlayMode(playMode);

    //EQ
    juce::StringArray filterParams;
    for (int i = 0; i < 16; i++)
    {
        filterParams.add(e->getStringAttribute("filterParams" + juce::String(i)));
    }
    playerToLoad->getFilterProcessor().setFilterParametersAsArray(filterParams);
    playerToLoad->bypassFX(e->getBoolAttribute("isBypassed"), false);

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

    if (e->getBoolAttribute("playerColourChanged"))
        playerToLoad->setPlayerColour(juce::Colour::fromString(e->getStringAttribute("playerColour")), false);

    playerToLoad->setSortcut(juce::KeyPress::createFromDescription(e->getStringAttribute("shortcut")));
    playerToLoad->setTrimVolume(trimvolume);
}


void SoundPlayer::positionViewport(int player)
{
    playlistbisViewport.setViewPosition(0, (player - 1) * 105);
}

void SoundPlayer::loadInFirstEmptyPlayer(juce::String file, juce::String name)
{
    if (soundPlayerMode == SoundPlayer::Mode::KeyMap)
    {
        if (keyMappedSoundboard != nullptr)
        {
            for (auto player : keyMappedSoundboard->mappedPlayers)
            {
                auto playerSource = player->getPlayer();
                if (!playerSource->isFileLoaded() && player->isVisible())
                {
                    playerSource->verifyAudioFileFormat(file);
                    if (name.isNotEmpty())
                        playerSource->setName(name.toStdString());
                    return;
                }
            }
        }
    }
    else
    {
        for (auto playlist : myPlaylists)
        {
            for (auto player : playlist->players)
            {
                if (!player->isLoadingFile)
                {
                    player->verifyAudioFileFormat(file);
                    if (name.isNotEmpty())
                        player->setName(name.toStdString());
                    return;
                }
            }
            if (soundPlayerMode == SoundPlayer::Mode::OnePlaylistOneCart)
                break;
        }
    }
    if (soundPlayerMode == SoundPlayer::Mode::OnePlaylistOneCart)
    {
        auto addedPlayer = myPlaylists[0]->addPlayer(myPlaylists[0]->players.size() - 1);
        addedPlayer->verifyAudioFileFormat(file);
        if (name.isNotEmpty())
            addedPlayer->setName(name.toStdString());
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
        draggedEnveloppeXArray = player->getEnveloppeXArray();
        draggedEnveloppeYArray = player->getEnveloppeYArray();
        draggedEnveloppeEnabled = player->isEnveloppeEnabled();
        draggedColour = player->getPlayerColour();
        draggedColourChanged = player->getColourHasChanged();
        mouseDragged = true;
    }
}

void SoundPlayer::drawDragLines()
{
    if (soundPlayerMode == SoundPlayer::Mode::OnePlaylistOneCart)
    {
        auto* sourcePlayer = myPlaylists[playlistDragSource]->players[playerDragSource];

        if (sourcePlayer != nullptr)
        {
            draggedSourcePlayer = sourcePlayer;
            if (sourcePlayer->isFileLoaded())
            {
                draggedDestinationPlayer = nullptr;
                for (auto player : myPlaylists[0]->players)
                    player->isDraggedOver(false);
                for (auto player : myPlaylists[1]->players)
                    player->isDraggedOver(false);
                for (int p = 0; p < myPlaylists.size(); p++)
                {
                    myPlaylists[p]->fileDragPlayerDestination = -1;
                    myPlaylists[p]->fileDragPaintLine = false;
                    insertTop = false;
                    myPlaylists[p]->repaint();

                    for (int i = 0; i < myPlaylists[p]->players.size(); i++)
                    {
                        auto* player = myPlaylists[p]->players[i];
                        juce::Point<int> mouseGlobal = localPointToGlobal(getMouseXYRelative());

                        if (myPlaylists[p]->players[0]->getScreenBounds().removeFromTop(playerInsertDragZoneHeight).contains(mouseGlobal))
                        {
                            insertTop = true;
                            playerMouseDragUp = -1;
                            myPlaylists[p]->fileDragPlayerDestination = playerMouseDragUp;
                            playlistDragDestination = p;
                            myPlaylists[p]->fileDragPaintLine = true;
                            destinationPlayerFound = true;
                            myPlaylists[p]->repaint();
                        }
                        else if (myPlaylists[p]->players[i]->getScreenBounds().removeFromBottom(playerInsertDragZoneHeight).contains(mouseGlobal))
                        {
                            myPlaylists[p]->fileDragPlayerDestination = i;
                            myPlaylists[p]->fileDragPaintLine = true;
                            playlistDragDestination = p;
                            playerMouseDragUp = i;
                            myPlaylists[p]->repaint();
                        }
                        else if (player->getScreenBounds().contains(mouseGlobal) && !player->isFileLoaded())
                        {
                            playlistDragDestination = p;
                            playerMouseDragUp = i;
                            draggedDestinationPlayer = player;
                            draggedDestinationPlayer->isDraggedOver(true);
                            return;
                        }
                    }
                }
                if (draggedDestinationPlayer != nullptr)
                {
                    draggedDestinationPlayer->isDraggedOver(true);
                }
            }
        }
    }
    else if (soundPlayerMode == Mode::EightFaders)
    {
        auto* sourcePlayer = myPlaylists[playlistDragSource]->players[playerDragSource];

        if (sourcePlayer != nullptr)
        {
            draggedSourcePlayer = sourcePlayer;
            if (sourcePlayer->isFileLoaded())
            {
                draggedDestinationPlayer = nullptr;
                for (int p = 0; p < myPlaylists.size(); p++)
                {
                    for (int i = 0; i < myPlaylists[p]->players.size(); i++)
                    {
                        auto* player = myPlaylists[p]->players[i];
                        juce::Point<int> mouseGlobal = localPointToGlobal(getMouseXYRelative());

                        if (player->getScreenBounds().contains(mouseGlobal) && !player->isFileLoaded())
                            draggedDestinationPlayer = player;
                    }
                }
                for (auto player : myPlaylists[0]->players)
                    player->isDraggedOver(false);
                for (auto player : myPlaylists[1]->players)
                    player->isDraggedOver(false);
                if (draggedDestinationPlayer != nullptr)
                {
                    draggedDestinationPlayer->isDraggedOver(true);
                }
            }
        }
    }
}

void SoundPlayer::mouseDragDefinePlayer()
{
    if (soundPlayerMode == Mode::OnePlaylistOneCart)
    {
        if (draggedSourcePlayer != nullptr && draggedDestinationPlayer != nullptr)
        {
            draggedDestinationPlayer->setPlayerInfo(draggedSourcePlayer->getPlayerInfo());
            draggedSourcePlayer->deleteFile();

        }
        else if (playlistDragDestination != -1)
            mouseDragSetInfos(playlistDragDestination, playerMouseDragUp);

    }
    else if (soundPlayerMode == Mode::EightFaders)
    {
        if (draggedSourcePlayer != nullptr && draggedDestinationPlayer != nullptr)
        {
            draggedDestinationPlayer->setPlayerInfo(draggedSourcePlayer->getPlayerInfo());
            draggedSourcePlayer->deleteFile();
        }
    }
    mouseDragEnd(0);
    mouseDragEnd(1);
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
    player->setEnveloppePath(Player::createEnveloppePathFromArrays(draggedEnveloppeXArray, draggedEnveloppeYArray));
    player->setEnveloppeEnabled(draggedEnveloppeEnabled, false, false);
    if (draggedColourChanged)
        player->setPlayerColour(draggedColour);
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
    draggedColourChanged = false;
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
        myPlaylists[playlistDragSource]->grabFocusBroadcaster->sendChangeMessage();
    }
    clearDragInfos();
    draggedSourcePlayer = nullptr;
    draggedDestinationPlayer = nullptr;
    for (auto player : myPlaylists[0]->players)
        player->isDraggedOver(false);
    for (auto player : myPlaylists[1]->players)
        player->isDraggedOver(false);
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
    else if (source == settings->keyboardLayoutBroadcaster.get())
    {
        if (soundPlayerMode == SoundPlayer::Mode::KeyMap)
        {
            if (keyMappedSoundboard != nullptr)
                keyMappedSoundboard->setShortcutKeys();
        }
    }
}


void SoundPlayer::playPlayer(int playerID)
{
    if (soundPlayerMode == Mode::OnePlaylistOneCart)
        myPlaylists[1]->playPlayer(playerID);
    else if (soundPlayerMode == Mode::EightFaders)
    {
        if (playerID > 3)
            myPlaylists[1]->playPlayer(playerID - 4);
        else
            myPlaylists[0]->playPlayer(playerID);
    }
    positionViewport(playerID);

}

void SoundPlayer::valueChanged(juce::Value& value)
{
    if (value.refersToSameSourceAs(myPlaylists[0]->minimumPlayer))
    {
        playlistViewport.setViewPosition(0, (((myPlaylists[0]->minimumPlayer).toString().getIntValue()) - 1) * 105);
    }
    else if (value.refersToSameSourceAs(Settings::audioOutputModeValue))
    {
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
            OSCsend(3, 0.);

        }
        else if (value.toString().getIntValue() == myPlaylists[1]->fader2Player)
        {
            OSCsend(4, 0.);
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

void SoundPlayer::timerCallback()
{
    if (Settings::audioOutputMode == 2 || Settings::audioOutputMode == 3)
        shortTermLoudness = loudnessMeter.getMomentaryLoudness();
    else if (Settings::audioOutputMode == 1)
        shortTermLoudness = loudnessMeter.getMomentaryLoudnessForIndividualChannels()[0];
    if (shortTermLoudness > 1)
        shortTermLoudness = 1;
    loudnessBarComponent.setLoudness(shortTermLoudness);

    juce::Time* time = new juce::Time(time->getCurrentTime());
    timeLabel.setText(time->toString(false, true, true, true), juce::NotificationType::dontSendNotification);

    //OSC SEND
    if (oscConnected && Settings::OSCEnabled)
    {
        if (soundPlayerMode == Mode::OnePlaylistOneCart)
        {
            if (myPlaylists[0]->players[myPlaylists[0]->fader1Player] != nullptr)
            {
                OSCsend(1, myPlaylists[0]->players[myPlaylists[0]->fader1Player]->remainingTimeString);
                sender->send("/1/gain1", juce::String(myPlaylists[0]->players[myPlaylists[0]->fader1Player]->volumeLabel.getText() + "dB"));
                delta1 = juce::Time::currentTimeMillis() - start1;
                if (delta1 > deltaTime)
                {

                    juce::NormalisableRange<float>valueToSendNorm(0., juce::Decibels::decibelsToGain(Settings::maxFaderValueGlobal), 0.0001, Settings::skewFactorGlobal);
                    sender->send("/1/fader1", valueToSendNorm.convertTo0to1(myPlaylists[0]->players[myPlaylists[0]->fader1Player]->volumeSlider.getValue()));

                }
            }
            else
            {
                OSCsend(1, "0:0");
            }

            if (myPlaylists[0]->players[myPlaylists[0]->fader2Player] != nullptr)
            {

                OSCsend(2, myPlaylists[0]->players[myPlaylists[0]->fader2Player]->remainingTimeString);
                sender->send("/1/gain2", juce::String(myPlaylists[0]->players[myPlaylists[0]->fader2Player]->volumeLabel.getText() + "dB"));
                delta2 = juce::Time::currentTimeMillis() - start2;
                if (delta2 > deltaTime)
                {
                    juce::NormalisableRange<float>valueToSendNorm(0., juce::Decibels::decibelsToGain(Settings::maxFaderValueGlobal), 0.0001, Settings::skewFactorGlobal);
                    sender->send("/1/fader2", valueToSendNorm.convertTo0to1(myPlaylists[0]->players[myPlaylists[0]->fader2Player]->volumeSlider.getValue()));
                }
            }
            else
            {
                OSCsend(2, "0:0");
            }

            if (myPlaylists[1]->players[myPlaylists[1]->fader1Player] != nullptr)
            {
                OSCsend(3, myPlaylists[1]->players[myPlaylists[1]->fader1Player]->remainingTimeString);
                sender->send("/1/gain3", juce::String(myPlaylists[1]->players[myPlaylists[1]->fader1Player]->volumeLabel.getText() + "dB"));
                delta3 = juce::Time::currentTimeMillis() - start3;
                if (delta3 > deltaTime)
                {
                    juce::NormalisableRange<float>valueToSendNorm(0., juce::Decibels::decibelsToGain(Settings::maxFaderValueGlobal), 0.0001, Settings::skewFactorGlobal);
                    sender->send("/1/fader3", valueToSendNorm.convertTo0to1(myPlaylists[1]->players[myPlaylists[1]->fader1Player]->volumeSlider.getValue()));
                }
            }
            else
            {
                OSCsend(3, "0:0");
            }

            if (myPlaylists[1]->players[myPlaylists[1]->fader2Player] != nullptr)
            {
                OSCsend(4, myPlaylists[1]->players[myPlaylists[1]->fader2Player]->remainingTimeString);
                sender->send("/1/gain4", juce::String(myPlaylists[1]->players[myPlaylists[1]->fader2Player]->volumeLabel.getText() + "dB"));
                delta4 = juce::Time::currentTimeMillis() - start4;
                if (delta4 > deltaTime)
                {
                    juce::NormalisableRange<float>valueToSendNorm(0., juce::Decibels::decibelsToGain(Settings::maxFaderValueGlobal), 0.0001, Settings::skewFactorGlobal);
                    sender->send("/1/fader4", valueToSendNorm.convertTo0to1(myPlaylists[1]->players[myPlaylists[1]->fader2Player]->volumeSlider.getValue()));
                }
            }
            else
            {
                OSCsend(4, "0:0");
            }
        }
        juce::NormalisableRange<float>valueToSendNorm(0., 1., 0.001, 0.2);
        OSCsend(5, valueToSendNorm.convertTo0to1(meterSource.getRMSLevel(1)));
    }
}

void SoundPlayer::OSCInitialize()
{
    if (Settings::OSCEnabled)
    {
        if (receiver.connect(juce::int32(Settings::inOscPort)) && settings->sender.connect(Settings::ipAdress, juce::int32(Settings::outOscPort)))
        {
            sender = &settings->sender;
            oscConnected = true;
            OSCClean();
        }

        receiver.addListener(this);
        //addListener(this, "/1/fader1");
        //addListener(this, "/1/fader2");
        //addListener(this, "/1/fader3");
        //addListener(this, "/1/fader4");
        //addListener(this, "/1/push1");
        //addListener(this, "/1/push2");
        //addListener(this, "/1/push3");
        //addListener(this, "/1/push4");
        //addListener(this, "/1/up");
        //addListener(this, "/1/reset");
        //addListener(this, "/1/down");
        //addListener(this, "/2/multipush1");
    }
    else
    {
        OSCClean();
        if (receiver.disconnect())   // [13]
        {
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
    if (soundPlayerMode == Mode::EightFaders)
    {
        for (int i = 1; i < 9; i++)
        {
            juce::String adress = "/8faderstime" + juce::String(i);
            settings->sender.send(adress, juce::String());
            adress = "/8fadersgain" + juce::String(i);
            settings->sender.send(adress, (float)0.0);
            adress = "/8faderslabel" + juce::String(i);
            settings->sender.send(adress, juce::String());
        }
    }
}
void SoundPlayer::oscMessageReceived(const juce::OSCMessage& message)
{
    if (soundPlayerMode == Mode::OnePlaylistOneCart)
        handleOSCPlaylist(message);
    else if (soundPlayerMode == Mode::EightFaders)
        handleOSCEightFaders(message);
    else if (soundPlayerMode == Mode::KeyMap)
        handleOSCKeyMap(message);
}

void SoundPlayer::handleOSCKeyMap(const juce::OSCMessage& message)
{
    for (int i = 0; i < 30; i++)
    {
        juce::String adress = "/kmpush" + juce::String(i + 1);
        if (message.getAddressPattern().matches(adress))
        {
            if (message.size() == 1 && message[0].isFloat32())
            {
                if (message[0].getFloat32() == 1)
                {
                    auto* player = myPlaylists[0]->players[i];
                    if (player != nullptr)
                    {
                        if (!player->isPlayerPlaying())
                            player->launch();
                        else
                            player->stop();
                    }
                }
            }
            return;
        }
    }
}

void SoundPlayer::handleOSCEightFaders(const juce::OSCMessage& message)
{
    for (int i = 1; i < 9; i++)
    {
        auto playlistID = i < 5 ? 0 : 1;
        auto playerID = i < 5 ? i - 1 : i - 5;
        auto player = myPlaylists[playlistID]->players[playerID];

        juce::String adress = "/8fadersgain" + juce::String(i);
        if (message.getAddressPattern().matches(adress))
        {
            if (message.size() == 1 && message[0].isFloat32() && player != nullptr)
            {
                player->handleOSCMessage(message[0].getFloat32());
            }
            return;
        }
        adress = "/8faderspush" + juce::String(i);
        if (message.getAddressPattern().matches(adress))
        {
            if (message.size() == 1 && message[0].getFloat32() == 1)
            {
                playPlayer(i - 1);
            }
            return;
        }
    }
}

void SoundPlayer::handleOSCPlaylist(const juce::OSCMessage& message)
{
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
                            //juce::NormalisableRange<float>valueToSendNorm(0., juce::Decibels::decibelsToGain(Settings::maxFaderValueGlobal), 0.001, Settings::skewFactorGlobal);
                            //myPlaylists[0]->handleFader1OSC(0);
                            //myPlaylists[0]->handleFader1OSC(valueToSendNorm.convertTo0to1(1.));
                            myPlaylists[0]->fader1Start(true);
                            myPlaylists[0]->handleFader1OSC(1);
                            //fader1OSCLaunched = true;
                        }
                        else
                            myPlaylists[0]->fader1Stop(true);
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
                            myPlaylists[0]->fader2Start(true);
                            myPlaylists[0]->handleFader2OSC(1);
                        }
                        else
                            myPlaylists[0]->fader2Stop(true);
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
                        if (!myPlaylists[1]->isFader1Playing())
                        {
                            myPlaylists[1]->playFader(1);
                            if (Settings::lauchAtZeroDB)
                                myPlaylists[1]->handleFader1OSC(1);
                        }
                        else
                            myPlaylists[1]->stopFader(1);
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
                        if (!myPlaylists[1]->isFader2Playing())
                        {
                            myPlaylists[1]->playFader(2);
                            if (Settings::lauchAtZeroDB)
                                myPlaylists[1]->handleFader2OSC(1);
                        }
                        else
                            myPlaylists[1]->stopFader(2);
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
            if (!sender->send("/1/fader1", valueToSendNorm.convertTo0to1(valueToSend)))
            {
            }
        }
        else if (destination == 2)
        {
            float valueToSend = value / 127;
            juce::NormalisableRange<float>valueToSendNorm(0.0, 1, 0.001, 0.5, false);
            if (!sender->send("/1/fader2", valueToSendNorm.convertTo0to1(valueToSend)))
            {
            }
        }
        else if (destination == 3)
        {
            float valueToSend = value / 127;
            juce::NormalisableRange<float>valueToSendNorm(0.0, 1, 0.001, 0.5, false);
            if (!sender->send("/1/fader3", valueToSendNorm.convertTo0to1(valueToSend)))
            {
            }
        }
        else if (destination == 4)
        {
            float valueToSend = value / 127;
            juce::NormalisableRange<float>valueToSendNorm(0.0, 1, 0.001, 0.5, false);
            if (!sender->send("/1/fader4", valueToSendNorm.convertTo0to1(valueToSend)))
            {
            }
        }
        else if (destination == 5)
        {
            if (!sender->send("/1/meter", value))
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
            if (!sender->send("/1/remaining1", string))
            {
            }
        }
        else if (destination == 2)
        {
            if (!sender->send("/1/remaining2", string))
            {
            }
        }
        else if (destination == 3)
        {
            if (!sender->send("/1/remaining3", string))
            {
            }
        }
        else if (destination == 4)
        {
            if (!sender->send("/1/remaining4", string))
            {
            }
        }
        else if (destination == 5)
        {
            if (!sender->send("/1/soundname1", string))
            {
            }
        }
        else if (destination == 6)
        {
            if (!sender->send("/1/soundname2", string))
            {
            }
        }
        else if (destination == 7)
        {
            if (!sender->send("/1/soundname3", string))
            {
            }
        }
        else if (destination == 8)
        {
            if (!sender->send("/1/soundname4", string))
            {
            }
        }
    }
}