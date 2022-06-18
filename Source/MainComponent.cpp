/*
  ==============================================================================

    MainComponent.cpp
    Created: 26 Jan 2021 7:36:59pm
    Author:  Lucien Lefebvre

  ==============================================================================
*/
#include "MainComponent.h"

bool MainComponent::exitAnswered;
MainComponent::MainComponent() : juce::AudioAppComponent(deviceManager),
                                 audioSetupComp(deviceManager, 0, 2, 0, 4, true, false, true, true),
                                 audioSetupWindow("Audiosetup",
                                     getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), true, true),
                                 settingsWindow("Settings",
                                     getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), true, true)
{

    logger.reset(juce::FileLogger::createDefaultAppLogger("LuPlayer", "Log.txt", "LuPlayer Log"));
    juce::FileLogger::setCurrentLogger(logger.get());
    juce::FileLogger::getCurrentLogger()->writeToLog("App launch");

    juce::String micAutorisation = juce::WindowsRegistry::getValue("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\CapabilityAccessManager\\ConsentStore\\microphone\\VALUE");

    if (micAutorisation.equalsIgnoreCase("Deny"))
        numInputsChannels = 0;
    else if (micAutorisation.equalsIgnoreCase("Allow"))
        numInputsChannels = 2;

    setAudioChannels(numInputsChannels, 2);
    tryPreferedAudioDevice(2);

    juce::MultiTimer::startTimer(0, 50);
    juce::MultiTimer::startTimer(1, 3000);

    if (deviceManager.getCurrentAudioDevice() != nullptr)
    {
        Settings::sampleRate = deviceManager.getAudioDeviceSetup().sampleRate;
    } 

    setWantsKeyboardFocus(true);

    menuBar.reset(new juce::MenuBarComponent(this));
    addAndMakeVisible(menuBar.get());
    menuBar->setWantsKeyboardFocus(false);

    initializeBottomComponent();

    audioSetupComp.setColour(juce::ResizableWindow::ColourIds::backgroundColourId, juce::Colours::white);
    audioSetupWindow.setSize(400, 800);
    audioSetupComp.setSize(400, 800);
    
    deviceManager.addChangeListener(this);
    Settings::audioOutputModeValue.addListener(this);
    Settings::showMeterValue.addListener(this);
    bottomComponent.textEditor.textEditor.textFocusLostBroadcaster->addChangeListener(this);

    midiInitialize();

    myLayout.setItemLayout(0, 210, 1080, -0.72);
    myLayout.setItemLayout(1, 4, 4, 4);
    myLayout.setItemLayout(2, 5, 610, 420);

    horizontalDividerBar.reset(new juce::StretchableLayoutResizerBar(&myLayout, 1, false));
    addAndMakeVisible(horizontalDividerBar.get());

#if RFBUILD
    juce::File convFile(Settings::convertedSoundsPath.toStdString());
    if (!convFile.isDirectory())
    {
        std::unique_ptr<juce::AlertWindow> noConvertSoundsWindow;
        noConvertSoundsWindow->showMessageBox(juce::AlertWindow::AlertIconType::InfoIcon, "", "Select Converted Sounds Folder");
        settingsButtonClicked();
    }
#endif

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

    midiMapper.setCommandManager(&commandManager);
    midiMapper.loadMappingFile();
    setMouseClickGrabsKeyboardFocus(true);
    addKeyListener(this);
    setWantsKeyboardFocus(true);

    if (Settings::autoCheckNewUpdate)
        checkNewVersion();
}

void MainComponent::settingsButtonClicked()
{  
    juce::FileLogger::getCurrentLogger()->writeToLog("Settings button clicked");

    settings.saveButton.addListener(this);
    juce::DialogWindow::LaunchOptions o;
    o.content.setNonOwned(&settings);
    o.content->setSize(600, 470);
    o.dialogTitle = TRANS("Settings");
    o.dialogBackgroundColour = getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId);
    o.escapeKeyTriggersCloseButton = true;
    o.useNativeTitleBar = false;
    o.resizable = false;
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
    o.dialogTitle = TRANS("Midi Mapper");
    juce::DialogWindow* window = o.launchAsync();
    juce::ModalComponentManager::getInstance()->attachCallback(window, juce::ModalCallbackFunction::create([this](int r)
        {
            midiMapper.setWantsKeyPress(false);
            grabKeyboardFocus();
        }));
}

