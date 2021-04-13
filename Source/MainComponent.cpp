#include "MainComponent.h"
#define JUCE_ASIO 1
//#define INFO_BUFFER_SIZE 32767
//==============================================================================
//bug si on supprime un lecteur qui est en train de jouer -> emp�cher
//bug affichage barre LU quand on passe de mono � st�reo
bool MainComponent::exitAnswered;
MainComponent::MainComponent() : 
    juce::AudioAppComponent(deviceManager),
    audioSetupComp(deviceManager,
    0,     // minimum input channels
    2,   // maximum input channels
    0,     // minimum output channels
    4,   // maximum output channels
    true, // ability to select midi inputs
    false, // ability to select midi output device
    true, // treat channels as stereo pairs
    true), // hide advanced options
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
        setAudioChannels(2, 4);
    }

    //setLookAndFeel(new juce::LookAndFeel_V4((juce::LookAndFeel_V4::getMidnightColourScheme())));
   /* addLookAndFeel(new LookAndFeel_V4(), "LookAndFeel_V4 (Dark)");
    addLookAndFeel(new LookAndFeel_V4(LookAndFeel_V4::getMidnightColourScheme()), "LookAndFeel_V4 (Midnight)");
    addLookAndFeel(new LookAndFeel_V4(LookAndFeel_V4::getGreyColourScheme()), "LookAndFeel_V4 (Grey)");
    addLookAndFeel(new LookAndFeel_V4(LookAndFeel_V4::getLightColourScheme()), "LookAndFeel_V4 (Light)");*/
    //std::unique_ptr<Settings> settings = std::make_unique<Settings>();
    //Settings* globalSettings = new Settings();

    tryPreferedAudioDevice(2);
    juce::MultiTimer::startTimer(0, 500);
    juce::MultiTimer::startTimer(1, 5000);
    deviceManager.initialise(2, 2, nullptr, true);
    if (deviceManager.getCurrentAudioDevice() != nullptr)
    {
        std::unique_ptr<Settings> settings = std::make_unique<Settings>();
        Settings::sampleRate = deviceManager.getAudioDeviceSetup().sampleRate;
    }
    
    //ADD SOUND PLAYER
    soundPlayers.add(new SoundPlayer());
    addAndMakeVisible(soundPlayers[0]);
    soundPlayers[0]->prepareToPlay(actualSamplesPerBlockExpected, actualSampleRate);
    //soundPlayers[0]->setBounds(0, 30, getWidth(), getHeight() - playersStartHeightPosition);
    myMixer.addInputSource(&soundPlayers[0]->myMixer, false);
    myCueMixer.addInputSource(&soundPlayers[0]->myCueMixer, false);

    //ADD BOTTOM COMPONENT
    addAndMakeVisible(bottomComponent);
    myMixer.addInputSource(&bottomComponent.myMixer, false);
    bottomComponent.audioPlaybackDemo.fileDraggedFromBrowser->addChangeListener(this);
    bottomComponent.audioPlaybackDemo.fileDroppedFromBrowser->addChangeListener(this);
    bottomComponent.dbBrowser.fileDraggedFromDataBase->addChangeListener(this);
    bottomComponent.dbBrowser.fileDroppedFromDataBase->addChangeListener(this);
    bottomComponent.recorderComponent.mouseDragInRecorder->addChangeListener(this);
    bottomComponent.recorderComponent.spaceBarKeyPressed->addChangeListener(this);
    bottomComponent.setName("bottom component");
    bottomComponent.setWantsKeyboardFocus(false);
    bottomComponent.getTabbedButtonBar().addChangeListener(this);

    bottomComponent.getTabbedButtonBar().getTabButton(0)->addMouseListener(this, false);
    bottomComponent.getTabbedButtonBar().getTabButton(1)->addMouseListener(this, false);
    bottomComponent.getTabbedButtonBar().getTabButton(2)->addMouseListener(this, false);
    bottomComponent.getTabbedButtonBar().getTabButton(3)->addMouseListener(this, false);
    //bottomComponent.setBounds(0, getHeight() - bottomHeight, getWidth(), bottomHeight);
    //addMouseListener(this, true);


    //ADD BUTTONS
    addAndMakeVisible(saveButton);
    saveButton.setBounds(500, 0, 100, 25);
    saveButton.setButtonText("Save");
    saveButton.onClick = [this] { savePlaylist(); };

    addAndMakeVisible(loadButton);
    loadButton.setBounds(600, 0, 100, 25);
    loadButton.setButtonText("Load");
    loadButton.onClick = [this] { loadPlaylist(); };
    
    addAndMakeVisible(timeLabel);
    timeLabel.setBounds(getParentWidth()-200, 0, 200, 50);
    timeLabel.setFont(juce::Font(50.00f, juce::Font::plain).withTypefaceStyle("Regular"));

    addAndMakeVisible(connectOSCButton);
    connectOSCButton.setBounds(200, 0, 100, 25);
    connectOSCButton.setButtonText("Connect OSC");
    connectOSCButton.onClick = [this] { OSCInitialize(); };


    /*addAndMakeVisible(oscStatusLabel);
    oscStatusLabel.setBounds(400, 0, 100, 25);
    oscStatusLabel.setText("OSC not connected", juce::NotificationType::dontSendNotification);
    oscStatusLabel.setColour(juce::Label::textColourId, juce::Colours::red);*/

    audioSetupComp.setColour(juce::ResizableWindow::ColourIds::backgroundColourId, juce::Colours::white);
    audioSetupWindow.setSize(400, 800);
    audioSetupComp.setSize(400, 800);
    addAndMakeVisible(audioSetupButton);
    audioSetupButton.setBounds(100, 0, 100, 25);
    audioSetupButton.setButtonText("Audio Setup");
    audioSetupButton.onClick = [this] { audioSetupWindow.showDialog("Audio Setup", &audioSetupComp, this, 
                                        getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), true, true, false); };

    deviceManager.addChangeListener(this);
    Settings::audioOutputModeValue.addListener(this);

    /*addAndMakeVisible(&cpuUsage);
    cpuUsage.setBounds(700, 0, 100, 25);*/
    //addAndMakeVisible(timerSlider);
    //timerSlider.setBounds(700, 0, 300, 25);

    timerSlider.setRange(10, 1000);
   // timerSlider.setValue(100);
    timerSlider.addListener(this);



    channelsMapping();
    midiInitialized = false;
    midiInitialize();

    myLayout.setItemLayout(0,          // for item 0
        200, 1080,    // must be between 50 and 100 pixels in size
        -0.75);      // and its preferred size is 60% of the total available space
    myLayout.setItemLayout(1, 4, 4, 4);
    myLayout.setItemLayout(2,          // for item 1
        5, 600, // size must be between 20% and 60% of the available space
        300);        // and its preferred size is 50 pixels

    horizontalDividerBar.reset(new juce::StretchableLayoutResizerBar(&myLayout, 1, false));
    addAndMakeVisible(horizontalDividerBar.get());


    settingsButton.setBounds(0, 0, 100, 25);
    addAndMakeVisible(settingsButton);
    settingsButton.setButtonText("Settings");
    settingsButton.onClick = [this] { settingsButtonClicked(); };


    addKeyListener(this);
    setWantsKeyboardFocus(true);
    start1 = juce::Time::currentTimeMillis();

    juce::File convFile(Settings::convertedSoundsPath.toStdString());
    if (!convFile.isDirectory())
    {
        std::unique_ptr<juce::AlertWindow> noConvertSoundsWindow;
        noConvertSoundsWindow->showMessageBox(juce::AlertWindow::AlertIconType::InfoIcon, "", "Select Converted Sounds Folder");
        settingsButtonClicked();
    }

    setSize(1300, 550);
    defaultPlayersPlaylist(4);

}

