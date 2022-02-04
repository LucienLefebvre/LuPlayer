#include "MainComponent.h"
#define JUCE_ASIO 1
//#define INFO_BUFFER_SIZE 32767
//==============================================================================
//Channel shift ne shift pas le trim
bool MainComponent::exitAnswered;
MainComponent::MainComponent() : juce::AudioAppComponent(deviceManager),
                                            audioSetupComp(deviceManager, 0, 2, 0, 4, true, false, true, true),
                                 audioSetupWindow("Audiosetup",
                                     getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), true, true),
                                 settingsWindow("Settings",
                                     getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), true, true),
                                 settingsFile(options)
{
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

    juce::MultiTimer::startTimer(0, 500);
    juce::MultiTimer::startTimer(1, 5000);

    tryPreferedAudioDevice(2);
    deviceManager.initialise(2, 2, nullptr, true);

    if (deviceManager.getCurrentAudioDevice() != nullptr)
    {
        settings = std::make_unique<Settings>();
        Settings::sampleRate = deviceManager.getAudioDeviceSetup().sampleRate;
    } 


    menuBar.reset(new juce::MenuBarComponent(this));
    addAndMakeVisible(menuBar.get());
    menuBar->setWantsKeyboardFocus(false);

    keyMapper.reset(new KeyMapper(settings.get()));
    keyMapper->setSize(600, 400);

    midiMapper.reset(new MidiMapper());
    midiMapper->setSize(600, 400);
    //keyMapper->loadKeyMapping();

    initializeBottomComponent();

    audioSetupComp.setColour(juce::ResizableWindow::ColourIds::backgroundColourId, juce::Colours::white);
    audioSetupWindow.setSize(400, 800);
    audioSetupComp.setSize(400, 800);

    addChildComponent(mainStopWatch);
    
    //addAndMakeVisible(timeLabel);
    //timeLabel.setBounds(getParentWidth()-200, 0, 200, topButtonsHeight);
    //timeLabel.setFont(juce::Font(50.00f, juce::Font::plain).withTypefaceStyle("Regular"));

    deviceManager.addChangeListener(this);
    Settings::audioOutputModeValue.addListener(this);

    timerSlider.setRange(10, 1000);
    timerSlider.addListener(this);

    midiInitialized = false;
    midiInitialize();

    myLayout.setItemLayout(0, 200, 1080, -0.70);
    myLayout.setItemLayout(1, 4, 4, 4);
    myLayout.setItemLayout(2, 5, 600, 420);

    horizontalDividerBar.reset(new juce::StretchableLayoutResizerBar(&myLayout, 1, false));
    addAndMakeVisible(horizontalDividerBar.get());

    addKeyListener(this);
    start1 = juce::Time::currentTimeMillis();

    juce::File convFile(Settings::convertedSoundsPath.toStdString());
    if (!convFile.isDirectory())
    {
        std::unique_ptr<juce::AlertWindow> noConvertSoundsWindow;
        noConvertSoundsWindow->showMessageBox(juce::AlertWindow::AlertIconType::InfoIcon, "", "Select Converted Sounds Folder");
        settingsButtonClicked();
    }

    SoundPlayer::Mode mode;
    int p = settings->getPreferedSoundPlayerMode();
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
    }

    launchSoundPlayer(mode);
    setApplicationCommandManagerToWatch(&commandManager);
    commandManager.registerAllCommandsForTarget(this);
    commandManager.registerAllCommandsForTarget(soundPlayers[0]);
    commandManager.getKeyMappings()->resetToDefaultMappings();
    getTopLevelComponent()->addKeyListener(commandManager.getKeyMappings());
    addKeyListener(commandManager.getKeyMappings());

    setWantsKeyboardFocus(true);

    keyMapper->setCommandManager(&commandManager);
    keyMapper->loadMappingFile();
    midiMapper->setCommandManager(&commandManager);
    midiMapper->loadMappingFile();
    setMouseClickGrabsKeyboardFocus(true);
}

void MainComponent::settingsButtonClicked()
{  
        Settings* settings = new Settings();
        settings->saveButton.addListener(this);
        juce::DialogWindow::LaunchOptions o;
        o.content.setOwned(settings);
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
    audioSetupWindow.showDialog("Audio Setup", &audioSetupComp, this,
        getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), true, true, false);
}

void MainComponent::keyMapperButtonClicked()
{
    std::unique_ptr<juce::DialogWindow> keyMapperWindow = std::make_unique<juce::DialogWindow>("Key Mapping",
        getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), true, true);
    keyMapperWindow->setSize(600, 400);
    keyMapperWindow->showDialog("Key Mapping", keyMapper.get(), this,
        getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), true, false, false);
    keyMapper->setWantsKeyboardFocus(true);
}

void MainComponent::midiMapperButtonClicked()
{
    std::unique_ptr<juce::DialogWindow> keyMapperWindow = std::make_unique<juce::DialogWindow>("Midi Mapping",
        getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), true, true);
    keyMapperWindow->setSize(600, 400);
    keyMapperWindow->showDialog("Midi Mapping", midiMapper.get(), this,
        getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), true, false, false);
    midiMapper->setWantsKeyboardFocus(true);
}

void MainComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
{

    if (source == &bottomComponent.getTabbedButtonBar())
    {   //Update red colour on fx button on player if the clipeffect tab is selected
        if (bottomComponent.getTabbedButtonBar().getCurrentTabIndex() != 5)
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

        int cartPosition = soundPlayers[0]->playlistbisViewport.getPosition().getX();
        int playlistScrollPosition = soundPlayers[0]->playlistViewport.getViewPositionY();
        int cartScrollPoisiton = soundPlayers[0]->playlistbisViewport.getViewPositionY();
        std::unique_ptr<juce::StringArray> null = std::make_unique<juce::StringArray >();

        if (getMouseXYRelative().getX() < (soundPlayers[0]->myPlaylists[0]->getPosition().getX() + soundPlayers[0]->myPlaylists[0]->getWidth()))
            soundPlayers[0]->myPlaylists[0]->fileDragMove(*null, getMouseXYRelative().getX(), getMouseXYRelative().getY() - 60 + playlistScrollPosition);
        else if (getMouseXYRelative().getX() > cartPosition)
            soundPlayers[0]->myPlaylists[1]->fileDragMove(*null, getMouseXYRelative().getX() - cartPosition, getMouseXYRelative().getY() - 60 + cartScrollPoisiton);
        else
        {
            soundPlayers[0]->myPlaylists[0]->fileDragExit(*null);
            soundPlayers[0]->myPlaylists[1]->fileDragExit(*null);
        }


    }
    else if (source == bottomComponent.audioPlaybackDemo.fileDroppedFromBrowser)
    {
        //std::unique_ptr<Settings> settings = std::make_unique<Settings>();
        int cartPosition = soundPlayers[0]->playlistbisViewport.getPosition().getX();
        int playlistScrollPosition = soundPlayers[0]->playlistViewport.getViewPositionY();
        int cartScrollPoisiton = soundPlayers[0]->playlistbisViewport.getViewPositionY();
        std::unique_ptr<juce::StringArray> null = std::make_unique<juce::StringArray >();
        std::unique_ptr<juce::File> myFile = std::make_unique<juce::File>(bottomComponent.audioPlaybackDemo.fileBrowser->getSelectedFile(0));
        if (getMouseXYRelative().getX() < (soundPlayers[0]->myPlaylists[0]->getPosition().getX() + soundPlayers[0]->myPlaylists[0]->getWidth()))
            soundPlayers[0]->myPlaylists[0]->filesDropped(myFile->getFullPathName(), getMouseXYRelative().getX(), getMouseXYRelative().getY() - 60 + playlistScrollPosition);
        else if (getMouseXYRelative().getX() > cartPosition)
            soundPlayers[0]->myPlaylists[1]->filesDropped(myFile->getFullPathName(), getMouseXYRelative().getX() - cartPosition, getMouseXYRelative().getY() - 60 + cartScrollPoisiton);
        soundPlayers[0]->myPlaylists[0]->fileDragExit(*null);
        soundPlayers[0]->myPlaylists[1]->fileDragExit(*null);
    }
    else if (source == bottomComponent.dbBrowser.fileDraggedFromDataBase)
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
                soundPlayers[0]->myPlaylists[0]->fileDragMove(*null, getMouseXYRelative().getX(), getMouseXYRelative().getY() - 60 + playlistScrollPosition);
            else if (getMouseXYRelative().getX() > cartPosition)
                soundPlayers[0]->myPlaylists[1]->fileDragMove(*null, getMouseXYRelative().getX() - cartPosition, getMouseXYRelative().getY() - 60 + cartScrollPoisiton);
            else
            {
                soundPlayers[0]->myPlaylists[0]->fileDragExit(*null);
                soundPlayers[0]->myPlaylists[1]->fileDragExit(*null);
            }
        }

    }
    else if (source == bottomComponent.dbBrowser.fileDroppedFromDataBase)
    {
        std::unique_ptr<juce::StringArray> null = std::make_unique<juce::StringArray >();


        juce::String selectedSoundName = bottomComponent.dbBrowser.getSelectedSoundName();
        juce::String fullPathName = bottomComponent.dbBrowser.getSelectedFile().getFullPathName();

        if (soundPlayers[0]->soundPlayerMode == SoundPlayer::Mode::KeyMap && soundPlayers[0]->keyMappedSoundboard != nullptr)
        {
            soundPlayers[0]->keyMappedSoundboard->setDroppedFile(getMouseXYRelative(), fullPathName, selectedSoundName);
            soundPlayers[0]->keyMappedSoundboard->fileDragExit();
        }
        else
        {
            int cartPosition = soundPlayers[0]->playlistbisViewport.getPosition().getX();
            int playlistScrollPosition = soundPlayers[0]->playlistViewport.getViewPositionY();
            int cartScrollPoisiton = soundPlayers[0]->playlistbisViewport.getViewPositionY();

            if (getMouseXYRelative().getX() < (soundPlayers[0]->myPlaylists[0]->getPosition().getX() + soundPlayers[0]->myPlaylists[0]->getWidth()))
            {
                soundPlayers[0]->myPlaylists[0]->setDroppedSoundName(selectedSoundName);
                soundPlayers[0]->myPlaylists[0]->filesDropped(fullPathName, getMouseXYRelative().getX(), getMouseXYRelative().getY() - 60 + playlistScrollPosition);

            }
            else if (getMouseXYRelative().getX() > cartPosition)
            {
                soundPlayers[0]->myPlaylists[1]->setDroppedSoundName(selectedSoundName);
                soundPlayers[0]->myPlaylists[1]->filesDropped(fullPathName, getMouseXYRelative().getX() - cartPosition, getMouseXYRelative().getY() - 60 + cartScrollPoisiton);
            }
            soundPlayers[0]->myPlaylists[0]->fileDragExit(*null);
            soundPlayers[0]->myPlaylists[1]->fileDragExit(*null);
        }

    }
    else if (source == bottomComponent.distantDbBrowser.fileDraggedFromDataBase)
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
                soundPlayers[0]->myPlaylists[0]->fileDragMove(*null, getMouseXYRelative().getX(), getMouseXYRelative().getY() - 60 + playlistScrollPosition);
            else if (getMouseXYRelative().getX() > cartPosition)
                soundPlayers[0]->myPlaylists[1]->fileDragMove(*null, getMouseXYRelative().getX() - cartPosition, getMouseXYRelative().getY() - 60 + cartScrollPoisiton);
            else
            {
                soundPlayers[0]->myPlaylists[0]->fileDragExit(*null);
                soundPlayers[0]->myPlaylists[1]->fileDragExit(*null);
            }
        }

    }
    else if (source == bottomComponent.distantDbBrowser.fileDroppedFromDataBase)
    {
        juce::String selectedSoundName = bottomComponent.distantDbBrowser.getSelectedSoundName();
        juce::String fullPathName = bottomComponent.distantDbBrowser.getSelectedFile().getFullPathName();

        std::unique_ptr<juce::StringArray> null = std::make_unique<juce::StringArray >();

        if (soundPlayers[0]->soundPlayerMode == SoundPlayer::Mode::KeyMap && soundPlayers[0]->keyMappedSoundboard != nullptr)
        {
            soundPlayers[0]->keyMappedSoundboard->setDroppedFile(getMouseXYRelative(), fullPathName, selectedSoundName);
            soundPlayers[0]->keyMappedSoundboard->fileDragExit();
        }
        else
        {
            int cartPosition = soundPlayers[0]->playlistbisViewport.getPosition().getX();
            int playlistScrollPosition = soundPlayers[0]->playlistViewport.getViewPositionY();
            int cartScrollPoisiton = soundPlayers[0]->playlistbisViewport.getViewPositionY();

            if (getMouseXYRelative().getX() < (soundPlayers[0]->myPlaylists[0]->getPosition().getX() + soundPlayers[0]->myPlaylists[0]->getWidth()))
            {
                soundPlayers[0]->myPlaylists[0]->setDroppedSoundName(selectedSoundName);
                soundPlayers[0]->myPlaylists[0]->filesDropped(fullPathName, getMouseXYRelative().getX(), getMouseXYRelative().getY() - 60 + playlistScrollPosition);

            }
            else if (getMouseXYRelative().getX() > cartPosition)
            {
                soundPlayers[0]->myPlaylists[1]->setDroppedSoundName(selectedSoundName);
                soundPlayers[0]->myPlaylists[1]->filesDropped(fullPathName, getMouseXYRelative().getX() - cartPosition, getMouseXYRelative().getY() - 60 + cartScrollPoisiton);
            }
            soundPlayers[0]->myPlaylists[0]->fileDragExit(*null);
            soundPlayers[0]->myPlaylists[1]->fileDragExit(*null);
        }
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
        bottomComponent.setCurrentTabIndex(6);
    }
    else if (source == soundPlayers[0]->myPlaylists[0]->envButtonBroadcaster || source == soundPlayers[0]->myPlaylists[1]->envButtonBroadcaster)
    {
        auto* player = soundPlayers[0]->myPlaylists[Settings::editedPlaylist]->players[Settings::editedPlayer];
        bottomComponent.clipEditor.setPlayer(player);
        bottomComponent.clipEffect.setPlayer(player);
        bottomComponent.setCurrentTabIndex(5);
    }
    else if (source == soundPlayers[0]->playlistLoadedBroadcaster)
    {
        juce::Time::waitForMillisecondCounter(juce::Time::getMillisecondCounter() + 1000);
        bottomComponent.clipEffect.setNullPlayer();
        bottomComponent.clipEditor.setNullPlayer();
    }
}

