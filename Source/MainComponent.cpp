#include "MainComponent.h"
//#define INFO_BUFFER_SIZE 32767
//==============================================================================
bool MainComponent::exitAnswered;
MainComponent::MainComponent() : juce::AudioAppComponent(deviceManager),
                                            audioSetupComp(deviceManager, 0, 2, 0, 4, true, false, true, true),
                                 audioSetupWindow("Audiosetup",
                                     getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), true, true),
                                 settingsWindow("Settings",
                                     getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), true, true),
                                 settingsFile(options)
{

    logger.reset(juce::FileLogger::createDefaultAppLogger("Multiplayer", "Log.txt", "Multiplayer Log"));
    juce::FileLogger::setCurrentLogger(logger.get());
    juce::FileLogger::getCurrentLogger()->writeToLog("App launch");

    if (juce::RuntimePermissions::isRequired(juce::RuntimePermissions::recordAudio)
        && !juce::RuntimePermissions::isGranted(juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request(juce::RuntimePermissions::recordAudio,
            [&](bool granted) { setAudioChannels(granted ? 2 : 0, 4); });
    }
    else
    {
        setAudioChannels(8, 4);
    }

    juce::MultiTimer::startTimer(0, 50);
    juce::MultiTimer::startTimer(1, 3000);

    tryPreferedAudioDevice(2);
    deviceManager.initialise(2, 2, nullptr, true);

    if (deviceManager.getCurrentAudioDevice() != nullptr)
    {
        //settings = std::make_unique<Settings>();
        Settings::sampleRate = deviceManager.getAudioDeviceSetup().sampleRate;
    } 

    setWantsKeyboardFocus(true);

    menuBar.reset(new juce::MenuBarComponent(this));
    addAndMakeVisible(menuBar.get());
    menuBar->setWantsKeyboardFocus(false);

    /*keyMapper.reset(new KeyMapper(settings.get()));
    keyMapper->setSize(600, 400);*/

    /*midiMapper.reset(new MidiMapper());
    midiMapper->setSize(600, 400);*/

    initializeBottomComponent();

    audioSetupComp.setColour(juce::ResizableWindow::ColourIds::backgroundColourId, juce::Colours::white);
    audioSetupWindow.setSize(400, 800);
    audioSetupComp.setSize(400, 800);
    

    deviceManager.addChangeListener(this);
    Settings::audioOutputModeValue.addListener(this);
    Settings::showMeterValue.addListener(this);

    timerSlider.setRange(10, 1000);
    timerSlider.addListener(this);

    midiInitialized = false;
    midiInitialize();

    myLayout.setItemLayout(0, 200, 1080, -0.70);
    myLayout.setItemLayout(1, 4, 4, 4);
    myLayout.setItemLayout(2, 5, 600, 420);

    horizontalDividerBar.reset(new juce::StretchableLayoutResizerBar(&myLayout, 1, false));
    addAndMakeVisible(horizontalDividerBar.get());


    start1 = juce::Time::currentTimeMillis();

    juce::File convFile(Settings::convertedSoundsPath.toStdString());
    if (!convFile.isDirectory())
    {
        std::unique_ptr<juce::AlertWindow> noConvertSoundsWindow;
        noConvertSoundsWindow->showMessageBox(juce::AlertWindow::AlertIconType::InfoIcon, "", "Select Converted Sounds Folder");
        settingsButtonClicked();
    }

    SoundPlayer::Mode mode;
    int p = settings.getPreferedSoundPlayerMode();
    switch (p)
    {
    case 1:
        mode = SoundPlayer::Mode::OnePlaylistOneCart;
        break;
    case 2:
        mode = SoundPlayer::Mode::EightFaders;
        break;
    case 3:
        mode = SoundPlayer::Mode::KeyMap;
        break;
    default:
        mode = SoundPlayer::Mode::OnePlaylistOneCart;
    }

    launchSoundPlayer(mode);
    setApplicationCommandManagerToWatch(&commandManager);
    commandManager.registerAllCommandsForTarget(this);
    commandManager.registerAllCommandsForTarget(soundPlayers[0]);
    commandManager.getKeyMappings()->resetToDefaultMappings();


    km = new KeyMapper(&settings);
    km->setCommandManager(&commandManager);
    km->loadMappingFile();

    /*keyMapper->setCommandManager(&commandManager);
    keyMapper->loadMappingFile();*/
    midiMapper.setCommandManager(&commandManager);
    midiMapper.loadMappingFile();
    setMouseClickGrabsKeyboardFocus(true);
    addKeyListener(this);
    setWantsKeyboardFocus(true);

}

void MainComponent::settingsButtonClicked()
{  
    juce::FileLogger::getCurrentLogger()->writeToLog("Settings button clicked");
        //Settings* _settings = new Settings();
        settings.saveButton.addListener(this);
        juce::DialogWindow::LaunchOptions o;
        o.content.setNonOwned(&settings);
        o.content->setSize(600, 470);
        o.dialogTitle = TRANS("Settings");
        o.dialogBackgroundColour = getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId);
        o.escapeKeyTriggersCloseButton = true;
        o.useNativeTitleBar = false;
        o.resizable = false;

        if (oscConnected)
            OSCInitialize();

        o.launchAsync();
}

void MainComponent::audioSettingsButtonClicked()
{
    juce::FileLogger::getCurrentLogger()->writeToLog("Audio settings button clicked");
    audioSetupWindow.showDialog("Audio Setup", &audioSetupComp, this,
        getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), true, true, false);
}

void MainComponent::keyMapperButtonClicked()
{
    juce::FileLogger::getCurrentLogger()->writeToLog("Key mapper button cliked");
    std::unique_ptr<juce::DialogWindow> keyMapperWindow = std::make_unique<juce::DialogWindow>("Key Mapping",
        getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), true, true);
    keyMapperWindow->setSize(600, 400);

    //std::unique_ptr<KeyMapper> km = std::make_unique<KeyMapper>(settings.get());

    km->setWantsKeyboardFocus(true);

    juce::DialogWindow::LaunchOptions o;
    o.dialogBackgroundColour = getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId);
    o.content.setNonOwned(km);
    o.content->setSize(600, 400);
    o.escapeKeyTriggersCloseButton = true;
    o.useNativeTitleBar = false;
    o.dialogTitle = TRANS("Key Mapper");
    juce::DialogWindow* window = o.launchAsync();
    juce::ModalComponentManager::getInstance()->attachCallback(window, juce::ModalCallbackFunction::create([this](int r)
        {
            grabKeyboardFocus();
        }));
}


void MainComponent::midiMapperButtonClicked()
{
    juce::FileLogger::getCurrentLogger()->writeToLog("Midi mapper button clicked");
    juce::DialogWindow::LaunchOptions o;
    o.dialogBackgroundColour = getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId);
    o.content.setNonOwned(&midiMapper);
    o.content->setSize(600, 400);
    o.escapeKeyTriggersCloseButton = true;
    o.useNativeTitleBar = false;
    o.dialogTitle = TRANS("Key Mapper");
    juce::DialogWindow* window = o.launchAsync();
    juce::ModalComponentManager::getInstance()->attachCallback(window, juce::ModalCallbackFunction::create([this](int r)
        {
            midiMapper.setWantsKeyPress(false);
            grabKeyboardFocus();
        }));

}

void MainComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &bottomComponent.getTabbedButtonBar())
    {   //Update red colour on fx button on player if the clipeffect tab is selected
        if (bottomComponent.getTabbedButtonBar().getCurrentTabIndex() != 2)
        {
            for (auto* p : soundPlayers[0]->myPlaylists)
                p->resetFxEditedButtons();
        }
        else
            bottomComponent.clipEffect.setFxEditedPlayer();
    }
    if (source == &deviceManager) //save devicemanager state each time it changes
    {
        if (deviceManager.getCurrentAudioDevice() != nullptr)
        {
            Settings* settings = new Settings();
            settings->setPreferedAudioDeviceType(deviceManager.getCurrentAudioDeviceType());
            settings->setPreferedAudioDeviceName(deviceManager.getCurrentAudioDevice()->getName());
            settings->setPreferedAudioDevice(deviceManager.getAudioDeviceSetup());
            //Settings::sampleRateValue = deviceManager.getAudioDeviceSetup().sampleRate;
            settings->updateSampleRateValue(deviceManager.getAudioDeviceSetup().sampleRate);
            settings->saveOptions();
            delete settings;
            prepareToPlay(deviceManager.getAudioDeviceSetup().bufferSize, deviceManager.getAudioDeviceSetup().sampleRate);
            Settings::outputChannelsNumber = deviceManager.getCurrentAudioDevice()->getOutputChannelNames().size();
            auto midiInputs = juce::MidiInput::getAvailableDevices();
            for (auto input : midiInputs)
            {
                if (deviceManager.isMidiInputDeviceEnabled(input.identifier))
                     deviceManager.addMidiInputDeviceCallback(input.identifier, this);
                else
                    deviceManager.removeMidiInputDeviceCallback(input.identifier, this);
            }
            //bottomComponent.mixerPanel.inputPanel.channelEditor.setDeviceManagerInfos(deviceManager);
            mixer.prepareToPlay(deviceManager.getAudioDeviceSetup().bufferSize, deviceManager.getAudioDeviceSetup().sampleRate);
            mixer.inputPanel.channelEditor.setDeviceManagerInfos(deviceManager);
        }
    }
    else if (source == bottomComponent.audioPlaybackDemo.fileDraggedFromBrowser) 
    {
        std::unique_ptr<juce::StringArray> null = std::make_unique<juce::StringArray >();
        if (soundPlayers[0]->soundPlayerMode == SoundPlayer::Mode::KeyMap && soundPlayers[0]->keyMappedSoundboard != nullptr)
        {
            soundPlayers[0]->keyMappedSoundboard->fileDragMove(*null, getMouseXYRelative().getX(), getMouseXYRelative().getY());
        }
        else
        {
            int cartPosition = soundPlayers[0]->playlistbisViewport.getPosition().getX();
            int playlistScrollPosition = soundPlayers[0]->playlistViewport.getViewPositionY();
            int cartScrollPoisiton = soundPlayers[0]->playlistbisViewport.getViewPositionY();

            if (getMouseXYRelative().getX() < (soundPlayers[0]->myPlaylists[0]->getPosition().getX() + soundPlayers[0]->myPlaylists[0]->getWidth()))
                soundPlayers[0]->myPlaylists[0]->fileDragMove(*null, getMouseXYRelative().getX(), getMouseXYRelative().getY() - playlistStartY + playlistScrollPosition);
            else if (getMouseXYRelative().getX() > cartPosition)
                soundPlayers[0]->myPlaylists[1]->fileDragMove(*null, getMouseXYRelative().getX() - cartPosition, getMouseXYRelative().getY() - playlistStartY + cartScrollPoisiton);
            else
            {
                soundPlayers[0]->myPlaylists[0]->fileDragExit(*null);
                soundPlayers[0]->myPlaylists[1]->fileDragExit(*null);
            }
        }
        grabKeyboardFocus();
    }
    else if (source == bottomComponent.audioPlaybackDemo.fileDroppedFromBrowser)
    {
        //std::unique_ptr<Settings> settings = std::make_unique<Settings>();

        int cartPosition = soundPlayers[0]->playlistbisViewport.getPosition().getX();
        int playlistScrollPosition = soundPlayers[0]->playlistViewport.getViewPositionY();
        int cartScrollPoisiton = soundPlayers[0]->playlistbisViewport.getViewPositionY();
        std::unique_ptr<juce::StringArray> null = std::make_unique<juce::StringArray >();
        std::unique_ptr<juce::File> myFile = std::make_unique<juce::File>(bottomComponent.audioPlaybackDemo.fileBrowser->getSelectedFile(0));
        if (soundPlayers[0]->soundPlayerMode == SoundPlayer::Mode::KeyMap && soundPlayers[0]->keyMappedSoundboard != nullptr)
        {
            soundPlayers[0]->keyMappedSoundboard->setDroppedFile(getMouseXYRelative(), myFile->getFullPathName(), myFile->getFileNameWithoutExtension());
            soundPlayers[0]->keyMappedSoundboard->fileDragExit();
        }
        else
        {
            if (getMouseXYRelative().getX() < (soundPlayers[0]->myPlaylists[0]->getPosition().getX() + soundPlayers[0]->myPlaylists[0]->getWidth()))
                soundPlayers[0]->myPlaylists[0]->filesDropped(myFile->getFullPathName(), getMouseXYRelative().getX(), getMouseXYRelative().getY() - playlistStartY + playlistScrollPosition);
            else if (getMouseXYRelative().getX() > cartPosition)
                soundPlayers[0]->myPlaylists[1]->filesDropped(myFile->getFullPathName(), getMouseXYRelative().getX() - cartPosition, getMouseXYRelative().getY() - playlistStartY + cartScrollPoisiton);
            soundPlayers[0]->myPlaylists[0]->fileDragExit(*null);
            soundPlayers[0]->myPlaylists[1]->fileDragExit(*null);
        }
        grabKeyboardFocus();
    }
    else if (source == bottomComponent.recorderComponent.mouseDragInRecorder)//when mouse is dragged in recorder, desactivate shortcuts keys for players
    {
        soundPlayers[0]->draggedPlaylist = -1;
    }
    else if (source == bottomComponent.recorderComponent.spaceBarKeyPressed)//security to catch the spacebar if the focus is lost
        soundPlayers[0]->myPlaylists[0]->spaceBarPressed();
    //XOR Solo on cues
    else if (source == soundPlayers[0]->myPlaylists[0]->cuePlaylistBroadcaster)
    {
        soundPlayers[0]->myPlaylists[1]->stopCues();
        bottomComponent.stopCue();
    }
    else if (source == soundPlayers[0]->myPlaylists[1]->cuePlaylistBroadcaster)
    {
        soundPlayers[0]->myPlaylists[0]->stopCues();
        bottomComponent.stopCue();
    }
    else if (source == soundPlayers[0]->myPlaylists[0]->playBroadcaster
            || source == soundPlayers[0]->myPlaylists[1]->playBroadcaster)
    {
        if (Settings::audioOutputMode == 2)
        {
            soundPlayers[0]->myPlaylists[0]->stopCues();
            soundPlayers[0]->myPlaylists[1]->stopCues();
            bottomComponent.stopCue();
        }
    }
    else if (source == bottomComponent.cuePlay)
    {
        soundPlayers[0]->myPlaylists[0]->stopCues();
        soundPlayers[0]->myPlaylists[1]->stopCues();
    }
    else if (source == soundPlayers[0]->playerSelectionChanged)
    {
        bottomComponent.clipEffect.setDummyPlayer();
    }
    else if (source == soundPlayers[0]->myPlaylists[0]->fxButtonBroadcaster || source == soundPlayers[0]->myPlaylists[1]->fxButtonBroadcaster)
    {
        auto* player = soundPlayers[0]->myPlaylists[Settings::editedPlaylist]->players[Settings::editedPlayer];
        bottomComponent.clipEffect.setPlayer(player);
        bottomComponent.clipEditor.setPlayer(player);
        bottomComponent.setCurrentTabIndex(2);
    }
    else if (source == soundPlayers[0]->myPlaylists[0]->envButtonBroadcaster || source == soundPlayers[0]->myPlaylists[1]->envButtonBroadcaster)
    {
        auto* player = soundPlayers[0]->myPlaylists[Settings::editedPlaylist]->players[Settings::editedPlayer];
        bottomComponent.clipEditor.setPlayer(player);
        bottomComponent.clipEffect.setPlayer(player);
        bottomComponent.setCurrentTabIndex(1);
    }
    else if (source == soundPlayers[0]->myPlaylists[0]->playerLaunchedBroadcaster.get()
    || source == soundPlayers[0]->myPlaylists[1]->playerLaunchedBroadcaster.get())
    {
        auto* player = soundPlayers[0]->myPlaylists[Settings::editedPlaylist]->players[Settings::editedPlayer];
        bottomComponent.clipEditor.setPlayer(player);
        bottomComponent.clipEffect.setPlayer(player);
        bottomComponent.setCurrentTabIndex(1);
    }
    else if (source == soundPlayers[0]->playlistLoadedBroadcaster)
    {
        juce::Time::waitForMillisecondCounter(juce::Time::getMillisecondCounter() + 1000);
        bottomComponent.clipEffect.setNullPlayer();
        bottomComponent.clipEditor.setNullPlayer();
    }
    else if (source == soundPlayers[0]->myPlaylists[0]->grabFocusBroadcaster || source == soundPlayers[0]->myPlaylists[1]->grabFocusBroadcaster)
    {
        grabKeyboardFocus();
    }
    else if (source == bottomComponent.clipEditor.grabFocusBroadcaster.get())
    {
        grabKeyboardFocus();
    }
}