void MainComponent::settingsButtonClicked()
{  


        //std::unique_ptr<Settings> settings;
        //settings.reset(new Settings(myPlaylists[0], myPlaylists[1], &sender));
        Settings* settings = new Settings();
        settings->saveButton.addListener(this);
        juce::DialogWindow::LaunchOptions o;
        //o.content.setOwned(new Settings(myPlaylists[0], myPlaylists[1], &sender));
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
void MainComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
{

    if (source == &bottomComponent.getTabbedButtonBar())
    {
          myLayout.setItemLayout(2,          // for item 1
              25, 600, // size must be between 20% and 60% of the available space
              300);        // and its preferred size is 50 pixels
          Component* comps[] = { soundPlayers[0], horizontalDividerBar.get(), &bottomComponent };
          myLayout.layOutComponents(comps, 3, 0, playersStartHeightPosition, getWidth(), getHeight() - playersStartHeightPosition, true, false);
    }
    if (source == &deviceManager)
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
    else if (source == bottomComponent.dbBrowser.fileDroppedFromDataBase)
    {
        //std::unique_ptr<Settings> settings = std::make_unique<Settings>();
        int cartPosition = soundPlayers[0]->playlistbisViewport.getPosition().getX();
        int playlistScrollPosition = soundPlayers[0]->playlistViewport.getViewPositionY();
        int cartScrollPoisiton = soundPlayers[0]->playlistbisViewport.getViewPositionY();
        std::unique_ptr<juce::StringArray> null = std::make_unique<juce::StringArray >();

        if (getMouseXYRelative().getX() < (soundPlayers[0]->myPlaylists[0]->getPosition().getX() + soundPlayers[0]->myPlaylists[0]->getWidth()))
        {
            soundPlayers[0]->myPlaylists[0]->setDroppedSoundName(bottomComponent.dbBrowser.getSelectedSoundName());
            soundPlayers[0]->myPlaylists[0]->filesDropped(bottomComponent.dbBrowser.getSelectedFile().getFullPathName(), getMouseXYRelative().getX(), getMouseXYRelative().getY() - 60 + playlistScrollPosition);
            
        }
        else if (getMouseXYRelative().getX() > cartPosition)
        {
            soundPlayers[0]->myPlaylists[1]->setDroppedSoundName(bottomComponent.dbBrowser.getSelectedSoundName());
            soundPlayers[0]->myPlaylists[1]->filesDropped(bottomComponent.dbBrowser.getSelectedFile().getFullPathName(), getMouseXYRelative().getX() - cartPosition, getMouseXYRelative().getY() - 60 + cartScrollPoisiton);          
        }
        soundPlayers[0]->myPlaylists[0]->fileDragExit(*null);
        soundPlayers[0]->myPlaylists[1]->fileDragExit(*null);
    }
    else if (source == bottomComponent.recorderComponent.mouseDragInRecorder)
    {
        soundPlayers[0]->draggedPlaylist = -1;
    }
    else if (source == bottomComponent.recorderComponent.spaceBarKeyPressed)
        soundPlayers[0]->myPlaylists[0]->spaceBarPressed();

}
MainComponent::~MainComponent()
{
    removeMouseListener(this);
    myMixer.removeAllInputs();
    myCueMixer.removeAllInputs();

    deviceManager.removeChangeListener(this);
    bottomComponent.recorderComponent.mouseDragInRecorder->removeChangeListener(this);
    Settings::audioOutputModeValue.removeListener(this);
    shutdownAudio();
    //delete globalSettings;
}

void MainComponent::exitRequested()
{

}
void MainComponent::deleteConvertedFiles()
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

    myMixer.prepareToPlay(samplesPerBlockExpected, sampleRate);
    myCueMixer.prepareToPlay(samplesPerBlockExpected, sampleRate);
    bottomComponent.prepareToPlay(samplesPerBlockExpected, sampleRate);

    actualSamplesPerBlockExpected = samplesPerBlockExpected;
    actualSampleRate = sampleRate;

  /*  loudnessMeter.prepareToPlay(actualSampleRate, 2, actualSamplesPerBlockExpected, 20);
    cueloudnessMeter.prepareToPlay(actualSampleRate, 2, actualSamplesPerBlockExpected, 20);
    meterSource.resize(2, sampleRate * 0.1 / samplesPerBlockExpected);
    cuemeterSource.resize(2, sampleRate * 0.1 / samplesPerBlockExpected);

    myMixer.prepareToPlay(samplesPerBlockExpected, sampleRate);
    myCueMixer.prepareToPlay(samplesPerBlockExpected, sampleRate);
    actualSampleRate = sampleRate;
    actualSamplesPerBlockExpected = samplesPerBlockExpected;*/

}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{

    if (soundPlayers[0] != nullptr)
    {
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
                soundPlayers[0]->loudnessMeter.processBlock(*outputBuffer);
                soundPlayers[0]->meterSource.measureBlock(*outputBuffer);
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

            bufferToFill.clearActiveBufferRegion();
            juce::AudioBuffer<float>* outputBuffer = new juce::AudioBuffer<float>(2, bufferToFill.buffer->getNumSamples());
            juce::AudioSourceChannelInfo* playAudioSource = new juce::AudioSourceChannelInfo(*outputBuffer);
            myMixer.getNextAudioBlock(*playAudioSource);
            bufferToFill.buffer->copyFrom(0, 0, *outputBuffer, 0, 0, bufferToFill.buffer->getNumSamples());
            bufferToFill.buffer->copyFrom(1, 0, *outputBuffer, 1, 0, bufferToFill.buffer->getNumSamples());

            bottomComponent.mixerPanel.getNextAudioBlock(bufferToFill);
            //OPUS
            //bottomComponent.mixerPanel.remoteInput1.sendStream(outputBuffer);
            //bottomComponent.mixerPanel.remoteInput1.receiveStream(outputBuffer);
            //bufferToFill.buffer->copyFrom(0, 0, bottomComponent.mixerPanel.remoteInput1.decodedBuffer, 0, 0, bufferToFill.buffer->getNumSamples());
            //bufferToFill.buffer->copyFrom(1, 0, bottomComponent.mixerPanel.remoteInput1.decodedBuffer, 0, 0, bufferToFill.buffer->getNumSamples());
            if (!bottomComponent.recorderComponent.isEnabled())
            {
                soundPlayers[0]->meterSource.measureBlock(*outputBuffer);
                soundPlayers[0]->loudnessMeter.processBlock(*outputBuffer);
            }
            if (bottomComponent.recorderComponent.isEnabled())
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
            delete(inputBuffer);
            delete(outputBuffer);
            delete(playAudioSource);
        }
    }
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
}