void MainComponent::signalGeneratorButtonClicked()
{
    juce::DialogWindow::LaunchOptions o;
    o.content.setNonOwned(&signalGenerator);
    o.content->setSize(200, 200);
    o.dialogTitle = TRANS("Signal Generator");
    o.dialogBackgroundColour = getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId);
    o.escapeKeyTriggersCloseButton = true;
    o.useNativeTitleBar = false;
    o.resizable = false;
    juce::DialogWindow* window = o.launchAsync();

    juce::ModalComponentManager::getInstance()->attachCallback(window, juce::ModalCallbackFunction::create([this](int r)
        {
            signalGenerator.setEnabled(false);
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
            settings.setPreferedAudioDeviceType(deviceManager.getCurrentAudioDeviceType());
            settings.setPreferedAudioDeviceName(deviceManager.getCurrentAudioDevice()->getName());
            settings.setPreferedAudioDevice(deviceManager.getAudioDeviceSetup());
            settings.updateSampleRateValue(deviceManager.getAudioDeviceSetup().sampleRate);
            settings.saveOptions();
            Settings::outputChannelsNumber = deviceManager.getCurrentAudioDevice()->getOutputChannelNames().size();
            auto midiInputs = juce::MidiInput::getAvailableDevices();
            for (auto input : midiInputs)
            {
                if (deviceManager.isMidiInputDeviceEnabled(input.identifier))
                     deviceManager.addMidiInputDeviceCallback(input.identifier, this);
                else
                    deviceManager.removeMidiInputDeviceCallback(input.identifier, this);
            }
        }
    }
    else if (source == bottomComponent.audioPlaybackDemo.fileDraggedFromBrowser) 
    {
        //std::unique_ptr<juce::StringArray> null = std::make_unique<juce::StringArray >();
        auto files = bottomComponent.audioPlaybackDemo.getSelectedFiles();
        if (soundPlayers[0]->soundPlayerMode == SoundPlayer::Mode::KeyMap && soundPlayers[0]->keyMappedSoundboard != nullptr)
        {
            soundPlayers[0]->keyMappedSoundboard->fileDragMove(files, 
                            getMouseXYRelative().getX(), 
                            getMouseXYRelative().getY());
        }
        else
        {
            int cartPosition = soundPlayers[0]->playlistbisViewport.getPosition().getX();
            int playlistScrollPosition = soundPlayers[0]->playlistViewport.getViewPositionY();
            int cartScrollPoisiton = soundPlayers[0]->playlistbisViewport.getViewPositionY();

            if (getMouseXYRelative().getX() < (soundPlayers[0]->myPlaylists[0]->getPosition().getX() + soundPlayers[0]->myPlaylists[0]->getWidth()))
                soundPlayers[0]->myPlaylists[0]->fileDragMove(files, getMouseXYRelative().getX(), getMouseXYRelative().getY() - playlistStartY + playlistScrollPosition);
            else if (getMouseXYRelative().getX() > cartPosition)
                soundPlayers[0]->myPlaylists[1]->fileDragMove(files, getMouseXYRelative().getX() - cartPosition, getMouseXYRelative().getY() - playlistStartY + cartScrollPoisiton);
            else
            {
                soundPlayers[0]->myPlaylists[0]->fileDragExit(files);
                soundPlayers[0]->myPlaylists[1]->fileDragExit(files);
            }
        }
        grabKeyboardFocus();
    }
    else if (source == bottomComponent.audioPlaybackDemo.fileDroppedFromBrowser)
    {
        int cartPosition = soundPlayers[0]->playlistbisViewport.getPosition().getX();
        int playlistScrollPosition = soundPlayers[0]->playlistViewport.getViewPositionY();
        int cartScrollPoisiton = soundPlayers[0]->playlistbisViewport.getViewPositionY();

        auto files = bottomComponent.audioPlaybackDemo.getSelectedFiles();
        if (soundPlayers[0]->soundPlayerMode == SoundPlayer::Mode::KeyMap && soundPlayers[0]->keyMappedSoundboard != nullptr)
        {
            soundPlayers[0]->keyMappedSoundboard->filesDropped(files, getMouseXYRelative().x, getMouseXYRelative().y);
            soundPlayers[0]->keyMappedSoundboard->fileDragExit(files);
        }
        else
        {
            if (getMouseXYRelative().getX() < (soundPlayers[0]->myPlaylists[0]->getPosition().getX() + soundPlayers[0]->myPlaylists[0]->getWidth()))
                soundPlayers[0]->myPlaylists[0]->filesDropped(files, getMouseXYRelative().getX(), getMouseXYRelative().getY() - playlistStartY + playlistScrollPosition);
            else if (getMouseXYRelative().getX() > cartPosition)
                soundPlayers[0]->myPlaylists[1]->filesDropped(files, getMouseXYRelative().getX() - cartPosition, getMouseXYRelative().getY() - playlistStartY + cartScrollPoisiton);
            soundPlayers[0]->myPlaylists[0]->fileDragExit(files);
            soundPlayers[0]->myPlaylists[1]->fileDragExit(files);
        }
        grabKeyboardFocus();
    }
    else if (source == bottomComponent.audioPlaybackDemo.fileDoubleClickBroadcaster.get())
    {
        soundPlayers[0]->loadInFirstEmptyPlayer(bottomComponent.audioPlaybackDemo.getSelectedFiles()[0]);
        grabKeyboardFocus();
    }
    else if (source == bottomComponent.recorderComponent.mouseDragInRecorder)//when mouse is dragged in recorder, desactivate shortcuts keys for players
    {
        soundPlayers[0]->draggedPlaylist = -1;
    }
    else if (source == bottomComponent.recorderComponent.spaceBarKeyPressed)//security to catch the spacebar if the focus is lost
        soundPlayers[0]->myPlaylists[0]->spaceBarPressed();
    //XOR Solo on cues
    else if (soundPlayers[0] != nullptr)
    {
        if (source == soundPlayers[0]->myPlaylists[0]->cuePlaylistBroadcaster)
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
        else if (source == bottomComponent.textEditor.textEditor.textFocusLostBroadcaster.get())
        {
            grabKeyboardFocus();
        }
#if RFBUILD
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
        else if (source == bottomComponent.dbBrowser.fileDroppedFromDataBase)
        {
            std::unique_ptr<juce::StringArray> null = std::make_unique<juce::StringArray >();


            juce::String selectedSoundName = bottomComponent.dbBrowser.getSelectedSoundName();
            juce::String fullPathName = bottomComponent.dbBrowser.getSelectedFile().getFullPathName();

            if (soundPlayers[0]->soundPlayerMode == SoundPlayer::Mode::KeyMap && soundPlayers[0]->keyMappedSoundboard != nullptr)
            {
                soundPlayers[0]->keyMappedSoundboard->setDroppedFile(getMouseXYRelative(), fullPathName, selectedSoundName);
                soundPlayers[0]->keyMappedSoundboard->fileDragExit(*null.get());
            }
            else
            {
                int cartPosition = soundPlayers[0]->playlistbisViewport.getPosition().getX();
                int playlistScrollPosition = soundPlayers[0]->playlistViewport.getViewPositionY();
                int cartScrollPoisiton = soundPlayers[0]->playlistbisViewport.getViewPositionY();

                if (getMouseXYRelative().getX() < (soundPlayers[0]->myPlaylists[0]->getPosition().getX() + soundPlayers[0]->myPlaylists[0]->getWidth()))
                {
                    soundPlayers[0]->myPlaylists[0]->setDroppedSoundName(selectedSoundName);
                    soundPlayers[0]->myPlaylists[0]->filesDropped(fullPathName, getMouseXYRelative().getX(), getMouseXYRelative().getY() - playlistStartY + playlistScrollPosition);

                }
                else if (getMouseXYRelative().getX() > cartPosition)
                {
                    soundPlayers[0]->myPlaylists[1]->setDroppedSoundName(selectedSoundName);
                    soundPlayers[0]->myPlaylists[1]->filesDropped(fullPathName, getMouseXYRelative().getX() - cartPosition, getMouseXYRelative().getY() - playlistStartY + cartScrollPoisiton);
                }
                soundPlayers[0]->myPlaylists[0]->fileDragExit(*null);
                soundPlayers[0]->myPlaylists[1]->fileDragExit(*null);
            }
            grabKeyboardFocus();
        }
        else if (source == bottomComponent.dbBrowser.fileDoubleClickBroadcaster.get())
        {
        juce::String selectedSoundName = bottomComponent.dbBrowser.getSelectedSoundName();
        juce::String fullPathName = bottomComponent.dbBrowser.getSelectedFile().getFullPathName();
        soundPlayers[0]->loadInFirstEmptyPlayer(fullPathName, selectedSoundName);
        grabKeyboardFocus();
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
        else if (source == bottomComponent.distantDbBrowser.fileDroppedFromDataBase)
        {
            juce::String selectedSoundName = bottomComponent.distantDbBrowser.getSelectedSoundName();
            juce::String fullPathName = bottomComponent.distantDbBrowser.getSelectedFile().getFullPathName();

            std::unique_ptr<juce::StringArray> null = std::make_unique<juce::StringArray >();

            if (soundPlayers[0]->soundPlayerMode == SoundPlayer::Mode::KeyMap && soundPlayers[0]->keyMappedSoundboard != nullptr)
            {
                soundPlayers[0]->keyMappedSoundboard->setDroppedFile(getMouseXYRelative(), fullPathName, selectedSoundName);
                soundPlayers[0]->keyMappedSoundboard->fileDragExit(*null.get());
            }
            else
            {
                int cartPosition = soundPlayers[0]->playlistbisViewport.getPosition().getX();
                int playlistScrollPosition = soundPlayers[0]->playlistViewport.getViewPositionY();
                int cartScrollPoisiton = soundPlayers[0]->playlistbisViewport.getViewPositionY();

                if (getMouseXYRelative().getX() < (soundPlayers[0]->myPlaylists[0]->getPosition().getX() + soundPlayers[0]->myPlaylists[0]->getWidth()))
                {
                    soundPlayers[0]->myPlaylists[0]->setDroppedSoundName(selectedSoundName);
                    soundPlayers[0]->myPlaylists[0]->filesDropped(fullPathName, getMouseXYRelative().getX(), getMouseXYRelative().getY() - playlistStartY + playlistScrollPosition);

                }
                else if (getMouseXYRelative().getX() > cartPosition)
                {
                    soundPlayers[0]->myPlaylists[1]->setDroppedSoundName(selectedSoundName);
                    soundPlayers[0]->myPlaylists[1]->filesDropped(fullPathName, getMouseXYRelative().getX() - cartPosition, getMouseXYRelative().getY() - playlistStartY + cartScrollPoisiton);
                }
                soundPlayers[0]->myPlaylists[0]->fileDragExit(*null);
                soundPlayers[0]->myPlaylists[1]->fileDragExit(*null);
            }
            grabKeyboardFocus();
        }
#endif
        else if (soundPlayers[0]->soundPlayerMode == SoundPlayer::Mode::KeyMap && soundPlayers[0]->keyMappedSoundboard != nullptr)
        {
            if (source == soundPlayers[0]->keyMappedSoundboard->grabFocusBroadcaster.get())
                grabKeyboardFocus();
        }
    }
}

MainComponent::~MainComponent()
{
    juce::FileLogger::setCurrentLogger(nullptr);
    delete km;
    removeMouseListener(this);
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
    isPreparedToPlay = false;

    bottomComponent.prepareToPlay(samplesPerBlockExpected, sampleRate);
    signalGenerator.prepareToPlay(samplesPerBlockExpected, sampleRate);

    actualSamplesPerBlockExpected = samplesPerBlockExpected;
    actualSampleRate = sampleRate;

    inputBuffer.reset(new juce::AudioBuffer<float>(2, samplesPerBlockExpected));
    outputBuffer.reset(new juce::AudioBuffer<float>(2, samplesPerBlockExpected));
    playAudioSource.reset(new juce::AudioSourceChannelInfo(*outputBuffer));
    newOutputBuffer.reset(new juce::AudioBuffer<float>(2, samplesPerBlockExpected));
    cueBuffer.reset(new juce::AudioBuffer<float>(2, samplesPerBlockExpected));
    cueAudioSource.reset(new juce::AudioSourceChannelInfo(*cueBuffer));
    bottomComponentBuffer.reset(new juce::AudioBuffer<float>(2, samplesPerBlockExpected));
    bottomComponentSource.reset(new juce::AudioSourceChannelInfo(*bottomComponentBuffer));

    if (soundPlayers[0] != nullptr)
        soundPlayers[0]->prepareToPlay(samplesPerBlockExpected, sampleRate);

    isPreparedToPlay = true;
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (soundPlayers[0] != nullptr && soundboardLaunched && isPreparedToPlay)
    {
        if (Settings::audioOutputMode == 1)
        {      
            inputBuffer->copyFrom(0, 0, *bufferToFill.buffer, 0, 0, bufferToFill.buffer->getNumSamples());

            outputBuffer->clear();
            cueBuffer->clear();

            if (soundPlayers[0] != nullptr)
                soundPlayers[0]->getNextAudioBlock(*playAudioSource.get(), *cueAudioSource.get());

            if (signalGenerator.isEnabled())
                signalGenerator.getNextAudioBlock(*playAudioSource.get());

            bufferToFill.clearActiveBufferRegion();
            bufferToFill.buffer->copyFrom(0, 0, *outputBuffer, 0, 0, bufferToFill.buffer->getNumSamples());
            bufferToFill.buffer->copyFrom(1, 0, *cueBuffer, 0, 0, bufferToFill.buffer->getNumSamples());

            bottomComponent.myMixer.getNextAudioBlock(*bottomComponentSource.get());
            bufferToFill.buffer->addFrom(1, 0, *bottomComponentBuffer.get(), 0, 0, bufferToFill.buffer->getNumSamples());

            if (!bottomComponent.recorderComponent.isEnabled())
            {
                if (soundPlayers[0] != nullptr)
                {
                    soundPlayers[0]->loudnessMeter.processBlock(*outputBuffer);
                    soundPlayers[0]->meterSource.measureBlock(*outputBuffer);
                    soundPlayers[0]->cueloudnessMeter.processBlock(*cueBuffer);
                    soundPlayers[0]->cuemeterSource.measureBlock(*cueBuffer);
                }
            }
            else if (bottomComponent.recorderComponent.isEnabled())
            {
                bottomComponent.recorderComponent.recordAudioBuffer(outputBuffer.get(), inputBuffer.get(), newOutputBuffer.get(), 2, actualSampleRate, bufferToFill.buffer->getNumSamples());
                if (bottomComponent.recorderComponent.enableMonitoring.getToggleState())
                {
                    bufferToFill.buffer->copyFrom(1, 0, *outputBuffer, 0, 0, bufferToFill.buffer->getNumSamples());
                    soundPlayers[0]->cuemeterSource.measureBlock(*bufferToFill.buffer);
                    bufferToFill.buffer->addFrom(1, 0, *newOutputBuffer, 0, 0, bufferToFill.buffer->getNumSamples());
                    soundPlayers[0]->loudnessMeter.processBlock(*bufferToFill.buffer);
                    soundPlayers[0]->meterSource.measureBlock(*bufferToFill.buffer);
                }
                else
                {
                    soundPlayers[0]->loudnessMeter.processBlock(*bufferToFill.buffer);
                    soundPlayers[0]->meterSource.measureBlock(*bufferToFill.buffer);
                    soundPlayers[0]->cueloudnessMeter.processBlock(*outputBuffer);
                    soundPlayers[0]->cuemeterSource.measureBlock(*bufferToFill.buffer);
                }
            }
        }
        else if (Settings::audioOutputMode == 2 || Settings::audioOutputMode == 3)
        {
            inputBuffer->copyFrom(0, 0, *bufferToFill.buffer, 0, 0, bufferToFill.buffer->getNumSamples());
            inputBuffer->copyFrom(1, 0, *bufferToFill.buffer, 1, 0, bufferToFill.buffer->getNumSamples());
            
            bufferToFill.clearActiveBufferRegion();

            if (soundPlayers[0] != nullptr)
                soundPlayers[0]->getNextAudioBlock(bufferToFill, bufferToFill);
            
            bottomComponent.myMixer.getNextAudioBlock(*bottomComponentSource.get());
            bufferToFill.buffer->addFrom(0, 0, *bottomComponentBuffer.get(), 0, 0, bufferToFill.buffer->getNumSamples());
            bufferToFill.buffer->addFrom(1, 0, *bottomComponentBuffer.get(), 1, 0, bufferToFill.buffer->getNumSamples());

            if (signalGenerator.isEnabled())
                signalGenerator.getNextAudioBlock(bufferToFill);

            if (!bottomComponent.recorderComponent.isEnabled())
            {
                if (soundPlayers[0] != nullptr)
                    soundPlayers[0]->meterSource.measureBlock(*bufferToFill.buffer);
                if (soundPlayers[0] != nullptr)
                    soundPlayers[0]->loudnessMeter.processBlock(*bufferToFill.buffer);
                if (soundPlayers[0] != nullptr)
                    soundPlayers[0]->newMeter->measureBlock(bufferToFill.buffer);
            }
            else if (bottomComponent.recorderComponent.isEnabled() && soundPlayers[0] != nullptr)
            {
                outputBuffer->copyFrom(0, 0, *bufferToFill.buffer, 0, 0, bufferToFill.buffer->getNumSamples());
                outputBuffer->copyFrom(1, 0, *bufferToFill.buffer, 1, 0, bufferToFill.buffer->getNumSamples());
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

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.setColour(juce::Colour(40, 134, 189));
    g.fillRect(horizontalDividerBar.get()->getBounds());
}

void MainComponent::resized()
{
    menuBar->setBounds(0, 0, getWidth(), menuBarHeight);
    timeLabel.setBounds(getWidth() - 220, 0, 200, 50);

    int windowWidth = getWidth();
    
   if (soundPlayers[0] != nullptr)
        soundPlayers[0]->setBounds(0, playersStartHeightPosition, getWidth(), getHeight() - playersStartHeightPosition - bottomHeight);

    bottomComponent.setBounds(0, getHeight(), getWidth(), 25);

    horizontalDividerBar.get()->setBounds(0, 200 - bottomHeight, getWidth(), 8);
    Component* comps[] = { soundPlayers[0], horizontalDividerBar.get(), &bottomComponent };
    if (showMixer)
        myLayout.layOutComponents(comps, 3, 0, playersStartHeightPosition, getWidth(), getHeight() - playersStartHeightPosition - 4, true, false);
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

void MainComponent::handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message)
{
    if (midiMapper.getWantsKeyPress())
    {
        midiMapper.handleMidiMessage(message);
    }
    else if (soundPlayers[0] != nullptr)
    {
        if (!soundPlayers[0]->handleIncomingMidiMessage(source, message, &midiMapper))
        {
            for (int i = 0; i < commandManager.getNumCommands(); i++)
            {
                juce::CommandID cID = commandManager.getCommandForIndex(i)->commandID;
                const juce::MessageManagerLock mmLock;
                if (message.getControllerNumber() == midiMapper.getMidiCCForCommand(cID) && message.getControllerValue() == 127)
                {
                    invokeDirectly(cID, true);
                    return;
                }
            }
        }
    }
}


bool MainComponent::keyPressed(const juce::KeyPress &key, juce::Component* originatingComponent)
{
    if (key == juce::KeyPress::tabKey)
    {
        grabKeyboardFocus();
        return true;
    }
    else if (soundPlayers[0]->soundPlayerMode == SoundPlayer::Mode::KeyMap
        && soundPlayers[0]->keyMappedSoundboard != nullptr)
    {
        if (bottomComponent.clipEditor.wantsKeyPress())
            bottomComponent.clipEditor.keyPressed(key, originatingComponent);
        else
            soundPlayers[0]->keyMappedSoundboard->keyPressed(key, originatingComponent);
    }
    else
    {
        auto command = commandManager.getKeyMappings()->findCommandForKeyPress(key);
        commandManager.invokeDirectly(command, true);
    }
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
    grabKeyboardFocus();
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
        if (soundPlayers[0]->keyMappedSoundboard != nullptr)
            soundPlayers[0]->keyMappedSoundboard->shouldShowMeters(Settings::showMeterValue.getValue());
    }
}

void MainComponent::buttonClicked(juce::Button* button)
{
    channelsMapping();
    repaint();
}

void MainComponent::channelsMapping()
{
    if (Settings::audioOutputMode == 1)
    {
        juce::FileLogger::getCurrentLogger()->writeToLog("Cannels mapping mode 1");
        tryPreferedAudioDevice(2);
        setAudioChannels(numInputsChannels, 2);
    }
    else if (Settings::audioOutputMode == 2)
    {
        juce::FileLogger::getCurrentLogger()->writeToLog("Channels mapping mode 2");
        tryPreferedAudioDevice(2);
        setAudioChannels(numInputsChannels, 2);
    }
    else if (Settings::audioOutputMode == 3)
    {
        juce::FileLogger::getCurrentLogger()->writeToLog("Channels mapping mode 3");
        tryPreferedAudioDevice(4);
        setAudioChannels(numInputsChannels, 4);
    }
}


void MainComponent::tryPreferedAudioDevice(int outputChannelsNeeded)
{
    juce::FileLogger::getCurrentLogger()->writeToLog("Trying prefered audio device");
    if (deviceManager.getCurrentAudioDevice() != nullptr)
    {
        const juce::OwnedArray<juce::AudioIODeviceType>& types = deviceManager.getAvailableDeviceTypes();
        if (types.size() > 1)
        {
            for (int i = 0; i < types.size(); ++i)
            {
                if (types[i]->getTypeName() == Settings::preferedAudioDeviceType)
                {
                    deviceManager.setCurrentAudioDeviceType(Settings::preferedAudioDeviceType, true);
                    deviceManager.initialise(numInputsChannels, outputChannelsNeeded, nullptr, true, Settings::preferedAudioDeviceName, &settings.getPreferedAudioDevice());
                    if (deviceManager.getCurrentAudioDevice() != nullptr)
                        Settings::outputChannelsNumber = deviceManager.getCurrentAudioDevice()->getOutputChannelNames().size();
                }
            }
        }
    }
    else
        deviceManager.initialise(numInputsChannels, outputChannelsNeeded, nullptr, true);

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

    soundPlayers.clear();
    soundPlayers.add(new SoundPlayer(m, &settings));
    addAndMakeVisible(soundPlayers[0]);
    soundPlayers[0]->prepareToPlay(actualSamplesPerBlockExpected, actualSampleRate);
    soundPlayers[0]->setWantsKeyboardFocus(false);

    settings.keyboardLayoutBroadcaster->removeAllChangeListeners();

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
        for (int i = 1; i < 9; i++)
        {
            auto playlistID = i < 5 ? 0 : 1;
            auto playerID = i < 5 ? i - 1 : i - 5;
            auto player = soundPlayers[0]->myPlaylists[playlistID]->players[playerID];
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
        settings.keyboardLayoutBroadcaster->addChangeListener(soundPlayers[0]);
        soundPlayers[0]->keyMappedSoundboard->grabFocusBroadcaster->addChangeListener(this);
    }
    else
    {
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

void MainComponent::stopWatchShortcuPressed()
{
    soundPlayers[0]->mainStopWatch.setVisible(true);
    soundPlayers[0]->mainStopWatch.startStopButtonClicked();
    soundPlayers[0]->resized();
}

juce::StringArray MainComponent::getMenuBarNames()
{
    return { "File", "Settings", "Soundplayer Mode", "View", "Tools", "Help" };
}

juce::PopupMenu MainComponent::getMenuForIndex(int menuIndex, const juce::String& menuName)
{
    juce::PopupMenu menu;

    if (menuIndex == 0)
    {
        menu.addCommandItem(&commandManager, CommandIDs::open, "Open");
        menu.addCommandItem(&commandManager, CommandIDs::save, "Save");
        menu.addSeparator();
        menu.addCommandItem(&commandManager, CommandIDs::quit, "Quit");
    }
    else if (menuIndex == 1)
    {
        menu.addCommandItem(&commandManager, CommandIDs::openSettings, "General settings");
        menu.addCommandItem(&commandManager, CommandIDs::audioSettings, "Audio settings");
        menu.addCommandItem(&commandManager, CommandIDs::keyboardMapping, "Keyboard shortcuts");
        menu.addCommandItem(&commandManager, CommandIDs::midiMapping, "Midi mapping");
        menu.addCommandItem(&commandManager, CommandIDs::enableOSC, "Enable OSC");
    }
    else if (menuIndex == 2)
    {
        menu.addCommandItem(&commandManager, CommandIDs::launchPlaylist, "One playlist, one cart");
        menu.addCommandItem(&commandManager, CommandIDs::launch8Faders, "Eight faders");
        menu.addCommandItem(&commandManager, CommandIDs::launchKeyMapped, "Keyboard Mapped");
    }
    else if (menuIndex == 3)
    {
        menu.addCommandItem(&commandManager, CommandIDs::showClock, "Show clock");
        menu.addCommandItem(&commandManager, CommandIDs::showTimer, "Show timer");
        menu.addSeparator();
        menu.addCommandItem(&commandManager, CommandIDs::showInBottomPanel, "Show clicked sound sound in panel");
        menu.addCommandItem(&commandManager, CommandIDs::viewLastPlayedSound, "Show last played sound in panel");
        menu.addCommandItem(&commandManager, CommandIDs::showEnveloppe, "Show enveloppe on clip");
        menu.addSeparator();
        menu.addCommandItem(&commandManager, CommandIDs::showIndividualMeters, "Show individuals meters");
    }
    else if (menuIndex == 4)
    {
        menu.addCommandItem(&commandManager, CommandIDs::showSignalGenerator, "Signal generator");
    }
    else if (menuIndex == 5)
    {
        menu.addCommandItem(&commandManager, CommandIDs::documentation, "Documentation");
        menu.addCommandItem(&commandManager, CommandIDs::about, "About");
        menu.addCommandItem(&commandManager, CommandIDs::autoCheckUpdate, "Automatically check for new update");
    }
    return menu;
}

void MainComponent::menuItemSelected(int menuItemID, int topLevelMenuIndex)
{

}

void MainComponent::initializeBottomComponent()
{
    addAndMakeVisible(bottomComponent);
    bottomComponent.audioPlaybackDemo.fileDraggedFromBrowser->addChangeListener(this);
    bottomComponent.audioPlaybackDemo.fileDroppedFromBrowser->addChangeListener(this);
    bottomComponent.audioPlaybackDemo.fileDoubleClickBroadcaster->addChangeListener(this);
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

#if RFBUILD
    bottomComponent.audioPlaybackDemo.fileDraggedFromBrowser->addChangeListener(this);
    bottomComponent.audioPlaybackDemo.fileDroppedFromBrowser->addChangeListener(this);
    bottomComponent.dbBrowser.fileDraggedFromDataBase->addChangeListener(this);
    bottomComponent.dbBrowser.fileDroppedFromDataBase->addChangeListener(this);
    bottomComponent.dbBrowser.fileDoubleClickBroadcaster->addChangeListener(this);
    bottomComponent.distantDbBrowser.fileDraggedFromDataBase->addChangeListener(this);
    bottomComponent.distantDbBrowser.fileDroppedFromDataBase->addChangeListener(this);
#endif
}


juce::ApplicationCommandTarget* MainComponent::getNextCommandTarget()
{
    return soundPlayers[0];
}


void MainComponent::getAllCommands(juce::Array<juce::CommandID>& commands)
{
    juce::Array<juce::CommandID> c{ CommandIDs::startTimer,
                                    CommandIDs::showTimer,
                                    CommandIDs::showClock,
                                    CommandIDs::showIndividualMeters,
                                    CommandIDs::showEnveloppe,
                                    CommandIDs::viewLastPlayedSound,
                                    CommandIDs::showInBottomPanel,
                                    CommandIDs::lanchRecord,
                                    CommandIDs::goToSoundBrowser,
                                    CommandIDs::goToClipEditor,
                                    CommandIDs::goToClipEffect,
                                    CommandIDs::goToRecorder,
                                    CommandIDs::goToTextEditor,
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
                                    CommandIDs::deleteOutMark,
                                    CommandIDs::documentation,
                                    CommandIDs::about,
                                    CommandIDs::autoCheckUpdate,
                                    CommandIDs::enableOSC,
                                    CommandIDs::showSignalGenerator };
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
        result.setTicked(Settings::showTimer);
        result.addDefaultKeypress('t', juce::ModifierKeys::commandModifier);
        break;
    case CommandIDs::showClock:
        result.setInfo("Show / hide clock", "Show / hide clock", "Menu", 0);
        result.setTicked(Settings::showClock);
        break;
    case CommandIDs::showSignalGenerator:
        result.setInfo("Show signal generator", "Signal generator", "Menu", 0);
        result.setTicked(false);
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
    case CommandIDs::goToTextEditor:
        result.setInfo("Display text editor", "Display text editor", "Menu", 0);
        result.setTicked(false);
        result.addDefaultKeypress('5', juce::ModifierKeys::noModifiers);
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
        result.setInfo("Show last played sound in bottom panel", "View last played sound", "Menu", 0);
        result.setTicked(Settings::viewLastPlayedSound);
        break;
    case CommandIDs::showInBottomPanel:
        result.setInfo("Show clicked sound in bottom panel", "View sound in bottom panel", "Menu", 0);
        result.setTicked(Settings::showInBottomPanel);
        break;
    case CommandIDs::documentation:
        result.setInfo("Show documentation", "Documentation", "Menu", 0);
        result.setTicked(false);
        break;
    case CommandIDs::about:
        result.setInfo("About", "About", "Menu", 0);
        result.setTicked(false);
        break;
    case CommandIDs::autoCheckUpdate:
        result.setInfo("Automatically check for new update", "Check update", "Menu", 0);
        result.setTicked(Settings::autoCheckNewUpdate);
        break;
    case CommandIDs::enableOSC:
        result.setInfo("Enable OSC", "Enable OSC", "Menu", 0);
        result.setTicked(Settings::OSCEnabled);
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
        settings.setShowTimer(!Settings::showTimer);
        soundPlayers[0]->resized();
        break;
    case CommandIDs::showClock:
        juce::FileLogger::getCurrentLogger()->writeToLog("show clock");
        settings.setShowClock(!Settings::showClock);
        soundPlayers[0]->resized();
        break;
    case CommandIDs::showSignalGenerator:
        signalGeneratorButtonClicked();
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
    case CommandIDs::goToTextEditor:
        juce::FileLogger::getCurrentLogger()->writeToLog("view text editor");
        bottomComponent.getTabbedButtonBar().setCurrentTabIndex(4);
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
            int choice = checkIfSoundLoaded();
            if (choice != 0)
            {
                launchSoundPlayer(SoundPlayer::Mode::OnePlaylistOneCart);
                settings.setPreferedSoundPlayerMode(1);
                if (choice == 1)
                    reloadTempPlayers();
            }
            grabKeyboardFocus();
        }
        break;
    case CommandIDs::launch8Faders:
        if (soundPlayers[0]->getSoundPlayerMode() != SoundPlayer::Mode::EightFaders)
        {
            int choice = checkIfSoundLoaded();
            if (choice != 0)
            {
                launchSoundPlayer(SoundPlayer::Mode::EightFaders);
                settings.setPreferedSoundPlayerMode(2);
                if (choice == 1)
                    reloadTempPlayers();
            }
            grabKeyboardFocus();
        }
        break;
    case CommandIDs::launchKeyMapped:
        if (soundPlayers[0]->getSoundPlayerMode() != SoundPlayer::Mode::KeyMap)
        {
            int choice = checkIfSoundLoaded();
            if (choice != 0)
            {
                launchSoundPlayer(SoundPlayer::Mode::KeyMap);
                settings.setPreferedSoundPlayerMode(3);
                if (choice == 1)
                    reloadTempPlayers();
            }
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
    case CommandIDs::showInBottomPanel:
        settings.setShowInBottomPanel(!Settings::showInBottomPanel);
        break;
    case CommandIDs::documentation:
    {
        juce::URL("https://github.com/LucienLefebvre/LuPlayer/blob/master/Documentation/Manual.md").launchInDefaultBrowser();
        break;
    }
    case CommandIDs::about:
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::NoIcon, "About",
            "Licensed under GPLv3\n"
            "Developped by Lucien Lefebvre\n"
            "To view source, go to : \n github.com/lucienlefebvre\n");
    }
        break;
    case CommandIDs::autoCheckUpdate:
        settings.setAutoCheckUpdate(!Settings::autoCheckNewUpdate);
        break;
    case CommandIDs::enableOSC:
        settings.setOSCEnabled(!Settings::OSCEnabled);
        if (Settings::OSCEnabled)
            soundPlayers[0]->OSCInitialize();
        break;
    default:
        return false;
    }
    return true;
}

void MainComponent::releaseResources()
{
}

void MainComponent::checkNewVersion()
{
    juce::URL latestVersionURL("https://api.github.com/repos/lucienlefebvre/luplayer/releases/latest");
    std::unique_ptr<juce::InputStream> inStream(latestVersionURL.createInputStream(false, nullptr, nullptr, {}, 5000));

    if (inStream != nullptr)
    {
        auto content = inStream->readEntireStreamAsString();
        auto latestReleaseDetails = juce::JSON::parse(content);
        auto* json = latestReleaseDetails.getDynamicObject();
        auto versionString = json->getProperty("tag_name").toString();

        auto currentTokens = juce::StringArray::fromTokens(ProjectInfo::versionString, ".", {});
        auto thisTokens = juce::StringArray::fromTokens(versionString, ".", {});

        bool result = false;
        if (currentTokens[0].getIntValue() == thisTokens[0].getIntValue())
        {
            if (currentTokens[1].getIntValue() == thisTokens[1].getIntValue())
            {
                if (currentTokens[2].getIntValue() == thisTokens[2].getIntValue())
                {
                    result = currentTokens[3].getIntValue() < thisTokens[3].getIntValue();
                }
                else
                    result = currentTokens[2].getIntValue() < thisTokens[2].getIntValue();
            }
            else
                result = currentTokens[1].getIntValue() < thisTokens[1].getIntValue();

        }
        else
            result =  currentTokens[0].getIntValue() < thisTokens[0].getIntValue();

        if (result)
        {
            auto releaseNotes = json->getProperty("body").toString();

            auto updateDial = std::make_unique<updateDialog>(releaseNotes, &settings);
            updateDial->setSize(600, 400);
            updateDial->setVisible(true);

            juce::DialogWindow::showModalDialog("Update", std::move(updateDial.get()), this, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), true);
        }
    }
}

int MainComponent::checkIfSoundLoaded()
{
    int choice = 2;
    tempPlayers.clear();

    if (soundPlayers[0] != nullptr)
    {
        for (auto playlist : soundPlayers[0]->myPlaylists)
        {
            for (auto player : playlist->players)
            {
                if (player->isFileLoaded())
                {
                    tempPlayers.add(player->getPlayerInfo());
                    choice = 1;
                }
            }
        }
    }

    if (choice == 1)
    {
        std::unique_ptr<juce::AlertWindow> soundLoadedWindow;
        choice = soundLoadedWindow->showYesNoCancelBox(juce::AlertWindow::QuestionIcon, "Keep loaded sounds ?", "Some sounds may still be deleted", "Keep", "Delete", "Cancel", nullptr, nullptr);
    }
    return choice;
}

void MainComponent::reloadTempPlayers()
{
    int playerToLoadID = 0;
    for (int i = 0; i < tempPlayers.size(); i++)
    {
        if (soundPlayers[0]->soundPlayerMode == SoundPlayer::Mode::EightFaders)
        {
            int tempPlayerId = 0;
            for (auto playlist : soundPlayers[0]->myPlaylists)
            {
                for (auto player : playlist->players)
                {
                    player->setPlayerInfo(tempPlayers[tempPlayerId]);
                    tempPlayerId++;
                    if (tempPlayers.size() == tempPlayerId)
                        return;
                }
            }
        }
        else if (soundPlayers[0]->soundPlayerMode == SoundPlayer::Mode::KeyMap)
        {
            if (soundPlayers[0]->keyMappedSoundboard->mappedPlayers[playerToLoadID] != nullptr)
            {
                if (soundPlayers[0]->keyMappedSoundboard->mappedPlayers[playerToLoadID]->isVisible())
                {
                    soundPlayers[0]->keyMappedSoundboard->mappedPlayers[playerToLoadID]->getPlayer()->setPlayerInfo(tempPlayers[i]);
                    playerToLoadID++;
                }

                else
                {
                    while (!soundPlayers[0]->keyMappedSoundboard->mappedPlayers[playerToLoadID]->isVisible())
                    {
                        if (soundPlayers[0]->keyMappedSoundboard->mappedPlayers[playerToLoadID + 1] != nullptr)
                            playerToLoadID++;
                        else
                            break;
                    }
                    soundPlayers[0]->keyMappedSoundboard->mappedPlayers[playerToLoadID]->getPlayer()->setPlayerInfo(tempPlayers[i]);
                    playerToLoadID++;
                }
            }
        }
        else if(soundPlayers[0]->soundPlayerMode == SoundPlayer::Mode::OnePlaylistOneCart)
        {
            while (tempPlayers.size() > soundPlayers[0]->myPlaylists[0]->players.size())
            {
                soundPlayers[0]->myPlaylists[0]->addPlayer(soundPlayers[0]->myPlaylists[0]->players.size() - 1);
            }
            auto playerToLoad = soundPlayers[0]->myPlaylists[0]->players[i];
            if (playerToLoad != nullptr)
                playerToLoad->setPlayerInfo(tempPlayers[i]);
        }
    }
}