MainComponent::~MainComponent()
{
    juce::FileLogger::setCurrentLogger(nullptr);
    delete km;
    removeMouseListener(this);
    myMixer.removeAllInputs();
    myCueMixer.removeAllInputs();
    soundPlayers[0]->playerSelectionChanged->removeChangeListener(this);
    deviceManager.removeChangeListener(this);
    bottomComponent.recorderComponent.mouseDragInRecorder->removeChangeListener(this);
    Settings::audioOutputModeValue.removeListener(this);
    shutdownAudio();
}


void MainComponent::deleteConvertedFiles() //this delete the converted files since the opening of the application
{
    juce::FileLogger::getCurrentLogger()->writeToLog("Delete converted files");
    exitAnswered = true;
    while (Settings::tempFiles.size() > 0)
    {
        juce::File fileToDelete(Settings::tempFiles[0]);
        fileToDelete.deleteFile();
        Settings::tempFiles.remove(0);
    }
}

void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    mixer.prepareToPlay(samplesPerBlockExpected, sampleRate);
    myMixer.prepareToPlay(samplesPerBlockExpected, sampleRate);
    myCueMixer.prepareToPlay(samplesPerBlockExpected, sampleRate);
    bottomComponent.prepareToPlay(samplesPerBlockExpected, sampleRate);

    actualSamplesPerBlockExpected = samplesPerBlockExpected;
    actualSampleRate = sampleRate;

    mixerOutputBuffer = std::make_unique<juce::AudioBuffer<float>>(2, samplesPerBlockExpected);

}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    //TODO vérifier bug changement de soundplayer sur mode mono et double stereo
    if (soundPlayers[0] != nullptr && soundboardLaunched)
    {
        soundPlayers[0]->getNextAudioBlock(bufferToFill);
        if (Settings::audioOutputMode == 3 && (bufferToFill.buffer->getNumChannels() > 3))
        {
            inputBuffer.reset(new juce::AudioBuffer<float>(2, bufferToFill.buffer->getNumSamples()));
            inputBuffer->copyFrom(0, 0, *bufferToFill.buffer, 0, 0, bufferToFill.buffer->getNumSamples());
            inputBuffer->copyFrom(1, 0, *bufferToFill.buffer, 1, 0, bufferToFill.buffer->getNumSamples());
            bufferToFill.clearActiveBufferRegion();
            outputBuffer.reset(new juce::AudioBuffer<float>(2, bufferToFill.buffer->getNumSamples()));
            playAudioSource.reset(new juce::AudioSourceChannelInfo(*outputBuffer));
            cueAudioSource.reset(new juce::AudioSourceChannelInfo(*outputBuffer));

            myCueMixer.getNextAudioBlock(*cueAudioSource);
            bufferToFill.buffer->copyFrom(2, 0, *outputBuffer, 0, 0, bufferToFill.buffer->getNumSamples());
            bufferToFill.buffer->copyFrom(3, 0, *outputBuffer, 1, 0, bufferToFill.buffer->getNumSamples());
            soundPlayers[0]->cueloudnessMeter.processBlock(*outputBuffer);
            soundPlayers[0]->cuemeterSource.measureBlock(*outputBuffer);
            myMixer.getNextAudioBlock(*playAudioSource);
            if (bottomComponent.recorderComponent.isEnabled())
            {
                newOutputBuffer.reset(new juce::AudioBuffer<float>(2, bufferToFill.buffer->getNumSamples()));
                bottomComponent.recorderComponent.recordAudioBuffer(outputBuffer.get(), inputBuffer.get(), newOutputBuffer.get(), 2, actualSampleRate, bufferToFill.buffer->getNumSamples());

                //bufferToFill.clearActiveBufferRegion();
                bufferToFill.buffer->copyFrom(0, 0, *outputBuffer, 0, 0, bufferToFill.buffer->getNumSamples());
                bufferToFill.buffer->copyFrom(1, 0, *outputBuffer, 1, 0, bufferToFill.buffer->getNumSamples());
                if (soundPlayers[0] != nullptr)
                    soundPlayers[0]->loudnessMeter.processBlock(*outputBuffer);
                if (soundPlayers[0] != nullptr)
                    soundPlayers[0]->meterSource.measureBlock(*outputBuffer);
                if (bottomComponent.recorderComponent.enableMonitoring.getToggleState())
                {
                    bufferToFill.buffer->copyFrom(2, 0, *newOutputBuffer, 0, 0, bufferToFill.buffer->getNumSamples());
                    bufferToFill.buffer->copyFrom(3, 0, *newOutputBuffer, 1, 0, bufferToFill.buffer->getNumSamples());
                }

                //delete newOutputBuffer;
            }
            else
            {
                bufferToFill.buffer->copyFrom(0, 0, *outputBuffer, 0, 0, bufferToFill.buffer->getNumSamples());
                bufferToFill.buffer->copyFrom(1, 0, *outputBuffer, 1, 0, bufferToFill.buffer->getNumSamples());
                //soundPlayers[0]->loudnessMeter.processBlock(*outputBuffer);
                //soundPlayers[0]->meterSource.measureBlock(*outputBuffer);
            }
            /*delete(inputBuffer);
            delete(outputBuffer);
            delete(playAudioSource);*/
        }
        else if (Settings::audioOutputMode == 1)
        {
            inputBuffer.reset(new juce::AudioBuffer<float>(2, bufferToFill.buffer->getNumSamples()));
            inputBuffer->copyFrom(0, 0, *bufferToFill.buffer, 0, 0, bufferToFill.buffer->getNumSamples());
            outputBuffer.reset(new juce::AudioBuffer<float>(2, bufferToFill.buffer->getNumSamples()));
            playAudioSource.reset(new juce::AudioSourceChannelInfo(*outputBuffer));
            myMixer.getNextAudioBlock(*playAudioSource);


            bufferToFill.buffer->copyFrom(0, 0, *outputBuffer, 0, 0, bufferToFill.buffer->getNumSamples());
            if (!bottomComponent.recorderComponent.isEnabled())
            {
                if (soundPlayers[0] != nullptr)
                    soundPlayers[0]->loudnessMeter.processBlock(*outputBuffer);
                if (soundPlayers[0] != nullptr)
                    soundPlayers[0]->meterSource.measureBlock(*bufferToFill.buffer);
            }

            if (bottomComponent.recorderComponent.isEnabled())
            {
                newOutputBuffer.reset(new juce::AudioBuffer<float>(2, bufferToFill.buffer->getNumSamples()));
                bottomComponent.recorderComponent.recordAudioBuffer(outputBuffer.get(), inputBuffer.get(), newOutputBuffer.get(), 2, actualSampleRate, bufferToFill.buffer->getNumSamples());
                if (bottomComponent.recorderComponent.enableMonitoring.getToggleState())
                {
                    myCueMixer.getNextAudioBlock(*playAudioSource);
                    bufferToFill.buffer->copyFrom(1, 0, *outputBuffer, 0, 0, bufferToFill.buffer->getNumSamples());
                    soundPlayers[0]->cuemeterSource.measureBlock(*bufferToFill.buffer);
                    bufferToFill.buffer->addFrom(1, 0, *newOutputBuffer, 0, 0, bufferToFill.buffer->getNumSamples());
                    soundPlayers[0]->loudnessMeter.processBlock(*bufferToFill.buffer);
                    soundPlayers[0]->meterSource.measureBlock(*bufferToFill.buffer);
                }
                else
                {
                    myCueMixer.getNextAudioBlock(*playAudioSource);
                    bufferToFill.buffer->copyFrom(1, 0, *outputBuffer, 0, 0, bufferToFill.buffer->getNumSamples());
                    soundPlayers[0]->loudnessMeter.processBlock(*bufferToFill.buffer);
                    soundPlayers[0]->meterSource.measureBlock(*bufferToFill.buffer);
                    soundPlayers[0]->cueloudnessMeter.processBlock(*outputBuffer);
                    soundPlayers[0]->cuemeterSource.measureBlock(*bufferToFill.buffer);
                }
                //delete newOutputBuffer;
            }
            else
            {
                myCueMixer.getNextAudioBlock(*playAudioSource);
                soundPlayers[0]->cueloudnessMeter.processBlock(*outputBuffer);
                bufferToFill.buffer->copyFrom(1, 0, *outputBuffer, 0, 0, bufferToFill.buffer->getNumSamples());
                soundPlayers[0]->cuemeterSource.measureBlock(*bufferToFill.buffer);
            }
            //delete(outputBuffer);
            //delete(inputBuffer);
            //delete(playAudioSource);
        }
        else if (Settings::audioOutputMode == 2 || Settings::audioOutputMode == 3)
        {
            inputBuffer.reset(new juce::AudioBuffer<float>(2, bufferToFill.buffer->getNumSamples()));
            inputBuffer->copyFrom(0, 0, *bufferToFill.buffer, 0, 0, bufferToFill.buffer->getNumSamples());
            inputBuffer->copyFrom(1, 0, *bufferToFill.buffer, 1, 0, bufferToFill.buffer->getNumSamples());

            //MIXER
            if (showMixer)
                mixer.getNextAudioBlock(bufferToFill.buffer, mixerOutputBuffer.get());
            //

            bufferToFill.clearActiveBufferRegion();
            outputBuffer.reset(new juce::AudioBuffer<float>(2, bufferToFill.buffer->getNumSamples()));
            playAudioSource.reset(new juce::AudioSourceChannelInfo(*outputBuffer));
            myMixer.getNextAudioBlock(*playAudioSource);
            bufferToFill.buffer->copyFrom(0, 0, *outputBuffer, 0, 0, bufferToFill.buffer->getNumSamples());
            bufferToFill.buffer->copyFrom(1, 0, *outputBuffer, 1, 0, bufferToFill.buffer->getNumSamples());


            //MIXER
            if (showMixer)
            {
                bufferToFill.buffer->addFrom(0, 0, *mixerOutputBuffer, 0, 0, bufferToFill.buffer->getNumSamples());
                bufferToFill.buffer->addFrom(1, 0, *mixerOutputBuffer, 1, 0, bufferToFill.buffer->getNumSamples());
            }
            //

            if (!bottomComponent.recorderComponent.isEnabled())
            {
                if (soundPlayers[0] != nullptr)
                    soundPlayers[0]->meterSource.measureBlock(*outputBuffer);
                if (soundPlayers[0] != nullptr)
                    soundPlayers[0]->loudnessMeter.processBlock(*outputBuffer);
                if (soundPlayers[0] != nullptr)
                    soundPlayers[0]->newMeter->measureBlock(outputBuffer.get());
            }
            if (bottomComponent.recorderComponent.isEnabled() && soundPlayers[0] != nullptr)
            {
                newOutputBuffer.reset(new juce::AudioBuffer<float>(2, bufferToFill.buffer->getNumSamples()));
                bottomComponent.recorderComponent.recordAudioBuffer(outputBuffer.get(), inputBuffer.get(), newOutputBuffer.get(), 2, actualSampleRate, bufferToFill.buffer->getNumSamples());
                bufferToFill.clearActiveBufferRegion();
                bufferToFill.buffer->copyFrom(0, 0, *outputBuffer, 0, 0, bufferToFill.buffer->getNumSamples());
                bufferToFill.buffer->copyFrom(1, 0, *outputBuffer, 1, 0, bufferToFill.buffer->getNumSamples());
                soundPlayers[0]->meterSource.measureBlock(*outputBuffer);
                soundPlayers[0]->loudnessMeter.processBlock(*outputBuffer);
                if (bottomComponent.recorderComponent.enableMonitoring.getToggleState())
                {
                    bufferToFill.buffer->copyFrom(0, 0, *newOutputBuffer, 0, 0, bufferToFill.buffer->getNumSamples());
                    bufferToFill.buffer->copyFrom(1, 0, *newOutputBuffer, 1, 0, bufferToFill.buffer->getNumSamples());
                }
            }
        }
    }
}