void MainComponent::paint(juce::Graphics& g)
{

    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.setColour(juce::Colour(40, 134, 189));
    //horizontalDividerBar.get()->paint(g);
    g.fillRect(horizontalDividerBar.get()->getBounds());

}

void MainComponent::resized()
{
    timeLabel.setBounds(getParentWidth() - 220, 0, 200, 50);
    int windowWidth = getWidth();
    
   if (soundPlayers[0] != nullptr)
        soundPlayers[0]->setBounds(0, playersStartHeightPosition, getWidth(), getHeight() - playersStartHeightPosition - bottomHeight);

       bottomComponent.setBounds(0, getHeight(), getWidth(), 25);

    horizontalDividerBar.get()->setBounds(0, 200 - bottomHeight, getWidth(), 8);
    //horizontalDividerBar.get()->setColour(juce::)
    // make a list of two of our child components that we want to reposition
    Component* comps[] = { soundPlayers[0], horizontalDividerBar.get(), &bottomComponent };

    // this will position the 2 components, one above the other, to fit
    // vertically into the rectangle provided.
    myLayout.layOutComponents(comps, 3, 0, playersStartHeightPosition, getWidth(), getHeight() - playersStartHeightPosition, true, false);
}


void MainComponent::timerCallback(int timerID)
{
    if (timerID == 0)
    {
        juce::Time* time = new juce::Time(time->getCurrentTime());
        timeLabel.setText(time->toString(false, true, true, true), juce::NotificationType::dontSendNotification);
        timeLabel.toFront(false);
        cpuUsage.setText(juce::String(deviceManager.getCpuUsage()), juce::NotificationType::dontSendNotification);
    }
    else if (timerID == 1)
    {
        //midiInitialize();
    }
}


