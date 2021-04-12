#include "MainComponent.h"
#define JUCE_ASIO 1


// fenêtre apparait devant les autres
//problème fader pas revenu à 0
//faderassign sur cart quand drag and drop
//==============================================================================
MainComponent::MainComponent() : 
    juce::AudioAppComponent(deviceManager),
    audioSetupComp(deviceManager,
    0,     // minimum input channels
    0,   // maximum input channels
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
    //progressBar(loadingProgress)

{



    
    // Some platforms require permissions to open input channels so request that here
    if (juce::RuntimePermissions::isRequired(juce::RuntimePermissions::recordAudio)
        && !juce::RuntimePermissions::isGranted(juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request(juce::RuntimePermissions::recordAudio,
            [&](bool granted) { setAudioChannels(granted ? 2 : 0, 4); });
    }
    else
    {
        setAudioChannels(0, 4);
    }
    playerNumber = 8;
    setSize(1300, 550);

    tryPreferedAudioDevice(2);
    juce::Timer::startTimer(100);
    deviceManager.initialise(0, 2, nullptr, true);
    if (deviceManager.getCurrentAudioDevice() != nullptr)
    {
        std::unique_ptr<Settings> settings = std::make_unique<Settings>();
    }




    //Settings::audioOutputMode = 3;

    //ADD PLAYLIST
    myPlaylists.add(new Playlist(0));
    myPlaylists[0]->prepareToPlay(actualSamplesPerBlockExpected, actualSampleRate);
    myPlaylists[0]->playlistMixer.prepareToPlay(actualSamplesPerBlockExpected, actualSampleRate);
    myPlaylists[0]->playlistCueMixer.prepareToPlay(actualSamplesPerBlockExpected, actualSampleRate);



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

    playlistViewport.setBounds(0, playersStartHeightPosition, 715, (getParentHeight() - playersStartHeightPosition));
    playlistViewport.setViewedComponent(myPlaylists[0], false);
    playlistViewport.setWantsKeyboardFocus(false);
    addAndMakeVisible(playlistViewport);
    playlistViewport.setMouseClickGrabsKeyboardFocus(false);

    addAndMakeVisible(addPlayerPlaylist);
    addPlayerPlaylist.setButtonText("Add Player");
    addPlayerPlaylist.onClick = [this] { myPlaylists[0]->addPlayer(myPlaylists[0]->players.size() - 1); };

    addAndMakeVisible(removePlayerPlaylist);
    removePlayerPlaylist.setButtonText("Remove Player");
    removePlayerPlaylist.onClick = [this] { myPlaylists[0]->removeButtonClicked();    };

    //ADD CART
    myPlaylists.add(new Playlist(1));
    myPlaylists[1]->prepareToPlay(actualSamplesPerBlockExpected, actualSampleRate);
    myPlaylists[1]->playlistMixer.prepareToPlay(actualSamplesPerBlockExpected, actualSampleRate);
    myPlaylists[1]->playlistCueMixer.prepareToPlay(actualSamplesPerBlockExpected, actualSampleRate);

    myPlaylists[1]->playerStoppedID.addListener(this);
    myPlaylists[1]->fader1Name.addListener(this);
    myPlaylists[1]->fader2Name.addListener(this);
    myPlaylists[1]->mouseDragX.addListener(this);
    myPlaylists[1]->mouseDragY.addListener(this);
    myPlaylists[1]->mouseDragSource.addListener(this);
    myPlaylists[1]->mouseDraggedUp.addListener(this);
    myPlaylists[1]->draggedPlayer.addListener(this);

    playlistbisViewport.setBounds(getParentWidth() / 2, playersStartHeightPosition, getParentWidth() / 2, (getParentHeight() - playersStartHeightPosition));
    playlistbisViewport.setViewedComponent(myPlaylists[1], false);
    playlistbisViewport.setWantsKeyboardFocus(false);
    addAndMakeVisible(playlistbisViewport);

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
    //deviceManager.getCurrentAudioDevice().
    deviceManager.addChangeListener(this);
    Settings::audioOutputModeValue.addListener(this);



    //CHANNELS MAPPING

    channelsMapping();
    metersInitialize();



    midiInitialize();




   

    settingsButton.setBounds(0, 0, 100, 25);
    addAndMakeVisible(settingsButton);
    settingsButton.setButtonText("Settings");
    settingsButton.onClick = [this] { settingsButtonClicked(); };


    addKeyListener(this);
    setWantsKeyboardFocus(true);

    myPlaylists[0]->setOptions();
    myPlaylists[1]->setOptions();

    start1 = juce::Time::currentTimeMillis();

    juce::File convFile(Settings::convertedSoundsPath.toStdString());
    if (!convFile.isDirectory())
    {
        std::unique_ptr<juce::AlertWindow> noConvertSoundsWindow;
        noConvertSoundsWindow->showMessageBox(juce::AlertWindow::AlertIconType::InfoIcon, "", "Select Converted Sounds Folder");
        settingsButtonClicked();
    }

    mouseDragEnd(0);
    mouseDragEnd(1);




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
        }
    }

    for (auto i = 0; i < myPlaylists[0]->players.size(); i++)
    {
        if (source == &myPlaylists[0]->players[i]->transport)
        {
            if (myPlaylists[0]->players[i]->transport.isPlaying() == false)
                DBG("player stop");
        }
    }
}
MainComponent::~MainComponent()
{
    mouseDragEnd(0);
    removeMouseListener(this);
    myMixer.removeAllInputs();
    myCueMixer.removeAllInputs();

    myPlaylists.clear(true);

    deviceManager.removeChangeListener(this);
    Settings::audioOutputModeValue.removeListener(this);
    OSCClean();
    shutdownAudio();
}