MainComponent::~MainComponent()
{
    removeMouseListener(this);
    myMixer.removeAllInputs();
    myCueMixer.removeAllInputs();
    soundPlayers[0]->playerSelectionChanged->removeChangeListener(this);
    deviceManager.removeChangeListener(this);
    bottomComponent.recorderComponent.mouseDragInRecorder->removeChangeListener(this);
    Settings::audioOutputModeValue.removeListener(this);
    shutdownAudio();
}

void MainComponent::exitRequested()
{

}
void MainComponent::deleteConvertedFiles() //this delete the converted files since the opening of the application
{
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
            juce::AudioBuffer<float>* inputBuffer = new juce::AudioBuffer<float>(2, bufferToFill.buffer->getNumSamples());
            inputBuffer->copyFrom(0, 0, *bufferToFill.buffer, 0, 0, bufferToFill.buffer->getNumSamples());
            inputBuffer->copyFrom(1, 0, *bufferToFill.buffer, 1, 0, bufferToFill.buffer->getNumSamples());
            bufferToFill.clearActiveBufferRegion();
            juce::AudioBuffer<float>* outputBuffer = new juce::AudioBuffer<float>(2, bufferToFill.buffer->getNumSamples());
            juce::AudioSourceChannelInfo* playAudioSource = new juce::AudioSourceChannelInfo(*outputBuffer);
            juce::AudioSourceChannelInfo* cueAudioSource = new juce::AudioSourceChannelInfo(*outputBuffer);

            myCueMixer.getNextAudioBlock(*cueAudioSource);
            bufferToFill.buffer->copyFrom(2, 0, *outputBuffer, 0, 0, bufferToFill.buffer->getNumSamples());
            bufferToFill.buffer->copyFrom(3, 0, *outputBuffer, 1, 0, bufferToFill.buffer->getNumSamples());
            soundPlayers[0]->cueloudnessMeter.processBlock(*outputBuffer);
            soundPlayers[0]->cuemeterSource.measureBlock(*outputBuffer);
            myMixer.getNextAudioBlock(*playAudioSource);
            if (bottomComponent.recorderComponent.isEnabled())
            {
                juce::AudioBuffer<float>* newOutputBuffer = new juce::AudioBuffer<float>(2, bufferToFill.buffer->getNumSamples());
                bottomComponent.recorderComponent.recordAudioBuffer(outputBuffer, inputBuffer, newOutputBuffer, 2, actualSampleRate, bufferToFill.buffer->getNumSamples());

                //bufferToFill.clearActiveBufferRegion();
                bufferToFill.buffer->copyFrom(0, 0, *outputBuffer, 0, 0, bufferToFill.buffer->getNumSamples());
                bufferToFill.buffer->copyFrom(1, 0, *outputBuffer, 1, 0, bufferToFill.buffer->getNumSamples());
                soundPlayers[0]->loudnessMeter.processBlock(*outputBuffer);
                soundPlayers[0]->meterSource.measureBlock(*outputBuffer);
                if (bottomComponent.recorderComponent.enableMonitoring.getToggleState())
                {
                    bufferToFill.buffer->copyFrom(2, 0, *newOutputBuffer, 0, 0, bufferToFill.buffer->getNumSamples());
                    bufferToFill.buffer->copyFrom(3, 0, *newOutputBuffer, 1, 0, bufferToFill.buffer->getNumSamples());
                }

                delete newOutputBuffer;
            }
            else
            {
                bufferToFill.buffer->copyFrom(0, 0, *outputBuffer, 0, 0, bufferToFill.buffer->getNumSamples());
                bufferToFill.buffer->copyFrom(1, 0, *outputBuffer, 1, 0, bufferToFill.buffer->getNumSamples());
                //soundPlayers[0]->loudnessMeter.processBlock(*outputBuffer);
                //soundPlayers[0]->meterSource.measureBlock(*outputBuffer);
            }
            delete(inputBuffer);
            delete(outputBuffer);
            delete(playAudioSource);
        }
        else if (Settings::audioOutputMode == 1)
        {
            juce::AudioBuffer<float>* inputBuffer = new juce::AudioBuffer<float>(2, bufferToFill.buffer->getNumSamples());
            inputBuffer->copyFrom(0, 0, *bufferToFill.buffer, 0, 0, bufferToFill.buffer->getNumSamples());
            juce::AudioBuffer<float>* outputBuffer = new juce::AudioBuffer<float>(2, bufferToFill.buffer->getNumSamples());
            juce::AudioSourceChannelInfo* playAudioSource = new juce::AudioSourceChannelInfo(*outputBuffer);
            myMixer.getNextAudioBlock(*playAudioSource);


            bufferToFill.buffer->copyFrom(0, 0, *outputBuffer, 0, 0, bufferToFill.buffer->getNumSamples());
            if (!bottomComponent.recorderComponent.isEnabled())
            {
                soundPlayers[0]->loudnessMeter.processBlock(*outputBuffer);
                soundPlayers[0]->meterSource.measureBlock(*bufferToFill.buffer);
            }

            if (bottomComponent.recorderComponent.isEnabled())
            {
                juce::AudioBuffer<float>* newOutputBuffer = new juce::AudioBuffer<float>(2, bufferToFill.buffer->getNumSamples());
                bottomComponent.recorderComponent.recordAudioBuffer(outputBuffer, inputBuffer, newOutputBuffer, 2, actualSampleRate, bufferToFill.buffer->getNumSamples());
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
                delete newOutputBuffer;
            }
            else
            {
                myCueMixer.getNextAudioBlock(*playAudioSource);
                soundPlayers[0]->cueloudnessMeter.processBlock(*outputBuffer);
                bufferToFill.buffer->copyFrom(1, 0, *outputBuffer, 0, 0, bufferToFill.buffer->getNumSamples());
                soundPlayers[0]->cuemeterSource.measureBlock(*bufferToFill.buffer);
            }
            delete(outputBuffer);
            delete(inputBuffer);
            delete(playAudioSource);
        }
        else if (Settings::audioOutputMode == 2 || Settings::audioOutputMode == 3)
        {
            juce::AudioBuffer<float>* inputBuffer = new juce::AudioBuffer<float>(2, bufferToFill.buffer->getNumSamples());
            inputBuffer->copyFrom(0, 0, *bufferToFill.buffer, 0, 0, bufferToFill.buffer->getNumSamples());
            inputBuffer->copyFrom(1, 0, *bufferToFill.buffer, 1, 0, bufferToFill.buffer->getNumSamples());

            //MIXER
            if (showMixer)
                mixer.getNextAudioBlock(bufferToFill.buffer, mixerOutputBuffer.get());
            //

            bufferToFill.clearActiveBufferRegion();
            juce::AudioBuffer<float>* outputBuffer = new juce::AudioBuffer<float>(2, bufferToFill.buffer->getNumSamples());
            juce::AudioSourceChannelInfo* playAudioSource = new juce::AudioSourceChannelInfo(*outputBuffer);
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
                    soundPlayers[0]->newMeter->measureBlock(outputBuffer);
            }
            if (bottomComponent.recorderComponent.isEnabled() && soundPlayers[0] != nullptr)
            {
                juce::AudioBuffer<float>* newOutputBuffer = new juce::AudioBuffer<float>(2, bufferToFill.buffer->getNumSamples());
                bottomComponent.recorderComponent.recordAudioBuffer(outputBuffer, inputBuffer, newOutputBuffer, 2, actualSampleRate, bufferToFill.buffer->getNumSamples());
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
                delete newOutputBuffer;
            }
            //delete(mixerOutputBuffer);
            delete(inputBuffer);
            delete(outputBuffer);
            delete(playAudioSource);
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
    mainStopWatch.setSize(120, topButtonsHeight);
    mainStopWatch.setCentrePosition(getWidth() / 2, topButtonsHeight / 2 + 1);

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
    //grabKeyboardFocus();
}