void MainComponent::audioInitialize()
{

}


void MainComponent::midiInitialize()
{
       /* addAndMakeVisible(midiInputList);
        midiInputList.setTextWhenNoChoicesAvailable("No MIDI Inputs Enabled");
        midiInputList.setBounds(200, 0, 100, 25);
        auto midiInputs = juce::MidiInput::getAvailableDevices();
        midiInputList.setWantsKeyboardFocus(false);
        midiInputList.clear();

        juce::StringArray midiInputNames;

        for (auto input : midiInputs)
            midiInputNames.add(input.name);*/

      /*  midiInputList.addItemList(midiInputNames, 1);
        midiInputList.onChange = [this] { setMidiInput(midiInputList.getSelectedItemIndex());
        std::unique_ptr<Settings> settings;
        DBG(midiInputList.getSelectedItemIndex());
        settings.reset(new Settings());
        settings->setPreferedMidiDevice(midiInputList.getSelectedItemIndex());
        };*/

        auto midiInputs = juce::MidiInput::getAvailableDevices();
        for (auto input : midiInputs)
        {
            if (!deviceManager.isMidiInputDeviceEnabled(input.identifier))
                deviceManager.setMidiInputDeviceEnabled(input.identifier, true);
                deviceManager.addMidiInputDeviceCallback(input.identifier, this);
        }

        //// find the first enabled device and use that by default
        //for (auto input : midiInputs)
        //{
        //    if (deviceManager.isMidiInputDeviceEnabled(input.identifier))
        //    {
        //        setMidiInput(midiInputs.indexOf(input));
        //        break;
        //    }
        //}

        //setMidiInput(Settings::preferedMidiDeviceIndex);
        //// if no enabled devices were found just use the first one in the list
        //if (midiInputList.getSelectedId() == 0)
        //    setMidiInput(0);
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
    //DBG(message.getControllerNumber());
    if (!isEightPlayerMode)
        soundPlayers[0]->handleIncomingMidiMessage(source, message);
    else if (eightPlayersLaunched)
        soundPlayers[0]->handleIncomingMidiMessageEightPlayers(source, message);
    if (message.getControllerNumber() == 45 && message.getControllerValue() == 127)
        {
        launchRecord();
        }
    else if (message.getControllerNumber() == 61 && message.getControllerValue() == 127)
    {
        bottomComponent.recorderComponent.setStart();
    }
    else if (message.getControllerNumber() == 62 && message.getControllerValue() == 127)
    {
        bottomComponent.recorderComponent.setStop();
    }

}