void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{

    meterSource.resize(2, sampleRate * 0.1 / samplesPerBlockExpected);
    cuemeterSource.resize(2, sampleRate * 0.1 / samplesPerBlockExpected);

    myMixer.prepareToPlay(samplesPerBlockExpected, sampleRate);
    myCueMixer.prepareToPlay(samplesPerBlockExpected, sampleRate);
    actualSampleRate = sampleRate;
    actualSamplesPerBlockExpected = samplesPerBlockExpected;
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();
    //DBG(bufferToFill.buffer->getNumChannels());
    if (Settings::audioOutputMode == 3 && (bufferToFill.buffer->getNumChannels() > 3))
    {
    juce::AudioBuffer<float>* outputBuffer = new juce::AudioBuffer<float>(bufferToFill.buffer->getNumChannels(), bufferToFill.buffer->getNumSamples());
    juce::AudioSourceChannelInfo* playAudioSource = new juce::AudioSourceChannelInfo(*outputBuffer);
    juce::AudioSourceChannelInfo* cueAudioSource = new juce::AudioSourceChannelInfo(*outputBuffer);

    myCueMixer.getNextAudioBlock(*cueAudioSource);
    bufferToFill.buffer->copyFrom(2, 0, *outputBuffer, 0, 0, bufferToFill.buffer->getNumSamples());
    bufferToFill.buffer->copyFrom(3, 0, *outputBuffer, 1, 0, bufferToFill.buffer->getNumSamples());
    cuemeterSource.measureBlock(*outputBuffer);
    myMixer.getNextAudioBlock(*playAudioSource);
    bufferToFill.buffer->copyFrom(0, 0, *outputBuffer, 0, 0, bufferToFill.buffer->getNumSamples());
    bufferToFill.buffer->copyFrom(1, 0, *outputBuffer, 1, 0, bufferToFill.buffer->getNumSamples());
    meterSource.measureBlock(*outputBuffer);

    delete(outputBuffer);
    delete(playAudioSource);
    }
    else if (Settings::audioOutputMode == 1)
    {
        juce::AudioBuffer<float>* outputBuffer = new juce::AudioBuffer<float>(bufferToFill.buffer->getNumChannels(), bufferToFill.buffer->getNumSamples());
        juce::AudioSourceChannelInfo* playAudioSource = new juce::AudioSourceChannelInfo(*outputBuffer);
        myMixer.getNextAudioBlock(*playAudioSource);
        bufferToFill.buffer->copyFrom(0, 0, *outputBuffer, 0, 0, bufferToFill.buffer->getNumSamples());
        //bufferToFill.buffer->copyFrom(0, 0, *outputBuffer, 1, 0, bufferToFill.buffer->getNumSamples());
        //bufferToFill.buffer->applyGain(0.707946);
        meterSource.measureBlock(*bufferToFill.buffer);
        myCueMixer.getNextAudioBlock(*playAudioSource);
        bufferToFill.buffer->copyFrom(1, 0, *outputBuffer, 0, 0, bufferToFill.buffer->getNumSamples());
        //bufferToFill.buffer->copyFrom(1, 0, *outputBuffer, 1, 0, bufferToFill.buffer->getNumSamples());
        cuemeterSource.measureBlock(*bufferToFill.buffer);
        delete(outputBuffer);
        delete(playAudioSource);
    }
    else if (Settings::audioOutputMode == 2)
    {
        myMixer.getNextAudioBlock(bufferToFill);
        meterSource.measureBlock(*bufferToFill.buffer);
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
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));


    //progressBar.paint(g);

}