void MainComponent::releaseResources()
{
}


void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.setColour(juce::Colour(40, 134, 189));
    g.fillRect(horizontalDividerBar.get()->getBounds());
    g.fillRect(0, mixer.getY() - 4, getWidth(), 4);
}

void MainComponent::resized()
{
    menuBar->setBounds(0, 0, getWidth(), menuBarHeight);
    timeLabel.setBounds(getWidth() - 220, 0, 200, 50);

    int windowWidth = getWidth();
    
    //reposition sound player
   if (soundPlayers[0] != nullptr)
        soundPlayers[0]->setBounds(0, playersStartHeightPosition, getWidth(), getHeight() - playersStartHeightPosition - bottomHeight);

    bottomComponent.setBounds(0, getHeight(), getWidth(), 25);
    if (showMixer)
        mixer.setBounds(0, getHeight() - mixerHeight, getWidth(), mixerHeight);
    //Layout the window
    horizontalDividerBar.get()->setBounds(0, 200 - bottomHeight, getWidth(), 8);
    Component* comps[] = { soundPlayers[0], horizontalDividerBar.get(), &bottomComponent };
    if (showMixer)
        myLayout.layOutComponents(comps, 3, 0, playersStartHeightPosition, getWidth(), getHeight() - playersStartHeightPosition - mixerHeight - 4, true, false);
    else
        myLayout.layOutComponents(comps, 3, 0, playersStartHeightPosition, getWidth(), getHeight() - playersStartHeightPosition - 4, true, false);
    
}


void MainComponent::timerCallback(int timerID)
{
    if (timerID == 0)
    {

    }
    else if (timerID == 1)
    {
        if (initializationFocusGained == false)
        {
            grabKeyboardFocus();
            initializationFocusGained = true;
        }
    }
}


void MainComponent::audioInitialize()
{

}


void MainComponent::midiInitialize()
{
    auto midiInputs = juce::MidiInput::getAvailableDevices();
    for (auto input : midiInputs)
    {
        juce::FileLogger::getCurrentLogger()->writeToLog("midi initialization");
        if (!deviceManager.isMidiInputDeviceEnabled(input.identifier))
            deviceManager.setMidiInputDeviceEnabled(input.identifier, true);
            deviceManager.addMidiInputDeviceCallback(input.identifier, this);
    }
}

void MainComponent::updateMidiInputs()
{
    midiInitialized = false;
    midiInitialize();
}

void MainComponent::setMidiInput(int index)
{
    juce::FileLogger::getCurrentLogger()->writeToLog("set midi input");
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
    if (midiMapper.getWantsKeyPress())
    {
        midiMapper.handleMidiMessage(message);
    }
    else
    {
        for (int i = 0; i < commandManager.getNumCommands(); i++)
        {
            juce::CommandID cID = commandManager.getCommandForIndex(i)->commandID;
            const juce::MessageManagerLock mmLock;
            if (message.getControllerNumber() == midiMapper.getMidiCCForCommand(cID) && message.getControllerValue() == 127)
                invokeDirectly(cID, true);
        }
        soundPlayers[0]->handleIncomingMidiMessage(source, message, &midiMapper);
    }
}





bool MainComponent::keyPressed(const juce::KeyPress &key, juce::Component* originatingComponent)
{
    auto command = commandManager.getKeyMappings()->findCommandForKeyPress(key);
    if (soundPlayers[0]->soundPlayerMode == SoundPlayer::Mode::KeyMap
        && soundPlayers[0]->keyMappedSoundboard != nullptr)
    {
        soundPlayers[0]->keyMappedSoundboard->keyPressed(key, originatingComponent);
    }
    else
        commandManager.invokeDirectly(command, true);
    return false;
}

void MainComponent::savePlaylist()
{
    juce::FileLogger::getCurrentLogger()->writeToLog("save playlist");
    soundPlayers[0]->savePlaylist();
    saved = true;
}

bool MainComponent::hasBeenSaved()
{
    return saved;
}

void MainComponent::loadPlaylist()
{
    juce::FileLogger::getCurrentLogger()->writeToLog("load playlist");
    soundPlayers[0]->loadPlaylist();
}

void MainComponent::setOptions(juce::String FFmpegPath, juce::String convertedSoundFolder, float skewFactor)
{

}

void MainComponent::valueChanged(juce::Value& value)
{
    if (value.refersToSameSourceAs(Settings::audioOutputModeValue))
    {
        channelsMapping();
    }
    else if (value.refersToSameSourceAs(Settings::showMeterValue))
    {
        if (soundPlayers[0]->myPlaylists[0] != nullptr)
            soundPlayers[0]->myPlaylists[0]->shouldShowMeters(Settings::showMeterValue.getValue());
        if (soundPlayers[0]->myPlaylists[1] != nullptr)
            soundPlayers[0]->myPlaylists[1]->shouldShowMeters(Settings::showMeterValue.getValue());
    }
}