bool MainComponent::keyPressed(const juce::KeyPress &key, juce::Component* originatingComponent)
{
    //DBG(key.getKeyCode());
    if (key == 73 || key == 75 || key == 79 || key == 76 || key == 67 || key == 88 || key == 86)
    {
        if (soundPlayers[0]->draggedPlaylist != -1)
        soundPlayers[0]->keyPressed(key, originatingComponent);
        else if (soundPlayers[0]->draggedPlaylist == -1)
            bottomComponent.recorderComponent.keyPressed(key);
    }
    else if (key == juce::KeyPress::spaceKey)
    {
        if (!eightPlayersLaunched)
            soundPlayers[0]->myPlaylists[0]->spaceBarPressed();
    }
    else if (key == juce::KeyPress::F1Key)
    {
        if (!isEightPlayerMode)
            soundPlayers[0]->myPlaylists[1]->playPlayer(0);
        else
            soundPlayers[0]->myPlaylists[0]->playPlayer(0);
    }
    else if (key == juce::KeyPress::F2Key)
    {
        if (!isEightPlayerMode)
        soundPlayers[0]->myPlaylists[1]->playPlayer(1);
        else
            soundPlayers[0]->myPlaylists[0]->playPlayer(1);
    }
    else if (key == juce::KeyPress::F3Key)
    {
        if (!isEightPlayerMode)
        soundPlayers[0]->myPlaylists[1]->playPlayer(2);
        else
            soundPlayers[0]->myPlaylists[0]->playPlayer(2);
    }
    else if (key == juce::KeyPress::F4Key)
    {
        if (!isEightPlayerMode)
        soundPlayers[0]->myPlaylists[1]->playPlayer(3);
        else
            soundPlayers[0]->myPlaylists[0]->playPlayer(3);
    }
    else if (key == juce::KeyPress::F5Key)
    {
        if (!isEightPlayerMode)
        soundPlayers[0]->myPlaylists[1]->playPlayer(4);
        else
            soundPlayers[0]->myPlaylists[1]->playPlayer(0);
    }
    else if (key == juce::KeyPress::F6Key)
    {
        if (!isEightPlayerMode)
        soundPlayers[0]->myPlaylists[1]->playPlayer(5);
        else
            soundPlayers[0]->myPlaylists[1]->playPlayer(1);
    }
    else if (key == juce::KeyPress::F7Key)
    {
        if (!isEightPlayerMode)
        soundPlayers[0]->myPlaylists[1]->playPlayer(6);
        else
            soundPlayers[0]->myPlaylists[1]->playPlayer(2);
    }
    else if (key == juce::KeyPress::F8Key)
    {
        if (!isEightPlayerMode)
        soundPlayers[0]->myPlaylists[1]->playPlayer(7);
        else
            soundPlayers[0]->myPlaylists[1]->playPlayer(3);
    }
    else if (key == juce::KeyPress::F9Key)
    {
        if (!isEightPlayerMode)
            soundPlayers[0]->myPlaylists[1]->playPlayer(8);
    }
    else if (key == juce::KeyPress::F10Key)
    {
        if (!isEightPlayerMode)
            soundPlayers[0]->myPlaylists[1]->playPlayer(9);
    }
    else if (key == juce::KeyPress::F11Key)
    {
        if (!isEightPlayerMode)
            soundPlayers[0]->myPlaylists[1]->playPlayer(10);
    }
    else if (key == juce::KeyPress::F12Key)
    {
        if (!isEightPlayerMode)
            soundPlayers[0]->myPlaylists[1]->playPlayer(11);
    }
    else if (key == juce::KeyPress::escapeKey)
    {
        if (soundPlayers[0]->myPlaylists[0]->spaceBarIsPlaying == false)
            soundPlayers[0]->myPlaylists[0]->playersResetPositionClicked();
    }
    else if (key == 82)
    {
        launchRecord();
    }
    
    return false;
}