void MainComponent::resized()
{
    playlistViewport.setBounds(0, playersStartHeightPosition, (getParentWidth()/2) - 50 - dragZoneWidth, (getParentHeight() - 40 -playersStartHeightPosition - bottomButtonsHeight));
    if (myPlaylists[0] != nullptr)
    {
        myPlaylists[0]->setSize(playlistViewport.getWidth() - 2, playlistViewport.getHeight() - 2);
        playlistViewport.setViewPosition(0, (myPlaylists[0]->minimumPlayer).toString().getIntValue() * 105);
    }
   // playlistbisViewport.setBounds(800, playersStartHeightPosition, 745, (getParentHeight() - 40 - playersStartHeightPosition));

    playlistbisViewport.setBounds(getParentWidth() / 2 + 50, playersStartHeightPosition, getParentWidth() / 2 - 60, (getParentHeight() - 40 - playersStartHeightPosition - bottomButtonsHeight));
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


    playersResetPosition.setBounds(playlistViewport.getWidth() - 200, 30, upDownButtonsWidth, upDownButtonsHeight);
    playersPreviousPosition.setBounds(playlistViewport.getWidth() - 300, 30, upDownButtonsWidth, upDownButtonsHeight);
    playersNextPosition.setBounds(playlistViewport.getWidth() - 100, 30, upDownButtonsWidth, upDownButtonsHeight);

    addPlayerPlaylist.setBounds(playlistViewport.getWidth() - 200, playlistViewport.getHeight() + playersStartHeightPosition + 5, 100, 25);
    removePlayerPlaylist.setBounds(playlistViewport.getWidth() - 100, playlistViewport.getHeight() + playersStartHeightPosition + 5, 100, 25);

    addPlayerCart.setBounds(playlistbisViewport.getX(), playlistbisViewport.getHeight() + playersStartHeightPosition + 5, 100, 25);
    removePlayerCart.setBounds(playlistbisViewport.getX() + 100, playlistbisViewport.getHeight() + playersStartHeightPosition + 5, 100, 25);

    int cueMeterXStart = playlistViewport.getWidth() + dragZoneWidth + 10;

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
        meter.setBounds(playlistViewport.getWidth() + dragZoneWidth + 10, getHeight() - levelMeterHeight, 80, std::min(getHeight() - playersStartHeightPosition, levelMeterHeight));
    }



    timeLabel.setBounds(getParentWidth() - 220, 0, 200, 50);


}