void MainComponent::OSCInitialize()
{
    juce::FileLogger::getCurrentLogger()->writeToLog("Osc initialize");
    soundPlayers[0]->OSCInitialize();
    if (soundPlayers[0]->oscConnected == true)
    {
        connectOSCButton.setButtonText("Disconnect OSC");
    }
    else 
    {
        connectOSCButton.setButtonText("Connect OSC");
    }
}


void MainComponent::buttonClicked(juce::Button* button)
{
    channelsMapping();
    repaint();
}


void MainComponent::mouseDown(const juce::MouseEvent& event)
{

}

void MainComponent::channelsMapping()
{

    myMixer.removeAllInputs();
    myCueMixer.removeAllInputs();

    if (Settings::audioOutputMode == 1)
    {
        juce::FileLogger::getCurrentLogger()->writeToLog("Cannels mapping mode 1");
        tryPreferedAudioDevice(2);
        setAudioChannels(2, 2);

        myMixer.addInputSource(&soundPlayers[0]->myMixer, false);
        myCueMixer.addInputSource(&soundPlayers[0]->myCueMixer, false);
        myCueMixer.addInputSource(&bottomComponent.myMixer, false);

    }
    else if (Settings::audioOutputMode == 2)
    {
        juce::FileLogger::getCurrentLogger()->writeToLog("Channels mapping mode 2");
        tryPreferedAudioDevice(2);
        setAudioChannels(2, 2);
        myMixer.addInputSource(&soundPlayers[0]->myMixer, false);
        myMixer.addInputSource(&soundPlayers[0]->myCueMixer, false);
        myMixer.addInputSource(&bottomComponent.myMixer, false);

    }
    else if (Settings::audioOutputMode == 3)
    {
        juce::FileLogger::getCurrentLogger()->writeToLog("Channels mapping mode 3");
        tryPreferedAudioDevice(4);
        setAudioChannels(2, 4);
        myMixer.addInputSource(&soundPlayers[0]->myMixer, false);
        myCueMixer.addInputSource(&bottomComponent.myMixer, false);
        myCueMixer.addInputSource(&soundPlayers[0]->myCueMixer, false);
    }
}


void MainComponent::tryPreferedAudioDevice(int outputChannelsNeeded)
{
    juce::FileLogger::getCurrentLogger()->writeToLog("Trying prefered audio device");
    if (deviceManager.getCurrentAudioDevice() != nullptr)
    {
        //std::unique_ptr<Settings> settings = std::make_unique<Settings>();
        const juce::OwnedArray<juce::AudioIODeviceType>& types = deviceManager.getAvailableDeviceTypes();
        if (types.size() > 1)
        {
            for (int i = 0; i < types.size(); ++i)
            {
                if (types[i]->getTypeName() == Settings::preferedAudioDeviceType)
                {
                    deviceManager.setCurrentAudioDeviceType(Settings::preferedAudioDeviceType, true);
                    deviceManager.initialise(2, outputChannelsNeeded, nullptr, true, Settings::preferedAudioDeviceName, &settings.getPreferedAudioDevice());
                    if (deviceManager.getCurrentAudioDevice() != nullptr)
                        Settings::outputChannelsNumber = deviceManager.getCurrentAudioDevice()->getOutputChannelNames().size();
                }
            }
        }
    }
    else
        deviceManager.initialise(2, outputChannelsNeeded, nullptr, true);

    if (deviceManager.getCurrentAudioDevice() != nullptr)
        Settings::outputChannelsNumber = deviceManager.getCurrentAudioDevice()->getOutputChannelNames().size();

}


void MainComponent::defaultPlayersPlaylist(int playersToAdd)
{
    juce::FileLogger::getCurrentLogger()->writeToLog("defaulting player playlist");
    if (soundPlayers[0]->soundPlayerMode != SoundPlayer::Mode::KeyMap)
    {
        for (auto i = -1; playersToAdd < 1; ++i)
        {
            soundPlayers[0]->myPlaylists[0]->addPlayer(i);
            soundPlayers[0]->myPlaylists[1]->addPlayer(i);
        }
        soundPlayers[0]->myPlaylists[1]->assignLeftFader(-1);
        soundPlayers[0]->myPlaylists[1]->assignLeftFader(0);
        soundPlayers[0]->myPlaylists[1]->assignRightFader(-1);
        soundPlayers[0]->myPlaylists[1]->assignRightFader(1);
    }
    else
    {
        soundPlayers[0]->initializeKeyMapPlayer();
    }
}




void MainComponent::MidiOutput(const juce::MidiMessage& message)
{
    auto availableOutputDevices = juce::MidiOutput::getAvailableDevices();
    if (availableOutputDevices.size() > 0)
    {
        for (int i = 0; i < availableOutputDevices.size(); i++)
        {

            auto midiOutput = juce::MidiOutput::openDevice(availableOutputDevices[i].name);
            midiOutput->sendMessageNow(message);
        }
    }

}


void MainComponent::sliderValueChanged(juce::Slider* slider)
{
    const juce::MessageManagerLock mmLock;
    soundPlayers[0]->setTimerTime((int)slider->getValue());
}


void MainComponent::launchRecord()
{
    const juce::MessageManagerLock mmLock;
    if (!bottomComponent.recorderComponent.isRecorderEnabled)
    {
        bottomComponent.recorderComponent.enabledButton.setToggleState(true, juce::NotificationType::sendNotification);
    }
    bottomComponent.recorderComponent.recordButtonClicked();
    bottomComponent.setCurrentTabIndex(3);
    myLayout.setItemLayout(2,          // for item 1
        25, 600, // size must be between 20% and 60% of the available space
        300);        // and its preferred size is 50 pixels
    Component* comps[] = { soundPlayers[0], horizontalDividerBar.get(), &bottomComponent };
    myLayout.layOutComponents(comps, 4, 0, playersStartHeightPosition, getWidth(), getHeight() - playersStartHeightPosition, true, false);
}

bool MainComponent::isPlayingOrRecording()
{
    return soundPlayers[0]->isPlaying();
}