void MainComponent::timerCallback(int timerID)
{
    if (timerID == 0)
    {
        /*juce::Time* time = new juce::Time(time->getCurrentTime());
        timeLabel.setText(time->toString(false, true, true, true), juce::NotificationType::dontSendNotification);
        timeLabel.toFront(false);*/
    }
    else if (timerID == 1)
    {
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
    if (midiMapper->getWantsKeyPress())
    {
        midiMapper->handleMidiMessage(message);
    }
    else
    {
        for (int i = 0; i < commandManager.getNumCommands(); i++)
        {
            juce::CommandID cID = commandManager.getCommandForIndex(i)->commandID;
            const juce::MessageManagerLock mmLock;
            if (message.getControllerNumber() == midiMapper->getMidiCCForCommand(cID) && message.getControllerValue() == 127)
                invokeDirectly(cID, true);
        }
        auto mode = soundPlayers[0]->getSoundPlayerMode();
        if (mode == SoundPlayer::Mode::OnePlaylistOneCart)
            soundPlayers[0]->handleIncomingMidiMessage(source, message);
        else if (mode == SoundPlayer::Mode::EightFaders)
            soundPlayers[0]->handleIncomingMidiMessageEightPlayers(source, message);
        //if (message.getControllerNumber() == midiMapper->getMidiCCForCommand(MidiMapper::MidiCommands::LaunchRecord) && message.getControllerValue() == 127)
        //{
        //    launchRecord();
        //}
        //else if (message.getControllerNumber() == midiMapper->getMidiCCForCommand(CommandIDs::goToClipEditor) && message.getControllerValue() == 127)
        //{
        //    //invokeDirectly(CommandIDs::goToClipEditor, true);
        //}
        //else if (message.getControllerNumber() == 61 && message.getControllerValue() == 127)
        //{
        //    const juce::MessageManagerLock mmLock;
        //    bottomComponent.setStart();
        //}
        //else if (message.getControllerNumber() == 62 && message.getControllerValue() == 127)
        //{
        //    const juce::MessageManagerLock mmLock;
        //    bottomComponent.setStop();
        //}
        //else if (message.getControllerNumber() == 60 && message.getControllerValue() == 127)
        //{
        //    const juce::MessageManagerLock mmLock;
        //    bottomComponent.clipEditor.launchCue();
        //}
        //else if (message.getControllerNumber() == 46 && message.getControllerValue() == 127)
        //{
        //    const juce::MessageManagerLock mmLock;
        //    stopWatchShortcuPressed();
        //}
    }
}





bool MainComponent::keyPressed(const juce::KeyPress &key, juce::Component* originatingComponent)
{
    grabKeyboardFocus();
    //int keyCode = key.getKeyCode();
    //if (keyMapper->getKeyMapping(7) == keyCode)
    //{
    //    commandManager.invokeDirectly(CommandIDs::startTimer, false);
    //    //commandManager.invokeDirectly(SoundPlayer::CommandIDs::setInMark, true);
    //}
    //
    //if (key.getModifiers() == juce::ModifierKeys::commandModifier)
    //{
    //    if (keyMapper->getKeyMapping(25) == keyCode)
    //        savePlaylist();
    //    else if (keyMapper->getKeyMapping(26) == keyCode)
    //        loadPlaylist();
    //    else if (keyMapper->getKeyMapping(27) == keyCode)
    //        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    //    else if (keyMapper->getKeyMapping(28) == keyCode)
    //        settingsButtonClicked();
    //    else if (keyMapper->getKeyMapping(29) == keyCode)
    //        audioSettingsButtonClicked();
    //    else if (keyMapper->getKeyMapping(30) == keyCode)
    //        keyMapperButtonClicked();
    //    else if (keyMapper->getKeyMapping(31) == keyCode)
    //        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    //}
    //else
    //{
    //    if (soundPlayers[0]->getSoundPlayerMode() == SoundPlayer::Mode::KeyMap)//Block all shortcuts for key mapped soundboard
    //    {
    //        soundPlayers[0]->keyMappedSoundboard->keyPressed(key, originatingComponent);
    //    }
    //    else if (keyMapper->getKeyMapping(12) == keyCode)
    //    {
    //        stopWatchShortcuPressed();
    //    }
    //    else
    //    {
    //        if (soundPlayers[0]->draggedPlaylist != -1)
    //            soundPlayers[0]->keyPressed(key, originatingComponent, keyMapper.get());
    //        else if (soundPlayers[0]->draggedPlaylist == -1)
    //            bottomComponent.recorderComponent.keyPressed(key, keyMapper.get());
    //    }

    //    if (keyMapper->getKeyMapping(0) == keyCode)
    //    {
    //        if (soundPlayers[0]->soundPlayerMode == SoundPlayer::Mode::OnePlaylistOneCart)
    //            soundPlayers[0]->myPlaylists[0]->spaceBarPressed();
    //        else
    //        {
    //            bottomComponent.keyPressed(key, originatingComponent, keyMapper.get());
    //        }
    //    }
    //    else if (keyMapper->getKeyMapping(13) == keyCode)
    //    {
    //        if (!isEightPlayerMode)
    //            soundPlayers[0]->myPlaylists[1]->playPlayer(0);
    //        else
    //            soundPlayers[0]->myPlaylists[0]->playPlayer(0);
    //        soundPlayers[0]->positionViewport(0);
    //    }
    //    else if (keyMapper->getKeyMapping(14) == keyCode)
    //    {
    //        if (!isEightPlayerMode)
    //            soundPlayers[0]->myPlaylists[1]->playPlayer(1);
    //        else
    //            soundPlayers[0]->myPlaylists[0]->playPlayer(1);
    //        soundPlayers[0]->positionViewport(1);
    //    }
    //    else if (keyMapper->getKeyMapping(15) == keyCode)
    //    {
    //        if (!isEightPlayerMode)
    //            soundPlayers[0]->myPlaylists[1]->playPlayer(2);
    //        else
    //            soundPlayers[0]->myPlaylists[0]->playPlayer(2);
    //        soundPlayers[0]->positionViewport(2);
    //    }
    //    else if (keyMapper->getKeyMapping(16) == keyCode)
    //    {
    //        if (!isEightPlayerMode)
    //            soundPlayers[0]->myPlaylists[1]->playPlayer(3);
    //        else
    //            soundPlayers[0]->myPlaylists[0]->playPlayer(3);
    //        soundPlayers[0]->positionViewport(3);
    //    }
    //    else if (keyMapper->getKeyMapping(17) == keyCode)
    //    {
    //        if (!isEightPlayerMode)
    //            soundPlayers[0]->myPlaylists[1]->playPlayer(4);
    //        else
    //            soundPlayers[0]->myPlaylists[1]->playPlayer(0);
    //        soundPlayers[0]->positionViewport(4);
    //    }
    //    else if (keyMapper->getKeyMapping(18) == keyCode)
    //    {
    //        if (!isEightPlayerMode)
    //            soundPlayers[0]->myPlaylists[1]->playPlayer(5);
    //        else
    //            soundPlayers[0]->myPlaylists[1]->playPlayer(1);
    //        soundPlayers[0]->positionViewport(5);
    //    }
    //    else if (keyMapper->getKeyMapping(19) == keyCode)
    //    {
    //        if (!isEightPlayerMode)
    //            soundPlayers[0]->myPlaylists[1]->playPlayer(6);
    //        else
    //            soundPlayers[0]->myPlaylists[1]->playPlayer(2);
    //        soundPlayers[0]->positionViewport(6);
    //    }
    //    else if (keyMapper->getKeyMapping(20) == keyCode)
    //    {
    //        if (!isEightPlayerMode)
    //            soundPlayers[0]->myPlaylists[1]->playPlayer(7);
    //        else
    //            soundPlayers[0]->myPlaylists[1]->playPlayer(3);
    //        soundPlayers[0]->positionViewport(7);
    //    }
    //    else if (keyMapper->getKeyMapping(21) == keyCode)
    //    {
    //        if (!isEightPlayerMode)
    //            soundPlayers[0]->myPlaylists[1]->playPlayer(8);
    //        soundPlayers[0]->positionViewport(8);
    //    }
    //    else if (keyMapper->getKeyMapping(22) == keyCode)
    //    {
    //        if (!isEightPlayerMode)
    //            soundPlayers[0]->myPlaylists[1]->playPlayer(9);
    //        soundPlayers[0]->positionViewport(9);
    //    }
    //    else if (keyMapper->getKeyMapping(23) == keyCode)
    //    {
    //        if (!isEightPlayerMode)
    //            soundPlayers[0]->myPlaylists[1]->playPlayer(10);
    //        soundPlayers[0]->positionViewport(10);
    //    }
    //    else if (keyMapper->getKeyMapping(24) == keyCode)
    //    {
    //        if (!isEightPlayerMode)
    //            soundPlayers[0]->myPlaylists[1]->playPlayer(11);
    //        soundPlayers[0]->positionViewport(11);
    //    }
    //    else if (keyMapper->getKeyMapping(1) == keyCode)
    //    {
    //        if (soundPlayers[0]->myPlaylists[0]->spaceBarIsPlaying == false)
    //            soundPlayers[0]->myPlaylists[0]->playersResetPositionClicked();
    //    }
    //    else if (keyMapper->getKeyMapping(2) == keyCode)
    //    {
    //        if (!isEightPlayerMode)
    //            soundPlayers[0]->myPlaylists[0]->playersNextPositionClicked();
    //    }
    //    else if (keyMapper->getKeyMapping(3) == keyCode)
    //    {
    //        if (!isEightPlayerMode)
    //            soundPlayers[0]->myPlaylists[0]->playersPreviousPositionClicked();
    //    }
    //    else if (keyMapper->getKeyMapping(11) == keyCode)
    //    {
    //        if (soundPlayers[0]->getSoundPlayerMode() != SoundPlayer::Mode::KeyMap)
    //            launchRecord();
    //    }
    //}
    //
    return false;
}

void MainComponent::savePlaylist()
{
    soundPlayers[0]->savePlaylist();
    saved = true;
}

bool MainComponent::hasBeenSaved()
{
    return saved;
}

void MainComponent::loadPlaylist()
{
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
}


void MainComponent::OSCInitialize()
{
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
        tryPreferedAudioDevice(2);
        setAudioChannels(2, 2);

        myMixer.addInputSource(&soundPlayers[0]->myMixer, false);
        myCueMixer.addInputSource(&soundPlayers[0]->myCueMixer, false);
        myCueMixer.addInputSource(&bottomComponent.myMixer, false);

    }
    else if (Settings::audioOutputMode == 2)
    {

        tryPreferedAudioDevice(2);
        setAudioChannels(2, 2);
        myMixer.addInputSource(&soundPlayers[0]->myMixer, false);
        myMixer.addInputSource(&soundPlayers[0]->myCueMixer, false);
        myMixer.addInputSource(&bottomComponent.myMixer, false);

    }
    else if (Settings::audioOutputMode == 3)
    {
        tryPreferedAudioDevice(4);
        setAudioChannels(2, 4);
        myMixer.addInputSource(&soundPlayers[0]->myMixer, false);
        myCueMixer.addInputSource(&bottomComponent.myMixer, false);
        myCueMixer.addInputSource(&soundPlayers[0]->myCueMixer, false);
    }
}