void MainComponent::savePlaylist()
{
    soundPlayers[0]->savePlaylist();
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
        //connectOSCButton.setColour(juce::TextButton::ColourIds::textColourOffId, juce::Colours::green);
        connectOSCButton.setButtonText("Disconnect OSC");
    }
    else 
    {
        //connectOSCButton.setColour(juce::TextButton::ColourIds::textColourOffId, juce::Colours::red);
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
    if (event.eventComponent == bottomComponent.getTabbedButtonBar().getTabButton(0) && bottomComponent.getCurrentContentComponent() == &bottomComponent.audioPlaybackDemo
        || event.eventComponent == bottomComponent.getTabbedButtonBar().getTabButton(3) && bottomComponent.getCurrentContentComponent() == &bottomComponent.recorderComponent
        || event.eventComponent == bottomComponent.getTabbedButtonBar().getTabButton(1) && bottomComponent.getCurrentContentComponent() == &bottomComponent.dbBrowser
        || event.eventComponent == bottomComponent.getTabbedButtonBar().getTabButton(2) && bottomComponent.getCurrentContentComponent() == &bottomComponent.dbImport)
    {
        if (bottomComponent.getHeight() < 50)
        {
            myLayout.setItemLayout(2,          // for item 1
                25, 600, // size must be between 20% and 60% of the available space
                300);        // and its preferred size is 50 pixels
            Component* comps[] = { soundPlayers[0], horizontalDividerBar.get(), &bottomComponent };
            myLayout.layOutComponents(comps, 3, 0, playersStartHeightPosition, getWidth(), getHeight() - playersStartHeightPosition, true, false);
        }
        else
        {
            myLayout.setItemLayout(2,          // for item 1
                25, 600, // size must be between 20% and 60% of the available space
                20);        // and its preferred size is 50 pixels
            Component* comps[] = { soundPlayers[0], horizontalDividerBar.get(), &bottomComponent };
            myLayout.layOutComponents(comps, 3, 0, playersStartHeightPosition, getWidth(), getHeight() - playersStartHeightPosition, true, false);
        }
    }
    //else if (event.eventComponent == bottomComponent.getTabbedButtonBar().getTabButton(1)
    //    && bottomComponent.getCurrentContentComponent() == &bottomComponent.recorderComponent)
    //{
    //    if (bottomComponent.getHeight() < 50)
    //    {
    //        myLayout.setItemLayout(2,          // for item 1
    //            25, 600, // size must be between 20% and 60% of the available space
    //            300);        // and its preferred size is 50 pixels
    //        Component* comps[] = { soundPlayers[0], horizontalDividerBar.get(), &bottomComponent };
    //        myLayout.layOutComponents(comps, 3, 0, playersStartHeightPosition, getWidth(), getHeight() - playersStartHeightPosition, true, false);
    //    }
    //    else
    //    {
    //        myLayout.setItemLayout(2,          // for item 1
    //            25, 600, // size must be between 20% and 60% of the available space
    //            20);        // and its preferred size is 50 pixels
    //        Component* comps[] = { soundPlayers[0], horizontalDividerBar.get(), &bottomComponent };
    //        myLayout.layOutComponents(comps, 3, 0, playersStartHeightPosition, getWidth(), getHeight() - playersStartHeightPosition, true, false);
    //    }
    //}
}