void MainComponent::setCommandLine(juce::String commandLine)
{
    if (!commandLine.compare("8p"))
    {
        launchSoundPlayer(SoundPlayer::Mode::EightFaders);
    }

}
void MainComponent::launchSoundPlayer(SoundPlayer::Mode m)
{
    soundboardLaunched = false;
    switch (m)
    {
    case SoundPlayer::Mode::OnePlaylistOneCart : 
        soundPlayerTypeSelector.setSelectedId(1, juce::NotificationType::dontSendNotification);
        break;
    case SoundPlayer::Mode::EightFaders :
        soundPlayerTypeSelector.setSelectedId(2, juce::NotificationType::dontSendNotification);
        break;
    case SoundPlayer::Mode::KeyMap :
        soundPlayerTypeSelector.setSelectedId(3, juce::NotificationType::dontSendNotification);
        break;
    }

    myMixer.removeAllInputs();
    myCueMixer.removeAllInputs();
    soundPlayers.clear();
    soundPlayers.add(new SoundPlayer(m));
    addAndMakeVisible(soundPlayers[0]);
    soundPlayers[0]->prepareToPlay(actualSamplesPerBlockExpected, actualSampleRate);
    soundPlayers[0]->setWantsKeyboardFocus(false);
    myMixer.addInputSource(&soundPlayers[0]->myMixer, false);
    myCueMixer.addInputSource(&soundPlayers[0]->myCueMixer, false);

    if (m == SoundPlayer::Mode::EightFaders)
    {
        juce::FileLogger::getCurrentLogger()->writeToLog("launching eight players mode");
        isEightPlayerMode = true;
        soundPlayers[0]->myPlaylists[0]->isEightPlayerMode(true);
        soundPlayers[0]->myPlaylists[0]->setPlaylistPosition(0);
        soundPlayers[0]->myPlaylists[1]->isEightPlayerMode(true);
        soundPlayers[0]->myPlaylists[1]->setPlaylistPosition(1);
        soundPlayers[0]->myPlaylists[1]->setEightPlayersSecondCart(true);
        eightPlayersLaunched = true;
        for (auto i = -1; i < 3; ++i)
        {
            soundPlayers[0]->myPlaylists[0]->addPlayer(i);
            soundPlayers[0]->myPlaylists[1]->addPlayer(i);
        }
    }
    else 
    {
        juce::FileLogger::getCurrentLogger()->writeToLog("launching playlist mode");
        for (auto i = -1; i < 1; ++i)
        {
            soundPlayers[0]->myPlaylists[0]->addPlayer(i);
            soundPlayers[0]->myPlaylists[1]->addPlayer(i);
        }
        //for (auto player : myPlaylists[0]->players)
        //    player->setPlayerColour(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

        soundPlayers[0]->myPlaylists[1]->assignLeftFader(-1);
        soundPlayers[0]->myPlaylists[1]->assignLeftFader(0);
        soundPlayers[0]->myPlaylists[1]->assignRightFader(-1);
        soundPlayers[0]->myPlaylists[1]->assignRightFader(1);
        soundPlayers[0]->myPlaylists[0]->setPlaylistPosition(0);
        soundPlayers[0]->myPlaylists[1]->setPlaylistPosition(1);
    }
    if (m == SoundPlayer::Mode::KeyMap)
    {
        juce::FileLogger::getCurrentLogger()->writeToLog("Launching keymapped mode");
        soundPlayers[0]->initializeKeyMapPlayer();
        removeKeyListener(commandManager.getKeyMappings());
        getTopLevelComponent()->removeKeyListener(commandManager.getKeyMappings());
    }
    else
    {
        //getTopLevelComponent()->addKeyListener(commandManager.getKeyMappings());
        //addKeyListener(commandManager.getKeyMappings());
    }

    soundPlayers[0]->playerSelectionChanged->addChangeListener(this);
    soundPlayers[0]->playlistLoadedBroadcaster->addChangeListener(this);
    soundPlayers[0]->myPlaylists[0]->cuePlaylistBroadcaster->addChangeListener(this);
    soundPlayers[0]->myPlaylists[0]->playBroadcaster->addChangeListener(this);
    soundPlayers[0]->myPlaylists[1]->cuePlaylistBroadcaster->addChangeListener(this);
    soundPlayers[0]->myPlaylists[1]->playBroadcaster->addChangeListener(this);
    soundPlayers[0]->myPlaylists[0]->fxButtonBroadcaster->addChangeListener(this);
    soundPlayers[0]->myPlaylists[1]->fxButtonBroadcaster->addChangeListener(this);
    soundPlayers[0]->myPlaylists[0]->envButtonBroadcaster->addChangeListener(this);
    soundPlayers[0]->myPlaylists[1]->envButtonBroadcaster->addChangeListener(this);
    soundPlayers[0]->myPlaylists[0]->grabFocusBroadcaster->addChangeListener(this);
    soundPlayers[0]->myPlaylists[1]->grabFocusBroadcaster->addChangeListener(this);
    soundPlayers[0]->myPlaylists[0]->playerLaunchedBroadcaster->addChangeListener(this);
    soundPlayers[0]->myPlaylists[1]->playerLaunchedBroadcaster->addChangeListener(this);

    soundPlayers[0]->setMouseClickGrabsKeyboardFocus(false);

    if (soundPlayers[0] != nullptr)
        soundPlayers[0]->setBounds(0, playersStartHeightPosition, getWidth(), getHeight() - playersStartHeightPosition - bottomHeight);
     
    Component* comps[] = { soundPlayers[0], horizontalDividerBar.get(), &bottomComponent };
    myLayout.layOutComponents(comps, 3, 0, playersStartHeightPosition, getWidth(), getHeight() - playersStartHeightPosition, true, false);

    channelsMapping();



    soundboardLaunched = true;
}

void MainComponent::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &soundPlayerTypeSelector)
    {
        int r = soundPlayerTypeSelector.getSelectedId();
        switch (r)
        {
        case 1 :
            launchSoundPlayer(SoundPlayer::Mode::OnePlaylistOneCart);
            break;
        case 2 :
            launchSoundPlayer(SoundPlayer::Mode::EightFaders);
            break;
        case 3 :
            launchSoundPlayer(SoundPlayer::Mode::KeyMap);
            break;
        }
        settings.setPreferedSoundPlayerMode(r);
    }
}


void MainComponent::modifierKeysChanged(const juce::ModifierKeys& modifiers)
{
    if (GetKeyState(VK_CONTROL) & 0x8000)
    {
        
    }
}

void MainComponent::stopWatchShortcuPressed()
{
    soundPlayers[0]->mainStopWatch.setVisible(true);
    soundPlayers[0]->mainStopWatch.startStopButtonClicked();
}

juce::StringArray MainComponent::getMenuBarNames()
{
    return { "File", "Settings", "View", "Soundplayer Mode", "Help" };
}

juce::PopupMenu MainComponent::getMenuForIndex(int menuIndex, const juce::String& menuName)
{
    juce::PopupMenu menu;

    if (menuIndex == 0)
    {
        menu.addCommandItem(&commandManager, CommandIDs::open, "Open");
        menu.addCommandItem(&commandManager, CommandIDs::save, "Save");
        menu.addSeparator();
        menu.addCommandItem(&commandManager, CommandIDs::open, "Quit");
    }
    else if (menuIndex == 1)
    {
        menu.addCommandItem(&commandManager, CommandIDs::openSettings, "General settings");
        menu.addCommandItem(&commandManager, CommandIDs::audioSettings, "Audio settings");
        menu.addCommandItem(&commandManager, CommandIDs::keyboardMapping, "Keyboard shortcuts");
        menu.addCommandItem(&commandManager, CommandIDs::midiMapping, "Midi mapping");
    }
    else if (menuIndex == 2)
    {
        menu.addCommandItem(&commandManager, CommandIDs::showIndividualMeters, "Show individuals meters");
        menu.addCommandItem(&commandManager, CommandIDs::showTimer, "Show timer");
        menu.addCommandItem(&commandManager, CommandIDs::showEnveloppe, "Show enveloppe on clip");
        menu.addCommandItem(&commandManager, CommandIDs::viewLastPlayedSound, "Show last played sound in panel");
    }
    else if (menuIndex == 3)
    {
        menu.addCommandItem(&commandManager, CommandIDs::launchPlaylist, "One playlist, one cart");
        menu.addCommandItem(&commandManager, CommandIDs::launch8Faders, "Eight faders");
        menu.addCommandItem(&commandManager, CommandIDs::launchKeyMapped, "Keyboard Mapped");
    }
    else if (menuIndex == 4)
    {
        menu.addItem(1, "Documentation");
        menu.addItem(2, "About");
    }


    return menu;

}

void MainComponent::menuItemSelected(int menuItemID, int topLevelMenuIndex)
{

}

void MainComponent::initializeBottomComponent()
{
    //ADD MIXER
    if (!showMixer)
        mixerHeight = 0;
    addAndMakeVisible(&mixer);
    //ADD BOTTOM COMPONENT
    addAndMakeVisible(bottomComponent);
    myMixer.addInputSource(&bottomComponent.myMixer, false);
    bottomComponent.audioPlaybackDemo.fileDraggedFromBrowser->addChangeListener(this);
    bottomComponent.audioPlaybackDemo.fileDroppedFromBrowser->addChangeListener(this);
    bottomComponent.recorderComponent.mouseDragInRecorder->addChangeListener(this);
    bottomComponent.recorderComponent.spaceBarKeyPressed->addChangeListener(this);
    bottomComponent.cuePlay->addChangeListener(this);
    bottomComponent.clipEditor.grabFocusBroadcaster->addChangeListener(this);

    bottomComponent.setName("bottom component");
    bottomComponent.setWantsKeyboardFocus(false);
    bottomComponent.getTabbedButtonBar().addChangeListener(this);
    bottomComponent.getTabbedButtonBar().getTabButton(0)->addMouseListener(this, false);
    bottomComponent.getTabbedButtonBar().getTabButton(1)->addMouseListener(this, false);
    bottomComponent.getTabbedButtonBar().getTabButton(2)->addMouseListener(this, false);
    bottomComponent.getTabbedButtonBar().getTabButton(3)->addMouseListener(this, false);
}


juce::ApplicationCommandTarget* MainComponent::getNextCommandTarget()
{
    return soundPlayers[0];
}