void MainComponent::timerCallback()
{
    juce::Time* time = new juce::Time(time->getCurrentTime());
    timeLabel.setText(time->toString(false, true, true, true), juce::NotificationType::dontSendNotification);
    //myPlaylists[0]->currentModifiers = juce::ComponentPeer::getCurrentModifiersRealtime();
    if (oscConnected)
    {
        if (myPlaylists[0]->players[myPlaylists[0]->fader1Player] != nullptr)
        {
            OSCsend(1, myPlaylists[0]->players[myPlaylists[0]->fader1Player]->remainingTimeString);
            sender.send("/1/gain1", juce::String(myPlaylists[0]->players[myPlaylists[0]->fader1Player]->volumeLabel.getText() + "dB"));
            delta1 = juce::Time::currentTimeMillis() - start1;
            if (delta1 > deltaTime && myPlaylists[0]->players[myPlaylists[0]->fader1Player]->isPlaying)
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
            if (delta2 > deltaTime && myPlaylists[0]->players[myPlaylists[0]->fader2Player]->isPlaying)
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
            if (delta3 > deltaTime && myPlaylists[1]->players[myPlaylists[1]->fader1Player]->isPlaying)
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
            if (delta4 > deltaTime && myPlaylists[1]->players[myPlaylists[1]->fader2Player]->isPlaying)
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
    if (midiMessageNumber == 44 && midiMessageValue == 127)
    {
        myPlaylists[0]->playersNextPositionClicked();
    }
    if (midiMessageNumber == 42 && midiMessageValue == 127)
    {
        myPlaylists[0]->playersResetPositionClicked();
    }

    const juce::ScopedValueSetter<bool> scopedInputFlag(isAddingFromMidiInput, true);
 
}





bool MainComponent::keyPressed(const juce::KeyPress &key, juce::Component* originatingComponent)
{
    if (key == 73)
    {
        if (myPlaylists[draggedPlaylist]->players[draggedPlayer] != nullptr)
        {
            myPlaylists[draggedPlaylist]->players[draggedPlayer]->setStart();
        }
    }
    else if (key == 75)
    {
        if (myPlaylists[draggedPlaylist]->players[draggedPlayer] != nullptr)
        {
            myPlaylists[draggedPlaylist]->players[draggedPlayer]->deleteStart();
        }
    }
    else if (key == 79)
    {
        if (myPlaylists[draggedPlaylist]->players[draggedPlayer] != nullptr)
        {
            myPlaylists[draggedPlaylist]->players[draggedPlayer]->setStop();
        }
    }
    else if (key == 76)
    {
        if (myPlaylists[draggedPlaylist]->players[draggedPlayer] != nullptr)
        {
            myPlaylists[draggedPlaylist]->players[draggedPlayer]->deleteStop();
        }
    }
    else if (key == 67)
    {
        if (myPlaylists[draggedPlaylist]->players[draggedPlayer] != nullptr)
        {
            myPlaylists[draggedPlaylist]->players[draggedPlayer]->cueButtonClicked();
        }
    }
    else if (key == juce::KeyPress::rightKey)
        copyPlayingSound();
    else if (myPlaylists[0] != nullptr)
    {
        myPlaylists[0]->keyPressed(key, originatingComponent);
        myPlaylists[1]->keyPressed(key, originatingComponent);
    }



    return false;
}

void MainComponent::savePlaylist()
{
    juce::XmlElement* playlist = new juce::XmlElement("PLAYLIST");

    for (int i = 0; i < myPlaylists[0]->players.size(); ++i)
    {
        if (myPlaylists[0]->players[i]->fileLoaded == true)
        {
            juce::XmlElement* player = new juce::XmlElement("PLAYER");
            player->setAttribute("ID", i);
            player->setAttribute("path", myPlaylists[0]->players[i]->getFilePath());
            player->setAttribute("trimvolume", myPlaylists[0]->players[i]->getTrimVolume());
            player->setAttribute("islooping", myPlaylists[0]->players[i]->getIsLooping());
            player->setAttribute("Name", juce::String(myPlaylists[0]->players[i]->getName()));
            player->setAttribute("isHpfEnabled", myPlaylists[0]->players[i]->isHpfEnabled());
            bool isStartTimeSet = myPlaylists[0]->players[i]->startTimeSet;
            player->setAttribute("isStartTimeSet", isStartTimeSet);
            if (isStartTimeSet)
                player->setAttribute("startTime", myPlaylists[0]->players[i]->getStart());
            bool isStopTimeSet = myPlaylists[0]->players[i]->stopTimeSet;
            player->setAttribute("isStopTimeSet", isStopTimeSet);
            if (isStopTimeSet)
                player->setAttribute("stopTime", myPlaylists[0]->players[i]->getStop());
            playlist->addChildElement(player);
        }
    }

    juce::XmlElement* cart = new juce::XmlElement("CART");

    for (int i = 0; i < myPlaylists[1]->players.size(); ++i)
    {
        if (myPlaylists[1]->players[i]->fileLoaded == true)
        {
            juce::XmlElement* player = new juce::XmlElement("PLAYER");
            player->setAttribute("ID", i);
            player->setAttribute("path", myPlaylists[1]->players[i]->getFilePath());
            player->setAttribute("trimvolume", myPlaylists[1]->players[i]->getTrimVolume());
            player->setAttribute("islooping", myPlaylists[1]->players[i]->getIsLooping());
            player->setAttribute("Name", myPlaylists[1]->players[i]->getName());
            player->setAttribute("isHpfEnabled", myPlaylists[1]->players[i]->isHpfEnabled());
            bool isStartTimeSet = myPlaylists[1]->players[i]->startTimeSet;
            player->setAttribute("isStartTimeSet", isStartTimeSet);
            if (isStartTimeSet)
                player->setAttribute("startTime", myPlaylists[1]->players[i]->getStart());
            bool isStopTimeSet = myPlaylists[1]->players[i]->stopTimeSet;
            player->setAttribute("isStopTimeSet", isStopTimeSet);
            if (isStopTimeSet)
                player->setAttribute("stopTime", myPlaylists[1]->players[i]->getStop());
            cart->addChildElement(player);
        }
    }


    juce::XmlElement multiPlayer("MULTIPLAYER");

    multiPlayer.addChildElement(playlist);
    multiPlayer.addChildElement(cart);


    auto xmlString = multiPlayer.toString();

    juce::FileChooser chooser("Choose an XML file", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*.xml");

    if (chooser.browseForFileToSave(true))
    {
        juce::File myPlaylistSave;
        myPlaylistSave = chooser.getResult();
        multiPlayer.writeTo(myPlaylistSave);
    }
}

void MainComponent::loadPlaylist()
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
        juce::FileChooser chooser("Choose an XML File", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*.xml");

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
                DBG(numberOfPlayersToLoad);

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
                                    if (myPlaylists[0]->players[iLoadedPlayer] == nullptr)
                                    {
                                        myPlaylists[0]->addPlayer(iLoadedPlayer - 1);
                                    }
                                    myPlaylists[0]->players[iLoadedPlayer]->verifyAudioFileFormat(filePath);
                                    myPlaylists[0]->players[iLoadedPlayer]->setTrimVolume(trimvolume);
                                    myPlaylists[0]->players[iLoadedPlayer]->setIsLooping(islooping);
                                    myPlaylists[0]->players[iLoadedPlayer]->setName(name);
                                    myPlaylists[0]->players[iLoadedPlayer]->enableHPF(hpfEnabled);
                                    if (isStartTimeSet)
                                        myPlaylists[0]->players[iLoadedPlayer]->setStartTime(startTime);
                                    if (isStopTimeSet)
                                        myPlaylists[0]->players[iLoadedPlayer]->setStopTime(stopTime);
                                    iLoadedPlayer++;
                                    loaddedPlayers++;
                                    loadingProgress = loaddedPlayers / numberOfPlayersToLoad;
                                    
                                    progressLabel->setText(juce::String(loadingProgress), juce::NotificationType::dontSendNotification);
                                    DBG(loadingProgress);
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
                                    if (myPlaylists[1]->players[iLoadedPlayer] == nullptr)
                                    {
                                        myPlaylists[1]->addPlayer(iLoadedPlayer - 1);
                                    }
                                    myPlaylists[1]->players[iLoadedPlayer]->verifyAudioFileFormat(filePath);
                                    myPlaylists[1]->players[iLoadedPlayer]->setTrimVolume(trimvolume);
                                    myPlaylists[1]->players[iLoadedPlayer]->setIsLooping(islooping);
                                    myPlaylists[1]->players[iLoadedPlayer]->setName(name);
                                    myPlaylists[1]->players[iLoadedPlayer]->enableHPF(hpfEnabled);
                                    if (isStartTimeSet)
                                        myPlaylists[1]->players[iLoadedPlayer]->setStartTime(startTime);
                                    if (isStopTimeSet)
                                        myPlaylists[1]->players[iLoadedPlayer]->setStopTime(stopTime);

                                    iLoadedPlayer++;
                                    myPlaylists[1]->assignLeftFader(0);
                                    myPlaylists[1]->assignRightFader(1);
                                    loaddedPlayers++;
                                    loadingProgress = loaddedPlayers / numberOfPlayersToLoad;
                                    DBG(loadingProgress);
                                }
                            }
                        }
                    }
                }
            }
            //if (myPlaylists[0]->players.size() == 0)
            //    myPlaylists[0]->addPlayer(0);
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

        }
    }
}