void MainComponent::channelsMapping()
{

    myMixer.removeAllInputs();
    myCueMixer.removeAllInputs();

 /*   loudnessMeter.prepareToPlay(actualSampleRate, 2, actualSamplesPerBlockExpected, 20);
    cueloudnessMeter.prepareToPlay(actualSampleRate, 2, actualSamplesPerBlockExpected, 20);*/

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
                    //juce::AudioDeviceManager::AudioDeviceSetup& preferedAudioDevice = Settings::Pref;
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
    for (auto i = -1; i < 3; ++i)
    {
        soundPlayers[0]->myPlaylists[0]->addPlayer(i);
        soundPlayers[0]->myPlaylists[1]->addPlayer(i);
    }
    soundPlayers[0]->myPlaylists[1]->assignLeftFader(-1);
    soundPlayers[0]->myPlaylists[1]->assignLeftFader(0);
    soundPlayers[0]->myPlaylists[1]->assignRightFader(-1);
    soundPlayers[0]->myPlaylists[1]->assignRightFader(1);
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
    if (soundPlayers[0]->myPlaylists[0]->fader1IsPlaying
        || soundPlayers[0]->myPlaylists[0]->fader2IsPlaying
        || soundPlayers[0]->myPlaylists[1]->fader1IsPlaying
        || soundPlayers[0]->myPlaylists[1]->fader2IsPlaying
        || bottomComponent.recorderComponent.isRecording())
        return true;
    else
        return false;
}

void MainComponent::setCommandLine(juce::String commandLine)
{
    if (!commandLine.compare("8p"))
    {
        DBG("command line : " << commandLine);
        isEightPlayerMode = true;
        launchEightPlayerMode();
    }

}
void MainComponent::launchEightPlayerMode()
{
    myMixer.removeAllInputs();
    myCueMixer.removeAllInputs();
    soundPlayers.clear();
    soundPlayers.add(new SoundPlayer(true));
    addAndMakeVisible(soundPlayers[0]);
    soundPlayers[0]->prepareToPlay(actualSamplesPerBlockExpected, actualSampleRate);
    //soundPlayers[0]->setBounds(0, 30, getWidth(), getHeight() - playersStartHeightPosition);
    myMixer.addInputSource(&soundPlayers[0]->myMixer, false);
    myCueMixer.addInputSource(&soundPlayers[0]->myCueMixer, false);
    soundPlayers[0]->myPlaylists[0]->isEightPlayerMode(true);
    soundPlayers[0]->myPlaylists[1]->isEightPlayerMode(true);
    for (auto i = -1; i < 3; ++i)
    {

        soundPlayers[0]->myPlaylists[0]->addPlayer(i);
        soundPlayers[0]->myPlaylists[1]->addPlayer(i);
    }
    soundPlayers[0]->myPlaylists[0]->isEightPlayerMode(true);
    soundPlayers[0]->myPlaylists[1]->isEightPlayerMode(true);
    soundPlayers[0]->myPlaylists[1]->setEightPlayersSecondCart(true);
    if (soundPlayers[0] != nullptr)
        soundPlayers[0]->setBounds(0, playersStartHeightPosition, getWidth(), getHeight() - playersStartHeightPosition - bottomHeight);
    Component* comps[] = { soundPlayers[0], horizontalDividerBar.get(), &bottomComponent };
    myLayout.layOutComponents(comps, 3, 0, playersStartHeightPosition, getWidth(), getHeight() - playersStartHeightPosition, true, false);
    eightPlayersLaunched = true;
    channelsMapping();
}