void MainComponent::getAllCommands(juce::Array<juce::CommandID>& commands)
{
    juce::Array<juce::CommandID> c{ CommandIDs::startTimer,
                                    CommandIDs::showTimer,
                                    CommandIDs::showIndividualMeters,
                                    CommandIDs::showEnveloppe,
                                    CommandIDs::viewLastPlayedSound,
                                    CommandIDs::lanchRecord,
                                    CommandIDs::goToSoundBrowser,
                                    CommandIDs::goToRecorder,
                                    CommandIDs::goToClipEditor,
                                    CommandIDs::goToClipEffect,
                                    CommandIDs::spaceBarPlay,
                                    CommandIDs::goToFirst,
                                    CommandIDs::goToNext,
                                    CommandIDs::goToPrevious,
                                    CommandIDs::play1,
                                    CommandIDs::play2,
                                    CommandIDs::play3,
                                    CommandIDs::play4,
                                    CommandIDs::play5,
                                    CommandIDs::play6,
                                    CommandIDs::play7,
                                    CommandIDs::play8,
                                    CommandIDs::play9,
                                    CommandIDs::play10,
                                    CommandIDs::play11,
                                    CommandIDs::play12,
                                    CommandIDs::save,
                                    CommandIDs::open,
                                    CommandIDs::quit,
                                    CommandIDs::openSettings,
                                    CommandIDs::audioSettings,
                                    CommandIDs::keyboardMapping,
                                    CommandIDs::midiMapping,
                                    CommandIDs::launchPlaylist,
                                    CommandIDs::launch8Faders,
                                    CommandIDs::launchKeyMapped,
                                    CommandIDs::setInMark,
                                    CommandIDs::deleteInMark,
                                    CommandIDs::setOutMark,
                                    CommandIDs::deleteOutMark };
    commands.addArray(c);
}

