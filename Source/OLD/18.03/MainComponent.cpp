#include "MainComponent.h"
#define JUCE_ASIO 1

//vérifier OSC
//Crash quand on change audio output mode sur 1 en lecture?????
//vérifier fader4
//empécher que la playlist se repositionne quand on fait bouger la barre de séparation
//lag barre de séparation
//couleurs cue meter trop rouges
//lecteurs joouent toujours quand on supprime le fichier
//faire bouton autoplay sur file browser
//drag and drop ne fonctionne plus depuis l'exterieur
//crackements en asio
//problème de sampleRate
//finir recorder : ajouter bouton save, in & out, gérer monitoring
//se souvenir des entrées / sorties sélectionnées
//demander si on veut supprimer l'neregistrement quand on clic de nouveau sur record
//bug lorsqu'on clique sur la waveform dans un player, puis dans le rec, puis de nouveau dans le même player
//==============================================================================
MainComponent::MainComponent() : 
    juce::AudioAppComponent(deviceManager),
    audioSetupComp(deviceManager,
    0,     // minimum input channels
    2,   // maximum input channels
    0,     // minimum output channels
    4,   // maximum output channels
    true, // ability to select midi inputs
    true, // ability to select midi output device
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


    tryPreferedAudioDevice(2);
    juce::Timer::startTimer(20);
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
    bottomComponent.recorderComponent.mouseDragInRecorder->addChangeListener(this);
    //bottomComponent.setBounds(0, getHeight() - bottomHeight, getWidth(), bottomHeight);



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
    connectOSCButton.setBounds(300, 0, 100, 25);
    connectOSCButton.setButtonText("Connect OSC");
    connectOSCButton.onClick = [this] { OSCInitialize(); };

    addAndMakeVisible(oscStatusLabel);
    oscStatusLabel.setBounds(400, 0, 100, 25);
    oscStatusLabel.setText("OSC not connected", juce::NotificationType::dontSendNotification);
    oscStatusLabel.setColour(juce::Label::textColourId, juce::Colours::red);

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




    channelsMapping();
    midiInitialize();

    myLayout.setItemLayout(0,          // for item 0
        200, 1080,    // must be between 50 and 100 pixels in size
        -0.75);      // and its preferred size is 60% of the total available space
    myLayout.setItemLayout(1, 4, 4, 4);
    myLayout.setItemLayout(2,          // for item 1
        25, 600, // size must be between 20% and 60% of the available space
        230);        // and its preferred size is 50 pixels

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
    if (source == &deviceManager)
    {
        if (deviceManager.getCurrentAudioDevice() != nullptr)
        {
            Settings* settings = new Settings();
            settings->setPreferedAudioDeviceType(deviceManager.getCurrentAudioDeviceType());
            settings->setPreferedAudioDeviceName(deviceManager.getCurrentAudioDevice()->getName());
            settings->setPreferedAudioDevice(deviceManager.getAudioDeviceSetup());
            Settings::sampleRateValue = deviceManager.getAudioDeviceSetup().sampleRate;
            settings->saveOptions();
            delete settings;

            Settings::outputChannelsNumber = deviceManager.getCurrentAudioDevice()->getOutputChannelNames().size();
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
            soundPlayers[0]->myPlaylists[0]->fileDragExit();
            soundPlayers[0]->myPlaylists[1]->fileDragExit();
        }


    }
    else if (source == bottomComponent.audioPlaybackDemo.fileDroppedFromBrowser)
    {
        //std::unique_ptr<Settings> settings = std::make_unique<Settings>();
        int cartPosition = soundPlayers[0]->playlistbisViewport.getPosition().getX();
        int playlistScrollPosition = soundPlayers[0]->playlistViewport.getViewPositionY();
        int cartScrollPoisiton = soundPlayers[0]->playlistbisViewport.getViewPositionY();
        std::unique_ptr<juce::File> myFile = std::make_unique<juce::File>(bottomComponent.audioPlaybackDemo.fileBrowser->getSelectedFile(0));
        if (getMouseXYRelative().getX() < (soundPlayers[0]->myPlaylists[0]->getPosition().getX() + soundPlayers[0]->myPlaylists[0]->getWidth()))
            soundPlayers[0]->myPlaylists[0]->filesDropped(myFile->getFullPathName(), getMouseXYRelative().getX(), getMouseXYRelative().getY() - 60 + playlistScrollPosition);
        else if (getMouseXYRelative().getX() > cartPosition)
            soundPlayers[0]->myPlaylists[1]->filesDropped(myFile->getFullPathName(), getMouseXYRelative().getX() - cartPosition, getMouseXYRelative().getY() - 60 + cartScrollPoisiton);
        soundPlayers[0]->myPlaylists[0]->fileDragExit();
        soundPlayers[0]->myPlaylists[1]->fileDragExit();
    }
    else if (source == bottomComponent.recorderComponent.mouseDragInRecorder)
    {
        soundPlayers[0]->draggedPlaylist = -1;
    }
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
    //bufferToFill.clearActiveBufferRegion();
    //myMixer.getNextAudioBlock(bufferToFill);
    if (soundPlayers[0] != nullptr)
    {
        if (Settings::audioOutputMode == 3 && (bufferToFill.buffer->getNumChannels() > 3))
        {
            juce::AudioBuffer<float>* outputBuffer = new juce::AudioBuffer<float>(2, bufferToFill.buffer->getNumSamples());
            juce::AudioSourceChannelInfo* playAudioSource = new juce::AudioSourceChannelInfo(*outputBuffer);
            juce::AudioSourceChannelInfo* cueAudioSource = new juce::AudioSourceChannelInfo(*outputBuffer);

            myCueMixer.getNextAudioBlock(*cueAudioSource);
            bufferToFill.buffer->copyFrom(2, 0, *outputBuffer, 0, 0, bufferToFill.buffer->getNumSamples());
            bufferToFill.buffer->copyFrom(3, 0, *outputBuffer, 1, 0, bufferToFill.buffer->getNumSamples());
            soundPlayers[0]->cueloudnessMeter.processBlock(*outputBuffer);
            soundPlayers[0]->cuemeterSource.measureBlock(*outputBuffer);
            myMixer.getNextAudioBlock(*playAudioSource);
            bufferToFill.buffer->copyFrom(0, 0, *outputBuffer, 0, 0, bufferToFill.buffer->getNumSamples());
            bufferToFill.buffer->copyFrom(1, 0, *outputBuffer, 1, 0, bufferToFill.buffer->getNumSamples());
            soundPlayers[0]->loudnessMeter.processBlock(*outputBuffer);
            soundPlayers[0]->meterSource.measureBlock(*outputBuffer);
            delete(outputBuffer);
            delete(playAudioSource);
        }
        else if (Settings::audioOutputMode == 1)
        {
            juce::AudioBuffer<float>* outputBuffer = new juce::AudioBuffer<float>(2, bufferToFill.buffer->getNumSamples());
            juce::AudioSourceChannelInfo* playAudioSource = new juce::AudioSourceChannelInfo(*outputBuffer);

            myMixer.getNextAudioBlock(*playAudioSource);
            soundPlayers[0]->loudnessMeter.processBlock(*outputBuffer);
            bufferToFill.buffer->copyFrom(0, 0, *outputBuffer, 0, 0, bufferToFill.buffer->getNumSamples());
            soundPlayers[0]->meterSource.measureBlock(*bufferToFill.buffer);

            myCueMixer.getNextAudioBlock(*playAudioSource);
            soundPlayers[0]->cueloudnessMeter.processBlock(*outputBuffer);
            bufferToFill.buffer->copyFrom(1, 0, *outputBuffer, 0, 0, bufferToFill.buffer->getNumSamples());
            soundPlayers[0]->cuemeterSource.measureBlock(*bufferToFill.buffer);



            delete(outputBuffer);
            delete(playAudioSource);
        }
        else if (Settings::audioOutputMode == 2)
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
            soundPlayers[0]->meterSource.measureBlock(*outputBuffer);
            soundPlayers[0]->loudnessMeter.processBlock(*outputBuffer);

            auto activeInputChannels = deviceManager.getCurrentAudioDevice()->getActiveInputChannels();
            auto maxInputChannels = activeInputChannels.getHighestBit() + 1;
            //DBG(activeInputChannels);



            //DBG(actualSampleRate);
            if (bottomComponent.recorderComponent.isEnabled())
            bottomComponent.recorderComponent.recordAudioBuffer(outputBuffer, inputBuffer, 2, actualSampleRate, bufferToFill.buffer->getNumSamples());

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


void MainComponent::timerCallback()
{
    juce::Time* time = new juce::Time(time->getCurrentTime());
    timeLabel.setText(time->toString(false, true, true, true), juce::NotificationType::dontSendNotification);
    repaint();
    //myPlaylists[0]->currentModifiers = juce::ComponentPeer::getCurrentModifiersRealtime();
   
}


void MainComponent::audioInitialize()
{

}


void MainComponent::midiInitialize()
{
    addAndMakeVisible(midiInputList);
    midiInputList.setTextWhenNoChoicesAvailable("No MIDI Inputs Enabled");
    midiInputList.setBounds(200, 0, 100, 25);
    auto midiInputs = juce::MidiInput::getAvailableDevices();
    midiInputList.setWantsKeyboardFocus(false);


    juce::StringArray midiInputNames;

    for (auto input : midiInputs)
        midiInputNames.add(input.name);

    midiInputList.addItemList(midiInputNames, 1);
    midiInputList.onChange = [this] { setMidiInput(midiInputList.getSelectedItemIndex());
                             std::unique_ptr<Settings> settings;
                            settings.reset(new Settings());
                            settings->setPreferedMidiDevice(midiInputList.getSelectedItemIndex());
                                        };



    // find the first enabled device and use that by default
    for (auto input : midiInputs)
    {
        if (deviceManager.isMidiInputDeviceEnabled(input.identifier))
        {
            setMidiInput(midiInputs.indexOf(input));
            break;
        }
    }

    setMidiInput(Settings::preferedMidiDeviceIndex);
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
    soundPlayers[0]->handleIncomingMidiMessage(source, message);
}





bool MainComponent::keyPressed(const juce::KeyPress &key, juce::Component* originatingComponent)
{
    if (key == 73 || key == 75 || key == 79 || key == 76)
    {
        if (soundPlayers[0]->draggedPlaylist != -1)
        soundPlayers[0]->keyPressed(key, originatingComponent);
        else if (soundPlayers[0]->draggedPlaylist == -1)
            bottomComponent.recorderComponent.keyPressed(key);
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
    if (oscConnected == false)
    {
        if (receiver.connect(juce::int32(Settings::inOscPort)) && sender.connect(Settings::ipAdress, juce::int32(Settings::outOscPort)))
        {
            oscStatusLabel.setText("OSC Connected", juce::NotificationType::dontSendNotification);
            oscStatusLabel.setColour(juce::Label::textColourId, juce::Colours::green);
            connectOSCButton.setButtonText("Disconnect OSC");
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
            connectOSCButton.setButtonText("Connect OSC");
            oscStatusLabel.setText("OSC Not Connected", juce::NotificationType::dontSendNotification);
            oscStatusLabel.setColour(juce::Label::textColourId, juce::Colours::red);
            oscConnected = false;
        }
    }
}


void MainComponent::OSCClean()
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
void MainComponent::oscMessageReceived(const juce::OSCMessage& message)
{
    soundPlayers[0]->oscMessageReceived(message);
}

void MainComponent::OSCsend(int destination, float value)
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



void MainComponent::OSCsend(int destination, juce::String string)
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

void MainComponent::buttonClicked(juce::Button* button)
{

    channelsMapping();
    soundPlayers[0]->metersInitialize();

}


void MainComponent::mouseDown(const juce::MouseEvent& event)
{

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
                    deviceManager.initialise(2, outputChannelsNeeded, nullptr, true, Settings::preferedAudioDeviceName);
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
}