void MainComponent::setOptions(juce::String FFmpegPath, juce::String convertedSoundFolder, float skewFactor)
{

}

void MainComponent::valueChanged(juce::Value& value)
{
    if (value.refersToSameSourceAs(myPlaylists[0]->minimumPlayer))
    {
        playlistViewport.setViewPosition(0, (((myPlaylists[0]->minimumPlayer).toString().getIntValue()) - 1) * 105);
    }
    else if (value.refersToSameSourceAs(Settings::audioOutputModeValue))
    {
        channelsMapping();
        metersInitialize();
    }
    else if (value.refersToSameSourceAs(myPlaylists[0]->fader1Value))
    {
        OSCsend(1, value.toString().getFloatValue());
    }
    else if (value.refersToSameSourceAs(myPlaylists[0]->playerStoppedID)) //playlist 1
    {
        if (value.toString().getIntValue() == myPlaylists[0]->fader1Player)
        {
            //DBG(value.toString().getIntValue());
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
            myPlaylists[1]->handleFader3(0);

        }
        else if (value.toString().getIntValue() == myPlaylists[1]->fader2Player)
        {
            OSCsend(4, 0.);
            myPlaylists[1]->handleFader4(0);
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
        draggedPlayer = myPlaylists[0]->draggedPlayer.toString().getIntValue();
        draggedPlaylist = 0;
        myPlaylists[1]->draggedPlayer = -1;
    }
    else if (value.refersToSameSourceAs(myPlaylists[1]->draggedPlayer))
    {
        draggedPlayer = myPlaylists[1]->draggedPlayer.toString().getIntValue();
        draggedPlaylist = 1;
        myPlaylists[0]->draggedPlayer = -1;
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
    metersInitialize();

    myPlaylists[0]->setOptions();
    myPlaylists[1]->setOptions();
   // std::unique_ptr<Settings> settingsss = std::make_unique<Settings>();
}


void MainComponent::mouseDown(const juce::MouseEvent& event)
{

    //DBG("mouse dragged");
    //DBG(getMouseXYRelative().getY());
    //for (auto i = 0; i < myPlaylists[0]->players.size(); i++)
    //{
    //    if ((getMouseXYRelative().getX() < dragZoneWidth)
    //        && (getMouseXYRelative().getY() > (playlistViewport.getPosition().getY() + (105 * i))
    //            && (getMouseXYRelative().getY() < (playlistViewport.getPosition().getY() + (105 * (i + 1))))))
    //    {
    //        playerMouseDrag = i;
    //    }
    //}
}

void MainComponent::mouseDragGetInfos(int playlistSource, int playerID)
{
    if (myPlaylists[playlistSource]->players[playerID] != nullptr)
    {
        draggedPath = myPlaylists[playlistSource]->players[playerID]->getFilePath();
        draggedTrim = myPlaylists[playlistSource]->players[playerID]->getTrimVolume();
        draggedName = myPlaylists[playlistSource]->players[playerID]->getName();
        draggedHpfEnabled = myPlaylists[playlistSource]->players[playerID]->isHpfEnabled();
        draggedStartTimeSet = myPlaylists[playlistSource]->players[playerID]->startTimeSet;
        draggedStopTimeSet = myPlaylists[playlistSource]->players[playerID]->stopTimeSet;
        draggedStartTime = myPlaylists[playlistSource]->players[playerID]->getStart();
        draggedStopTime = myPlaylists[playlistSource]->players[playerID]->getStop();

        mouseDragged = true;
    }
}

void MainComponent::drawDragLines()
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
                myPlaylists[0]->fileDragPaintLine = true;
                myPlaylists[0]->fileDragPaintRectangle = false;
                myPlaylists[1]->fileDragPaintRectangle = false;
                myPlaylists[1]->fileDragPaintLine = false;
                myPlaylists[0]->fileDragPlayerDestination = playerMouseDragUp;
                myPlaylists[0]->repaint();
                destinationPlayerFound = true;
                insertTop = false;
            }
            else if (getMouseXYRelative().getY() < (playlistViewport.getPosition().getY() - playlistViewport.getViewPositionY() + dragZoneHeight)
                && getMouseXYRelative().getY() > (playlistViewport.getPosition().getY() - playlistViewport.getViewPositionY() - dragZoneHeight)
                && getMouseXYRelative().getX() < playlistViewport.getWidth()
                && (getMouseXYRelative().getX() > 0))
            {
                insertTop = true;
                playerMouseDragUp = -1;
                playlistDragDestination = 0;
                myPlaylists[0]->fileDragPaintLine = true;
                myPlaylists[0]->fileDragPaintRectangle = false;
                myPlaylists[1]->fileDragPaintRectangle = false;
                myPlaylists[1]->fileDragPaintLine = false;
                myPlaylists[0]->fileDragPlayerDestination = playerMouseDragUp;
                myPlaylists[0]->repaint();
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
                    myPlaylists[1]->fileDragPaintLine = true;
                    myPlaylists[1]->fileDragPaintRectangle = false;
                    myPlaylists[0]->fileDragPaintRectangle = false;
                    myPlaylists[0]->fileDragPaintLine = false;
                    myPlaylists[1]->fileDragPlayerDestination = playerMouseDragUp;
                    myPlaylists[1]->repaint();
                    destinationPlayerFound = true;
                    insertTop = false;
                }
                else if (getMouseXYRelative().getY() < (playlistbisViewport.getPosition().getY() - playlistbisViewport.getViewPositionY() + dragZoneHeight)
                    && getMouseXYRelative().getY() > (playlistbisViewport.getPosition().getY() - playlistbisViewport.getViewPositionY() - dragZoneHeight)
                    && getMouseXYRelative().getX() > playlistbisViewport.getPosition().getX()
                    && (getMouseXYRelative().getX() < getParentWidth()))
                {
                    insertTop = true;
                    playerMouseDragUp = -1;
                    playlistDragDestination = 1;
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

void MainComponent::mouseDragDefinePlayer()
{
    if (playlistDragDestination != -1)
            mouseDragSetInfos(playlistDragDestination, playerMouseDragUp);
    else
    {
        mouseDragEnd(0);
        mouseDragEnd(1);
    }

}

bool MainComponent::isDraggable(int playlistSource, int playerSource, int playlistDestination, int playerDestination)
{


        return true;


}

void MainComponent::mouseDragSetInfos(int playlistDestination, int playerIdDestination)
{
    if (myPlaylists[playlistDestination] != nullptr
        && myPlaylists[playlistDragSource] != nullptr)
    {
        if (insertTop == true)
        {
            if (isDraggable(playlistDragSource, playerDragSource, playlistDestination, playerIdDestination))
            {
                if (myPlaylists[playlistDestination] == myPlaylists[playlistDragSource])
                {
                    reassignFaders(playerIdDestination, playerDragSource, playlistDestination);
                    myPlaylists[playlistDestination]->addPlayer(playerIdDestination);
                    checkAndRemovePlayer(playlistDestination, playerDragSource + 1);
                    playerIdDestination++;
                    setSoundInfos(playlistDestination, playerIdDestination);
                }
                else
                {

                        myPlaylists[playlistDestination]->fader1Player++;
                        myPlaylists[playlistDestination]->fader2Player++;

                    myPlaylists[playlistDestination]->addPlayer(playerIdDestination);
                    playerIdDestination++;
                    setSoundInfos(playlistDestination, playerIdDestination);
                }
                clearDragInfos();
            }
        }
        if (myPlaylists[playlistDestination]->players[playerIdDestination] != myPlaylists[playlistDragSource]->players[playerDragSource])
        {
            if (isDraggable(playlistDragSource, playerDragSource, playlistDestination, playerIdDestination))
            {
                if (myPlaylists[playlistDestination]->players[playerIdDestination] != nullptr && mouseDragged)
                {
                    if (myPlaylists[playlistDestination] == myPlaylists[playlistDragSource]) //IF BOTH PLAYERS ARE ON THE SAME PLAYLIST
                    {
                        if (myPlaylists[playlistDestination]->fileDragPaintLine)
                        {
                            if (playlistDestination == 0)
                            {
                                reassignFaders(playerIdDestination, playerDragSource, playlistDestination);
                                if (playerIdDestination > playerDragSource)
                                {
                                    myPlaylists[playlistDestination]->addPlayer(playerIdDestination);
                                    checkAndRemovePlayer(playlistDestination, playerDragSource);
                                }
                                if (playerIdDestination < playerDragSource)
                                {
                                    myPlaylists[playlistDestination]->addPlayer(playerIdDestination);
                                    checkAndRemovePlayer(playlistDestination, playerDragSource + 1);
                                    playerIdDestination++;
                                }
                                setSoundInfos(playlistDestination, playerIdDestination);
                            }
                            else if (playlistDestination == 1)
                            {

                               /* if (playerIdDestination > playerDragSource)
                                {
                                    if (playerDragSource < myPlaylists[1]->fader1Player)
                                        myPlaylists[1]->fader1Player--;
                                    if (playerDragSource < myPlaylists[1]->fader2Player)
                                        myPlaylists[1]->fader2Player--;
                                    myPlaylists[playlistDestination]->addPlayer(playerIdDestination);
                                    checkAndRemovePlayer(playlistDestination, playerDragSource);
                                }
                                if (playerIdDestination < playerDragSource)
                                {
                                    if (playerIdDestination < myPlaylists[1]->fader1Player)
                                        myPlaylists[1]->fader1Player++;
                                    if (playerIdDestination < myPlaylists[1]->fader2Player)
                                        myPlaylists[1]->fader2Player++;

                                    myPlaylists[playlistDestination]->addPlayer(playerIdDestination);
                                    checkAndRemovePlayer(playlistDestination, playerDragSource + 1);
                                    playerIdDestination++;
                                }*/

                                reassignFaders(playerIdDestination, playerDragSource, playlistDestination);
                                if (playerIdDestination > playerDragSource)
                                {
                                    myPlaylists[playlistDestination]->addPlayer(playerIdDestination);
                                    checkAndRemovePlayer(playlistDestination, playerDragSource);
                                }
                                if (playerIdDestination < playerDragSource)
                                {
                                    myPlaylists[playlistDestination]->addPlayer(playerIdDestination);
                                    checkAndRemovePlayer(playlistDestination, playerDragSource + 1);
                                    playerIdDestination++;
                                }
                                setSoundInfos(playlistDestination, playerIdDestination);
                            }

                        }

                    }
                    else if (myPlaylists[playlistDestination] != myPlaylists[playlistDragSource]) //IF PLAYERS ARE ON DIFFERENTS PLAYLISTS
                    {
                        if (myPlaylists[playlistDestination]->fileDragPaintLine)
                        {
                            DBG("differents playlist");
                            if (playlistDestination == 0)
                            {
                                if (playerIdDestination < myPlaylists[playlistDestination]->fader1Player)
                                {
                                    myPlaylists[playlistDestination]->fader1Player++;
                                }
                                if (playerIdDestination < myPlaylists[playlistDestination]->fader2Player)
                                {
                                    myPlaylists[playlistDestination]->fader2Player++;
                                }
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
                            //myPlaylists[playlistDragSource]->removePlayer(playerDragSource);
                            checkAndRemovePlayer(playlistDragSource, playerDragSource);
                    }
                    clearDragInfos();
                }
            }
        }
    }
    mouseDragEnd(playlistDestination);
}

void MainComponent::setSoundInfos(int playlistDestination, int playerIdDestination)
{
    myPlaylists[playlistDestination]->players[playerIdDestination]->loadFile(draggedPath);
    myPlaylists[playlistDestination]->players[playerIdDestination]->setTrimVolume(draggedTrim);
    myPlaylists[playlistDestination]->players[playerIdDestination]->setName(draggedName);
    myPlaylists[playlistDestination]->players[playerIdDestination]->enableHPF(draggedHpfEnabled);
    if (draggedStartTimeSet)
        myPlaylists[playlistDestination]->players[playerIdDestination]->setStartTime(draggedStartTime);
    if (draggedStopTimeSet)
        myPlaylists[playlistDestination]->players[playerIdDestination]->setStopTime(draggedStopTime);
}

void MainComponent::clearDragInfos()
{
    draggedPath = "";
    draggedName = "";
    draggedTrim = 0.0;
    draggedHpfEnabled = false;
    draggedStartTimeSet = false;
    draggedStopTimeSet = false;
    draggedStartTime = 0.0;
    draggedStopTime = 0.0;
    mouseDragged = false;
    myPlaylists[playlistDragSource]->mouseDragSource.setValue(-1);
}

void MainComponent::mouseDragEnd(int playlistDestination)
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

void MainComponent::checkAndRemovePlayer(int playlist, int player)
{
    if (myPlaylists[playlist] != nullptr)
    {
        if (myPlaylists[playlist]->players.size() > 1)
            myPlaylists[playlist]->removePlayer(player);
        else
            myPlaylists[playlist]->players[player]->deleteFile();
    }
}


void MainComponent::reassignFaders(int playerIdDestination, int playerDragSource, int playlistDestination)
{
if (playerIdDestination > playerDragSource)
{
    if (playerDragSource < myPlaylists[playlistDestination]->fader1Player
        && playerIdDestination >= myPlaylists[playlistDestination]->fader1Player)
        myPlaylists[playlistDestination]->fader1Player--;
    if (playerDragSource < myPlaylists[playlistDestination]->fader2Player
        && playerIdDestination >= myPlaylists[playlistDestination]->fader2Player)
        myPlaylists[playlistDestination]->fader2Player--;
    /*  if (playerIdDestination > std::min(myPlaylists[playlistDestination]->fader1Player, myPlaylists[playlistDestination]->fader2Player)
          && playerDragSource < std::min(myPlaylists[playlistDestination]->fader1Player, myPlaylists[playlistDestination]->fader2Player))
      {
          myPlaylists[playlistDestination]->fader1Player--;
          myPlaylists[playlistDestination]->fader2Player--;
          myPlaylists[playlistDestination]->spaceBarPlayerId--;
      }*/
}
else if (playerIdDestination < playerDragSource)
{

    if (playerDragSource == myPlaylists[playlistDestination]->fader1Player
        && myPlaylists[playlistDestination]->fader1IsPlaying)
    {
        myPlaylists[playlistDestination]->fader1Player = playerIdDestination + 1;
    }
    else if (playerDragSource == myPlaylists[playlistDestination]->fader2Player
        && myPlaylists[playlistDestination]->fader2IsPlaying)
    {
        myPlaylists[playlistDestination]->fader2Player = playerIdDestination + 1;
    }
    else if (playerIdDestination < std::min(myPlaylists[playlistDestination]->fader1Player, myPlaylists[playlistDestination]->fader2Player)
        && playerDragSource < std::min(myPlaylists[playlistDestination]->fader1Player, myPlaylists[playlistDestination]->fader2Player))
    {

    }

    if (playerIdDestination < myPlaylists[playlistDestination]->fader1Player
        && playerDragSource < myPlaylists[playlistDestination]->fader1Player)
    { }
    else if (playerIdDestination < myPlaylists[playlistDestination]->fader1Player)
    {
        myPlaylists[playlistDestination]->fader1Player++;
        if (!myPlaylists[playlistDestination]->fader1IsPlaying && playerIdDestination >= myPlaylists[playlistDestination]->fader2Player)
            myPlaylists[playlistDestination]->fader1Player--;

    }
    if (playerIdDestination < myPlaylists[playlistDestination]->fader2Player
        && playerDragSource < myPlaylists[playlistDestination]->fader2Player)
    {
    }
    else if (playerIdDestination < myPlaylists[playlistDestination]->fader2Player)
    {
        myPlaylists[playlistDestination]->fader2Player++;
        if (!myPlaylists[playlistDestination]->fader2IsPlaying && playerIdDestination >= myPlaylists[playlistDestination]->fader1Player)
            myPlaylists[playlistDestination]->fader2Player--;
    }
}

}

void MainComponent::copyPlayingSound()
{
    DBG("copy");
    if (myPlaylists[0]->fader1IsPlaying)
    {
        if (myPlaylists[0]->players[myPlaylists[0]->fader1Player] != nullptr)
        {
            myPlaylists[1]->addPlayer(myPlaylists[1]->players.size() - 1);
            myPlaylists[1]->players.getLast()->loadFile(myPlaylists[0]->players[myPlaylists[0]->fader1Player]->getFilePath());
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
            myPlaylists[1]->players.getLast()->loadFile(myPlaylists[0]->players[myPlaylists[0]->fader2Player]->getFilePath());
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
            myPlaylists[1]->players.getLast()->loadFile(myPlaylists[0]->players[myPlaylists[0]->spaceBarPlayerId]->getFilePath());
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

void MainComponent::channelsMapping()
{

    myMixer.removeAllInputs();
    myCueMixer.removeAllInputs();

    if (Settings::audioOutputMode == 1)
    {
        tryPreferedAudioDevice(2);
        setAudioChannels(0, 2);
        myMixer.addInputSource(&myPlaylists[0]->playlistMixer, false);
        myCueMixer.addInputSource(&myPlaylists[0]->playlistCueMixer, false);
        myMixer.addInputSource(&myPlaylists[1]->playlistMixer, false);
        myCueMixer.addInputSource(&myPlaylists[1]->playlistCueMixer, false);
    }
    else if (Settings::audioOutputMode == 2)
    {
        tryPreferedAudioDevice(2);
        setAudioChannels(0, 2);
        myMixer.addInputSource(&myPlaylists[0]->playlistMixer, false);
        myMixer.addInputSource(&myPlaylists[0]->playlistCueMixer, false);
        myMixer.addInputSource(&myPlaylists[1]->playlistMixer, false);
        myMixer.addInputSource(&myPlaylists[1]->playlistCueMixer, false);
    }
    else if (Settings::audioOutputMode == 3)
    {
        tryPreferedAudioDevice(4);
        setAudioChannels(0, 4);
        myMixer.addInputSource(&myPlaylists[0]->playlistMixer, false);
        myCueMixer.addInputSource(&myPlaylists[0]->playlistCueMixer, false);
        myMixer.addInputSource(&myPlaylists[1]->playlistMixer, false);
        myCueMixer.addInputSource(&myPlaylists[1]->playlistCueMixer, false);
    }
}

void MainComponent::metersInitialize()
{
    lnf.setColour(foleys::LevelMeter::lmBackgroundColour, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    lnf.setColour(foleys::LevelMeter::lmMeterGradientLowColour, juce::Colours::green);
    lnf.setColour(foleys::LevelMeter::lmMeterGradientMaxColour, juce::Colours::red);
    lnf.setColour(foleys::LevelMeter::lmMeterGradientMidColour, juce::Colours::orange);


    if (Settings::audioOutputMode == 1)
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
    else if (Settings::audioOutputMode == 2)
    {
        meter.setLookAndFeel(&lnf);
        meter.setMeterSource(&meterSource);
        addAndMakeVisible(meter);
        meter.setMeterFlags(foleys::LevelMeter::MeterFlags::Default);
        removeChildComponent(&cuemeter);
    }

    else if (Settings::audioOutputMode == 3)
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


    int cueMeterXStart = playlistViewport.getWidth() + dragZoneWidth + 10;
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
        meter.setBounds(playlistViewport.getWidth() + dragZoneWidth + 10, getHeight() - levelMeterHeight, 80, std::min(getHeight() - playersStartHeightPosition, levelMeterHeight));
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
                    deviceManager.initialise(0, outputChannelsNeeded, nullptr, true, Settings::preferedAudioDeviceName);
                    if (deviceManager.getCurrentAudioDevice() != nullptr)
                        Settings::outputChannelsNumber = deviceManager.getCurrentAudioDevice()->getOutputChannelNames().size();
                }
            }
        }
    }
    else
        deviceManager.initialise(0, outputChannelsNeeded, nullptr, true);

    if (deviceManager.getCurrentAudioDevice() != nullptr)
        Settings::outputChannelsNumber = deviceManager.getCurrentAudioDevice()->getOutputChannelNames().size();
}