void MainComponent::getCommandInfo(juce::CommandID commandID, juce::ApplicationCommandInfo& result)
{
    switch (commandID)
    {
    case CommandIDs::startTimer:
        result.setInfo("Start Timer", "Start Timer", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress('t', juce::ModifierKeys::noModifiers);
        break;
    case CommandIDs::showTimer:
        result.setInfo("Show / hide timer", "Show / hide timer", "Menu", 0);
        result.setTicked(soundPlayers[0]->mainStopWatch.isVisible());
        result.addDefaultKeypress('t', juce::ModifierKeys::commandModifier);
        break;
    case CommandIDs::lanchRecord:
        result.setInfo("Launch record", "Launch record", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress('r', juce::ModifierKeys::noModifiers);
        break;
    case CommandIDs::goToSoundBrowser:
        result.setInfo("Display sound browser", "Display sound browser", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress('1', juce::ModifierKeys::noModifiers);
        break;
    case CommandIDs::goToRecorder:
        result.setInfo("Display recorder", "Display recorder", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress('4', juce::ModifierKeys::noModifiers);
        break;
    case CommandIDs::goToClipEditor:
        result.setInfo("Display clip editor", "Display clip editor", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress('2', juce::ModifierKeys::noModifiers);
        break;
    case CommandIDs::goToClipEffect:
        result.setInfo("Display clip effect", "Display clip effect", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress('3', juce::ModifierKeys::noModifiers);
        break;
    case CommandIDs::spaceBarPlay:
        result.setInfo("Play next sound in playlist (playlist mode only) or cue play (other modes)", "Play", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress(juce::KeyPress::spaceKey, juce::ModifierKeys::noModifiers);
        break;
    case CommandIDs::goToFirst:
        result.setInfo("Go to first sound in playlist", "Go to first", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress(juce::KeyPress::escapeKey, juce::ModifierKeys::noModifiers);
        break;
    case CommandIDs::goToNext:
        result.setInfo("Go to next sound in playlist", "Go to next", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress(juce::KeyPress::pageDownKey, juce::ModifierKeys::noModifiers);
        break;
    case CommandIDs::goToPrevious:
        result.setInfo("Go to previous sound in playlist", "Go to previous", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress(juce::KeyPress::pageUpKey, juce::ModifierKeys::noModifiers);
        break;
    case CommandIDs::play1:
        result.setInfo("Play cart 1 (playlist) or player 1 (8 players mode)", "Play cart 1 (playlist) or player 1 (8 players mode)", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress(juce::KeyPress::F1Key, juce::ModifierKeys::noModifiers);
        break;
    case CommandIDs::play2:
        result.setInfo("Play cart 2 (playlist) or player 21 (8 players mode)", "Play cart 1 (playlist) or player 1 (8 players mode)", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress(juce::KeyPress::F2Key, juce::ModifierKeys::noModifiers);
        break;
    case CommandIDs::play3:
        result.setInfo("Play cart 3 (playlist) or player 3 (8 players mode)", "Play cart 1 (playlist) or player 1 (8 players mode)", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress(juce::KeyPress::F3Key, juce::ModifierKeys::noModifiers);
        break;
    case CommandIDs::play4:
        result.setInfo("Play cart 4 (playlist) or player 4 (8 players mode)", "Play cart 1 (playlist) or player 1 (8 players mode)", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress(juce::KeyPress::F4Key, juce::ModifierKeys::noModifiers);
        break;
    case CommandIDs::play5:
        result.setInfo("Play cart 5 (playlist) or player 5 (8 players mode)", "Play cart 1 (playlist) or player 1 (8 players mode)", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress(juce::KeyPress::F5Key, juce::ModifierKeys::noModifiers);
        break;
    case CommandIDs::play6:
        result.setInfo("Play cart 6 (playlist) or player 6 (8 players mode)", "Play cart 1 (playlist) or player 1 (8 players mode)", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress(juce::KeyPress::F6Key, juce::ModifierKeys::noModifiers);
        break;
    case CommandIDs::play7:
        result.setInfo("Play cart 7 (playlist) or player 7 (8 players mode)", "Play cart 1 (playlist) or player 1 (8 players mode)", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress(juce::KeyPress::F7Key, juce::ModifierKeys::noModifiers);
        break;
    case CommandIDs::play8:
        result.setInfo("Play cart 8 (playlist) or player 8 (8 players mode)", "Play cart 1 (playlist) or player 1 (8 players mode)", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress(juce::KeyPress::F8Key, juce::ModifierKeys::noModifiers);
        break;
    case CommandIDs::play9:
        result.setInfo("Play cart 9 (playlist)", "Play cart 9 (playlist)", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress(juce::KeyPress::F9Key, juce::ModifierKeys::noModifiers);
        break;
    case CommandIDs::play10:
        result.setInfo("Play cart 10 (playlist)", "Play cart 10 (playlist)", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress(juce::KeyPress::F10Key, juce::ModifierKeys::noModifiers);
        break;
    case CommandIDs::play11:
        result.setInfo("Play cart 11 (playlist)", "Play cart 11 (playlist)", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress(juce::KeyPress::F11Key, juce::ModifierKeys::noModifiers);
        break;
    case CommandIDs::play12:
        result.setInfo("Play cart 12 (playlist)", "Play cart 12 (playlist)", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress(juce::KeyPress::F12Key, juce::ModifierKeys::noModifiers);
        break;
    case CommandIDs::save:
        result.setInfo("Save", "Save", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress('s', juce::ModifierKeys::commandModifier);
        break;
    case CommandIDs::open:
        result.setInfo("Open", "Open", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress('o', juce::ModifierKeys::commandModifier);
        break;
    case CommandIDs::quit:
        result.setInfo("Quit", "Quit", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress('q', juce::ModifierKeys::commandModifier);
        break;
    case CommandIDs::openSettings:
        result.setInfo("Open general settings", "Open Settings", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress('a', juce::ModifierKeys::commandModifier);
        break;
    case CommandIDs::audioSettings:
        result.setInfo("Open audio settings", "Open audio settings", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress('z', juce::ModifierKeys::commandModifier);
        break;
    case CommandIDs::keyboardMapping:
        result.setInfo("Open keyboard mapping", "Open keyboard mapping", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress('e', juce::ModifierKeys::commandModifier);
        break;
    case CommandIDs::midiMapping:
        result.setInfo("Open midi mapping", "Open midi mappings", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress('r', juce::ModifierKeys::commandModifier);
        break;
    case CommandIDs::launchPlaylist:
        result.setInfo("Launch playlist mode", "Launch playlist mode", "Menu", 0);
        result.setTicked(soundPlayers[0]->getSoundPlayerMode() == SoundPlayer::Mode::OnePlaylistOneCart ? true : false);
        result.addDefaultKeypress(juce::KeyPress::F1Key, juce::ModifierKeys::altModifier);
        break;
    case CommandIDs::launch8Faders:
        result.setInfo("Launch eight faders mode", "Launch eight faders mode", "Menu", 0);
        result.setTicked(soundPlayers[0]->getSoundPlayerMode() == SoundPlayer::Mode::EightFaders ? true : false);
        result.addDefaultKeypress(juce::KeyPress::F2Key, juce::ModifierKeys::altModifier);
        break;
    case CommandIDs::launchKeyMapped:
        result.setInfo("Launch key map mode", "Launch key map mode", "Menu", 0);
        result.setTicked(soundPlayers[0]->getSoundPlayerMode() == SoundPlayer::Mode::KeyMap ? true : false);
        result.addDefaultKeypress(juce::KeyPress::F3Key, juce::ModifierKeys::altModifier);
        break;
    case CommandIDs::setInMark:
        result.setInfo("Set in mark", "Set in mark at cue position cursor", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress('i', juce::ModifierKeys::noModifiers);
        break;
    case CommandIDs::deleteInMark:
        result.setInfo("Delete in mark", "Delete in mark", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress('k', juce::ModifierKeys::noModifiers);
        break;
    case CommandIDs::setOutMark:
        result.setInfo("Set out mark", "Set out mark at cue position cursor", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress('o', juce::ModifierKeys::noModifiers);
        break;
    case CommandIDs::deleteOutMark:
        result.setInfo("Delete out mark", "Delete out mark", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress('l', juce::ModifierKeys::noModifiers);
        break;
    case CommandIDs::showIndividualMeters:
        result.setInfo("Show individuals meters", "Show meter", "Menu", 0);
        result.setTicked(Settings::showMeter);
        break;
    case CommandIDs::showEnveloppe:
        result.setInfo("Show enveloppes on clip", "Show enveloppe", "Menu", 0);
        result.setTicked(Settings::showEnveloppe);
        break;
    case CommandIDs::viewLastPlayedSound:
        result.setInfo("Automatic show last played sound in bottom panel", "View last played sound", "Menu", 0);
        result.setTicked(Settings::viewLastPlayedSound);
        break;
    default:
        break;
    }
}

bool MainComponent::perform(const InvocationInfo& info)
{
    switch (info.commandID)
    {
    case CommandIDs::startTimer:
        juce::FileLogger::getCurrentLogger()->writeToLog("start timer");
        stopWatchShortcuPressed();
        break;
    case CommandIDs::showTimer:
        juce::FileLogger::getCurrentLogger()->writeToLog("show timer");
        soundPlayers[0]->mainStopWatch.setVisible(!soundPlayers[0]->mainStopWatch.isVisible());
        break;
    case CommandIDs::lanchRecord:
        juce::FileLogger::getCurrentLogger()->writeToLog("launch record");
        launchRecord();
        break;
    case CommandIDs::goToSoundBrowser:
        juce::FileLogger::getCurrentLogger()->writeToLog("view sound browser");
        bottomComponent.getTabbedButtonBar().setCurrentTabIndex(0);
        break;
    case CommandIDs::goToRecorder:
        juce::FileLogger::getCurrentLogger()->writeToLog("view recorder");
        bottomComponent.getTabbedButtonBar().setCurrentTabIndex(3);
        break;
    case CommandIDs::goToClipEditor:
        juce::FileLogger::getCurrentLogger()->writeToLog("view clip editor");
        bottomComponent.getTabbedButtonBar().setCurrentTabIndex(1);
        break;
    case CommandIDs::goToClipEffect:
        juce::FileLogger::getCurrentLogger()->writeToLog("view clip effect");
        bottomComponent.getTabbedButtonBar().setCurrentTabIndex(2);
        break;
    case CommandIDs::spaceBarPlay:
        juce::FileLogger::getCurrentLogger()->writeToLog("space bar play");
        if (soundPlayers[0]->soundPlayerMode == SoundPlayer::Mode::OnePlaylistOneCart)
            soundPlayers[0]->myPlaylists[0]->spaceBarPressed();
        else
            bottomComponent.spaceBarPressed();
        break;
    case CommandIDs::goToPrevious:
        juce::FileLogger::getCurrentLogger()->writeToLog("go to previous");
        if (soundPlayers[0]->soundPlayerMode == SoundPlayer::Mode::OnePlaylistOneCart)
            soundPlayers[0]->myPlaylists[0]->playersPreviousPositionClicked();
        break;
    case CommandIDs::goToFirst:
        juce::FileLogger::getCurrentLogger()->writeToLog("go to first");
        if (soundPlayers[0]->soundPlayerMode == SoundPlayer::Mode::OnePlaylistOneCart)
            soundPlayers[0]->myPlaylists[0]->playersResetPositionClicked();
        break;
    case CommandIDs::goToNext:
        juce::FileLogger::getCurrentLogger()->writeToLog("go to next");
        if (soundPlayers[0]->soundPlayerMode == SoundPlayer::Mode::OnePlaylistOneCart)
            soundPlayers[0]->myPlaylists[0]->playersNextPositionClicked();
        break;
    case CommandIDs::play1:
        soundPlayers[0]->playPlayer(0);
        break;
    case CommandIDs::play2:
        soundPlayers[0]->playPlayer(1);
        break;
    case CommandIDs::play3:
        soundPlayers[0]->playPlayer(2);
        break;
    case CommandIDs::play4:
        soundPlayers[0]->playPlayer(3);
        break;
    case CommandIDs::play5:
        soundPlayers[0]->playPlayer(4);
        break;
    case CommandIDs::play6:
        soundPlayers[0]->playPlayer(5);
        break;
    case CommandIDs::play7:
        soundPlayers[0]->playPlayer(6);
        break;
    case CommandIDs::play8:
        soundPlayers[0]->playPlayer(7);
        break;
    case CommandIDs::play9:
        soundPlayers[0]->playPlayer(8);
        break;
    case CommandIDs::play10:
        soundPlayers[0]->playPlayer(9);
        break;
    case CommandIDs::play11:
        soundPlayers[0]->playPlayer(10);
        break;
    case CommandIDs::play12:
        soundPlayers[0]->playPlayer(11);
        break;
    case CommandIDs::save:
        savePlaylist();
        break;
    case CommandIDs::open:
        loadPlaylist();
        break;
    case CommandIDs::quit:
        juce::FileLogger::getCurrentLogger()->writeToLog("quit");
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
        break;
    case CommandIDs::openSettings:
        settingsButtonClicked();
        break;
    case CommandIDs::audioSettings:
        audioSettingsButtonClicked();
        break;
    case CommandIDs::keyboardMapping:
        keyMapperButtonClicked();
        break;
    case CommandIDs::midiMapping:
        midiMapperButtonClicked();
        break;
    case CommandIDs::launchPlaylist:
        if (soundPlayers[0]->getSoundPlayerMode() != SoundPlayer::Mode::OnePlaylistOneCart)
        {
            launchSoundPlayer(SoundPlayer::Mode::OnePlaylistOneCart);
            settings.setPreferedSoundPlayerMode(1);
            grabKeyboardFocus();
        }
        break;
    case CommandIDs::launch8Faders:
        if (soundPlayers[0]->getSoundPlayerMode() != SoundPlayer::Mode::EightFaders)
        {
            launchSoundPlayer(SoundPlayer::Mode::EightFaders);
            settings.setPreferedSoundPlayerMode(2);
            grabKeyboardFocus();
        }
        break;
    case CommandIDs::launchKeyMapped:
        if (soundPlayers[0]->getSoundPlayerMode() != SoundPlayer::Mode::KeyMap)
        {
            launchSoundPlayer(SoundPlayer::Mode::KeyMap);
            settings.setPreferedSoundPlayerMode(3);
            grabKeyboardFocus();
        }
        break;
    case CommandIDs::setInMark:
        bottomComponent.setOrDeleteStart(true);
        break;
    case CommandIDs::deleteInMark:
        bottomComponent.setOrDeleteStart(false);
        break;
    case CommandIDs::setOutMark:
        bottomComponent.setOrDeleteStop(true);
        break;
    case CommandIDs::deleteOutMark:
        bottomComponent.setOrDeleteStop(false);
        break;
    case CommandIDs::showIndividualMeters:
        settings.setShowMeters(!Settings::showMeter);
        break;
    case CommandIDs::showEnveloppe:
        settings.setShowEnveloppe(!Settings::showEnveloppe);
        if (soundPlayers[0]->myPlaylists[0] != nullptr)
            soundPlayers[0]->myPlaylists[0]->rearrangePlayers();
        if (soundPlayers[0]->myPlaylists[1] != nullptr)
            soundPlayers[0]->myPlaylists[1]->rearrangePlayers();
        break;
    case CommandIDs::viewLastPlayedSound:
        settings.setViewLastPlayed(!Settings::viewLastPlayedSound);
        break;
    default:
        return false;
    }
    return true;
}

void MainComponent::focusLost(juce::Component::FocusChangeType cause)
{
    //grabKeyboardFocus();
}

void MainComponent::globalFocusChanged(juce::Component* focusedComponent)
{

}