void MainComponent::tryPreferedAudioDevice(int outputChannelsNeeded)
{
    if (deviceManager.getCurrentAudioDevice() != nullptr)
    {
        std::unique_ptr<Settings> settings = std::make_unique<Settings>();
        const juce::OwnedArray<juce::AudioIODeviceType>& types = deviceManager.getAvailableDeviceTypes();
        if (types.size() > 1)
        {
            for (int i = 0; i < types.size(); ++i)
            {
                if (types[i]->getTypeName() == Settings::preferedAudioDeviceType)
                {
                    deviceManager.setCurrentAudioDeviceType(Settings::preferedAudioDeviceType, true);
                    deviceManager.initialise(2, outputChannelsNeeded, nullptr, true, Settings::preferedAudioDeviceName, &settings->getPreferedAudioDevice());
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
    bottomComponent.setCurrentTabIndex(4);
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
    myMixer.addInputSource(&soundPlayers[0]->myMixer, false);
    myCueMixer.addInputSource(&soundPlayers[0]->myCueMixer, false);

    if (m == SoundPlayer::Mode::EightFaders)
    {
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
        for (auto i = -1; i < 1; ++i)
        {
            soundPlayers[0]->myPlaylists[0]->addPlayer(i);
            soundPlayers[0]->myPlaylists[1]->addPlayer(i);
        }
        soundPlayers[0]->myPlaylists[1]->assignLeftFader(-1);
        soundPlayers[0]->myPlaylists[1]->assignLeftFader(0);
        soundPlayers[0]->myPlaylists[1]->assignRightFader(-1);
        soundPlayers[0]->myPlaylists[1]->assignRightFader(1);
        soundPlayers[0]->myPlaylists[0]->setPlaylistPosition(0);
        soundPlayers[0]->myPlaylists[1]->setPlaylistPosition(1);
    }
    if (m == SoundPlayer::Mode::KeyMap)
    {
        soundPlayers[0]->initializeKeyMapPlayer();
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
        settings->setPreferedSoundPlayerMode(r);
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
    mainStopWatch.setVisible(true);
    mainStopWatch.startStopButtonClicked();
}

juce::StringArray MainComponent::getMenuBarNames()
{
    return { "File", "Settings", "Soundplayer Mode", "Help" };
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
        menu.addCommandItem(&commandManager, CommandIDs::launchPlaylist, "One playlist, one cart");
        menu.addCommandItem(&commandManager, CommandIDs::launch8Faders, "Eight faders");
        menu.addCommandItem(&commandManager, CommandIDs::launchKeyMapped, "Keyboard Mapped");
    }
    else if (menuIndex == 3)
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
    bottomComponent.dbBrowser.fileDraggedFromDataBase->addChangeListener(this);
    bottomComponent.dbBrowser.fileDroppedFromDataBase->addChangeListener(this);
    bottomComponent.distantDbBrowser.fileDraggedFromDataBase->addChangeListener(this);
    bottomComponent.distantDbBrowser.fileDroppedFromDataBase->addChangeListener(this);
    bottomComponent.recorderComponent.mouseDragInRecorder->addChangeListener(this);
    bottomComponent.recorderComponent.spaceBarKeyPressed->addChangeListener(this);
    bottomComponent.cuePlay->addChangeListener(this);
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
                                    CommandIDs::lanchRecord,
                                    CommandIDs::goToSoundBrowser,
                                    CommandIDs::goToDataBaseBrowser,
                                    CommandIDs::goToDistantDataBaseBrowser,
                                    CommandIDs::goToDatabaseImport,
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
                                    CommandIDs::launchKeyMapped};
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
    case CommandIDs::lanchRecord:
        result.setInfo("Launch record", "Launch record", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress('r', juce::ModifierKeys::noModifiers);
        break;
    case CommandIDs::goToSoundBrowser:
        result.setInfo("Display sound browser", "Display sound browser", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress('&', juce::ModifierKeys::noModifiers);
        break;
    case CommandIDs::goToDataBaseBrowser:
        result.setInfo("Display Netia database browser", "Display Netia database browser", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress('é', juce::ModifierKeys::noModifiers);
        break;
    case CommandIDs::goToDistantDataBaseBrowser:
        result.setInfo("Display distant database browser", "Display distant database browser", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress('"', juce::ModifierKeys::noModifiers);
        break;
    case CommandIDs::goToDatabaseImport:
        result.setInfo("Display database import", "Display database import", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress('\'', juce::ModifierKeys::noModifiers);
        break;
    case CommandIDs::goToRecorder:
        result.setInfo("Display recorder", "Display recorder", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress('(', juce::ModifierKeys::noModifiers);
        break;
    case CommandIDs::goToClipEditor:
        result.setInfo("Display clip editor", "Display clip editor", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress('-', juce::ModifierKeys::noModifiers);
        break;
    case CommandIDs::goToClipEffect:
        result.setInfo("Display clip effect", "Display clip effect", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress('è', juce::ModifierKeys::noModifiers);
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
    default:
        break;
    }
}

bool MainComponent::perform(const InvocationInfo& info)
{
    switch (info.commandID)
    {
    case CommandIDs::startTimer:
        mainStopWatch.setVisible(true);
        mainStopWatch.startStopButtonClicked();
        break;
    case CommandIDs::lanchRecord:
        launchRecord();
        break;
    case CommandIDs::goToSoundBrowser:
        bottomComponent.getTabbedButtonBar().setCurrentTabIndex(0);
        break;
    case CommandIDs::goToDataBaseBrowser:
        bottomComponent.getTabbedButtonBar().setCurrentTabIndex(1);
        break;
    case CommandIDs::goToDistantDataBaseBrowser:
        bottomComponent.getTabbedButtonBar().setCurrentTabIndex(2);
        break;
    case CommandIDs::goToDatabaseImport:
        bottomComponent.getTabbedButtonBar().setCurrentTabIndex(3);
        break;
    case CommandIDs::goToRecorder:
        bottomComponent.getTabbedButtonBar().setCurrentTabIndex(4);
        break;
    case CommandIDs::goToClipEditor:
        bottomComponent.getTabbedButtonBar().setCurrentTabIndex(5);
        break;
    case CommandIDs::goToClipEffect:
        bottomComponent.getTabbedButtonBar().setCurrentTabIndex(6);
        break;
    case CommandIDs::spaceBarPlay:
        if (soundPlayers[0]->soundPlayerMode == SoundPlayer::Mode::OnePlaylistOneCart)
            soundPlayers[0]->myPlaylists[0]->spaceBarPressed();
        else
        {
            bottomComponent.spaceBarPressed();
        }
        break;
    case CommandIDs::goToPrevious:
        if (soundPlayers[0]->soundPlayerMode == SoundPlayer::Mode::OnePlaylistOneCart)
            soundPlayers[0]->myPlaylists[0]->playersPreviousPositionClicked();
        break;
    case CommandIDs::goToFirst:
        if (soundPlayers[0]->soundPlayerMode == SoundPlayer::Mode::OnePlaylistOneCart)
            soundPlayers[0]->myPlaylists[0]->playersResetPositionClicked();
        break;
    case CommandIDs::goToNext:
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
            settings->setPreferedSoundPlayerMode(1);
        }
        break;
    case CommandIDs::launch8Faders:
        if (soundPlayers[0]->getSoundPlayerMode() != SoundPlayer::Mode::EightFaders)
        {
            launchSoundPlayer(SoundPlayer::Mode::EightFaders);
            settings->setPreferedSoundPlayerMode(2);
        }
        break;
    case CommandIDs::launchKeyMapped:
        if (soundPlayers[0]->getSoundPlayerMode() != SoundPlayer::Mode::KeyMap)
        {
            launchSoundPlayer(SoundPlayer::Mode::KeyMap);
            settings->setPreferedSoundPlayerMode(3);
        }
        break;
    default:
        return false;
    }

    return true;
}
