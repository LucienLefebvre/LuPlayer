/*
  ==============================================================================

    Settings.cpp
    Created: 12 Feb 2021 10:47:21am
    Author:  Lucien

  ==============================================================================
*/

#include <JuceHeader.h>
#include "Settings.h"

float Settings::maxFaderValueGlobal;
juce::Value Settings::maxFaderValue;
float Settings::skewFactorGlobal;
int Settings::midiShift;
int Settings::faderTempTime;
int Settings::preferedMidiDeviceIndex;
int Settings::inOscPort;
int Settings::outOscPort;
juce::String Settings::adress1;
juce::String Settings::adress2;
juce::String Settings::adress3;
juce::String Settings::adress4;
juce::String Settings::ipAdress;
juce::String Settings::FFmpegPath;
juce::String Settings::exiftoolPath;
juce::String Settings::convertedSoundsPath;
int Settings::audioOutputMode;
juce::Value Settings::audioOutputModeValue;
juce::String Settings::preferedAudioDeviceType;
juce::String Settings::preferedAudioDeviceName;
juce::AudioDeviceManager::AudioDeviceSetup Settings::preferedAudioDevice;
juce::String Settings::outputDeviceName;
juce::String Settings::inputDeviceName;
double Settings::sampleRate;
int Settings::bufferSize;
juce::BigInteger Settings::inputChannels;
bool Settings::useDefaultInputChannels;
juce::BigInteger Settings::outputChannels;
bool Settings::useDefaultOutputsChannels;
bool Settings::lauchAtZeroDB;
bool Settings::mouseWheelControlVolume;
bool Settings::autoNormalize;

int Settings::outputChannelsNumber;
juce::Value Settings::sampleRateValue;

juce::StringArray Settings::tempFiles;

int Settings::draggedPlaylist;
int Settings::draggedPlayer;
//==============================================================================
Settings::Settings() : settingsFile(options)
{
    //OPTIONS
    options.applicationName = ProjectInfo::projectName;
    options.filenameSuffix = ".settings";

    options.folderName = juce::File::getCurrentWorkingDirectory().getFullPathName();
    //options.folderName = juce::File::getSpecialLocation(juce::File::SpecialLocationType::userApplicationDataDirectory).getChildFile("MultiPlayer").getFullPathName();
    options.storageFormat = juce::PropertiesFile::storeAsXML;
    properties.setStorageParameters(options);



    //LOAD PROPERTIES
    Settings::FFmpegPath = properties.getUserSettings()->getValue("FFmpeg Path");
    Settings::exiftoolPath = properties.getUserSettings()->getValue("Exiftool Path");
    Settings::convertedSoundsPath = properties.getUserSettings()->getValue("Converted Sound Path");

    if (properties.getUserSettings()->getValue("Skew Factor").isEmpty())
        Settings::skewFactorGlobal = 0.5;
    else
    Settings::skewFactorGlobal = properties.getUserSettings()->getValue("Skew Factor").getFloatValue();

    if (properties.getUserSettings()->getValue("Max Fader Value").isEmpty())
        Settings::maxFaderValueGlobal = 0;
    else
        Settings::maxFaderValueGlobal = properties.getUserSettings()->getValue("Max Fader Value").getFloatValue();
    Settings::maxFaderValue = Settings::maxFaderValueGlobal;

    if (properties.getUserSettings()->getValue("Audio Output Mode").isEmpty())
        Settings::audioOutputMode = 2;
    else
        Settings::audioOutputMode = properties.getUserSettings()->getValue("Audio Output Mode").getIntValue();
    audioOutputModeValue = Settings::audioOutputMode;

    if (properties.getUserSettings()->getValue("Fader Temp").isEmpty())
        Settings::faderTempTime = 1000;
    else
        Settings::faderTempTime = properties.getUserSettings()->getValue("Fader Temp").getIntValue();

    Settings::preferedMidiDeviceIndex = properties.getUserSettings()->getValue("Midi Device").getIntValue();
    Settings::midiShift = properties.getUserSettings()->getValue("Midi Shift").getIntValue();
    Settings::adress1 = properties.getUserSettings()->getValue("IP Adress 1");
    Settings::adress2 = properties.getUserSettings()->getValue("IP Adress 2");
    Settings::adress3 = properties.getUserSettings()->getValue("IP Adress 3");
    Settings::adress4 = properties.getUserSettings()->getValue("IP Adress 4");
    Settings::ipAdress = properties.getUserSettings()->getValue("IP Adress");
    Settings::inOscPort = properties.getUserSettings()->getValue("In OSC Port").getIntValue();
    Settings::outOscPort = properties.getUserSettings()->getValue("Out OSC Port").getIntValue();
    Settings::preferedAudioDeviceType = properties.getUserSettings()->getValue("Audio Device Type");
    Settings::preferedAudioDeviceName = properties.getUserSettings()->getValue("audioDeviceName");

    if (properties.getUserSettings()->getValue("Launchatzero").isEmpty())
        Settings::lauchAtZeroDB = true;
    else
    Settings::lauchAtZeroDB = properties.getUserSettings()->getValue("Launchatzero").getIntValue();

    if (properties.getUserSettings()->getValue("MouseWheelControl").isEmpty())
        Settings::mouseWheelControlVolume = true;
    else
        Settings::mouseWheelControlVolume = properties.getUserSettings()->getValue("MouseWheelControl").getIntValue();

    if (properties.getUserSettings()->getValue("AutoNormalize").isEmpty())
        Settings::autoNormalize = true;
    else
        Settings::autoNormalize = properties.getUserSettings()->getValue("AutoNormalize").getIntValue();

    //SAVE & CLOSE BUTTon
    saveButton.setBounds(250, 400, 100, 50);
    addAndMakeVisible(saveButton);
    saveButton.setButtonText("Save & Close");
    saveButton.onClick = [this] { setOptions();
                                    makeIpAdress(); saveOptions(); };
    

    //MAX FADER LEVEL
    maxFaderValueSlider.setBounds(150, 0, 450, 25);
    maxFaderValueSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    maxFaderValueSlider.setRange(0., 12., 1.);
    maxFaderValueSlider.addListener(this);
    maxFaderValueSlider.setValue(Settings::maxFaderValueGlobal);
    addAndMakeVisible(maxFaderValueSlider);
    maxFaderValueSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 50, 100);

    

    maxFaderValueLabel.setBounds(0, 0, 150, 25);
    addAndMakeVisible(maxFaderValueLabel);
    maxFaderValueLabel.setText("Fader Maximum Value", juce::NotificationType::dontSendNotification);

    //SkEW FACTOR
    skewFactorSlider.setBounds(200, 50, 300, 25);
    skewFactorSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    skewFactorSlider.setRange(0.1, 1.0, 0.1);
    skewFactorSlider.addListener(this);
    skewFactorSlider.setValue(Settings::skewFactorGlobal);
    skewFactorSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(skewFactorSlider);


    skewFactorLabel.setBounds(0, 50, 130, 25);
    addAndMakeVisible(skewFactorLabel);
    skewFactorLabel.setText("Fader Acceleration", juce::NotificationType::dontSendNotification);
    skewFactorLabelLeft.setBounds(130, 50, 70, 25);
    addAndMakeVisible(skewFactorLabelLeft);
    skewFactorLabelLeft.setText("Logarithmic", juce::NotificationType::dontSendNotification);
    skewFactorLabelLeft.setJustificationType(juce::Justification::right);
    skewFactorLabelRight.setBounds(500, 50, 100, 25);
    addAndMakeVisible(skewFactorLabelRight);
    skewFactorLabelRight.setText("Linear", juce::NotificationType::dontSendNotification);

    //FMPEG PATH
    ffmpegPathLabel.setBounds(200, 300, 400, 25);
    //addAndMakeVisible(ffmpegPathLabel);

    selectFFmpegButton.setBounds(0, 300, 200, 25);
    selectFFmpegButton.setButtonText("Select FFmpeg.exe");
    //addAndMakeVisible(selectFFmpegButton);
    selectFFmpegButton.onClick = [this] { selectFFmpeg(); };
    ffmpegPathLabel.setText(Settings::FFmpegPath, juce::NotificationType::dontSendNotification);

    //EXIFTOOL PATH
    exiftoolLabel.setBounds(200, 350, 400, 25);
    //addAndMakeVisible(exiftoolLabel);

    selectExiftoolButton.setBounds(0, 350, 200, 25);
    selectExiftoolButton.setButtonText("Select Exiftool.exe");
    //addAndMakeVisible(selectExiftoolButton);
    selectExiftoolButton.onClick = [this] { selectExiftool(); };
    exiftoolLabel.setText(Settings::exiftoolPath, juce::NotificationType::dontSendNotification);

    //CONVERTED SOUNDS PATH 
    convertedSoundsLabel.setBounds(200, 300, 400, 25);
    addAndMakeVisible(convertedSoundsLabel);
    convertedSoundsLabel.setText(Settings::convertedSoundsPath, juce::NotificationType::dontSendNotification);

    convertedSoundsButtons.setBounds(1, 300, 199, 25);
    convertedSoundsButtons.setButtonText("Select converted sounds folder");
    addAndMakeVisible(convertedSoundsButtons);
    convertedSoundsButtons.onClick = [this] { selectSoundsFolder(); };

    //MIDI SHIFT
    midiShiftLabel.setBounds(0, 100, 200, 25);
    midiShiftLabel.setText("Midi Channel Shift", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(midiShiftLabel);
    midiShiftValue.setBounds(200, 100, 25, 25);
    midiShiftValue.setText(juce::String(Settings::midiShift), juce::NotificationType::dontSendNotification);
    addAndMakeVisible(midiShiftValue);
    midiShiftValue.setEditable(true, true, false);
    midiShiftValue.addListener(this);
    midiShiftValue.setJustificationType(juce::Justification::centred);
    midiShiftValue.setColour(juce::Label::outlineColourId, juce::Colours::black);

    //FADER TEMP
    faderTempLabel.setBounds(300, 100, 200, 25);
    faderTempLabel.setText("Fader Temporisation (ms)", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(faderTempLabel);
    faderTempValue.setBounds(500, 100, 50, 25);
    faderTempValue.setText(juce::String(Settings::faderTempTime), juce::NotificationType::dontSendNotification);
    addAndMakeVisible(faderTempValue);
    faderTempValue.setEditable(true, true, false);
    faderTempValue.addListener(this);
    faderTempValue.setJustificationType(juce::Justification::centred);
    faderTempValue.setColour(juce::Label::outlineColourId, juce::Colours::black);


    //OSC Ports
    addAndMakeVisible(oscPorts);
    oscPorts.setText("OSC", juce::NotificationType::dontSendNotification);
    oscPorts.setBounds(0, 150, 100, 25);

    //OUT
    addAndMakeVisible(oscOutPortLabel);
    oscOutPortLabel.setText("Outgoing", juce::NotificationType::dontSendNotification);
    oscOutPortLabel.setBounds(50, 150, 100, 25);

    addAndMakeVisible(oscOutPort);
    oscOutPort.setText(juce::String(Settings::outOscPort), juce::NotificationType::dontSendNotification);
    oscOutPort.setBounds(125, 150, 50, 25);
    oscOutPort.setEditable(true, true, false);
    oscOutPort.addListener(this);
    oscOutPort.setJustificationType(juce::Justification::centred);
    oscOutPort.setColour(juce::Label::outlineColourId, juce::Colours::black);

    //IN
    addAndMakeVisible(oscInPortLabel);
    oscInPortLabel.setText("Incoming", juce::NotificationType::dontSendNotification);
    oscInPortLabel.setBounds(175, 150, 100, 25);

    addAndMakeVisible(oscInPort);
    oscInPort.setText(juce::String(Settings::inOscPort), juce::NotificationType::dontSendNotification);
    oscInPort.setBounds(250, 150, 50, 25);
    oscInPort.setEditable(true, true, false);
    oscInPort.addListener(this);
    oscInPort.setJustificationType(juce::Justification::centred);
    oscInPort.setColour(juce::Label::outlineColourId, juce::Colours::black);

    //OSC IP Adress Destination
    addAndMakeVisible(ipAdressLabel);
    ipAdressLabel.setBounds(300, 150, 50, 25);
    ipAdressLabel.setText("Ip", juce::NotificationType::dontSendNotification);

    addAndMakeVisible(ipAdress1);
    ipAdress1.setBounds(350, 150, 50, 25);
    ipAdress1.setText(Settings::adress1, juce::NotificationType::sendNotification);
    ipAdress1.setEditable(true, true, false);
    ipAdress1.addListener(this);
    ipAdress1.setJustificationType(juce::Justification::centred);
    ipAdress1.setColour(juce::Label::outlineColourId, juce::Colours::black);
    addAndMakeVisible(ipAdress2);
    ipAdress2.setBounds(400, 150, 50, 25);
    ipAdress2.setText(Settings::adress2, juce::NotificationType::sendNotification);
    ipAdress2.setEditable(true, true, false);
    ipAdress2.addListener(this);
    ipAdress2.setJustificationType(juce::Justification::centred);
    ipAdress2.setColour(juce::Label::outlineColourId, juce::Colours::black);
    addAndMakeVisible(ipAdress3);
    ipAdress3.setBounds(450, 150, 50, 25);
    ipAdress3.setText(Settings::adress3, juce::NotificationType::sendNotification);
    ipAdress3.setEditable(true, true, false);
    ipAdress3.addListener(this);
    ipAdress3.setJustificationType(juce::Justification::centred);
    ipAdress3.setColour(juce::Label::outlineColourId, juce::Colours::black);
    addAndMakeVisible(ipAdress4);
    ipAdress4.setBounds(500, 150, 50, 25);
    ipAdress4.setText(Settings::adress4, juce::NotificationType::sendNotification);
    ipAdress4.setEditable(true, true, false);
    ipAdress4.addListener(this);
    ipAdress4.setJustificationType(juce::Justification::centred);
    ipAdress4.setColour(juce::Label::outlineColourId, juce::Colours::black);

    addAndMakeVisible(&normalizeButton);
    normalizeButton.setButtonText("Auto normalize sounds at 0 LU");
    normalizeButton.setBounds(0, 200, 300, 25);
    normalizeButton.setToggleState(Settings::autoNormalize, juce::NotificationType::dontSendNotification);
    normalizeButton.addListener(this);

    addAndMakeVisible(&launchLevelButton);
    launchLevelButton.setButtonText("Always launch sounds at 0dB");
    launchLevelButton.setBounds(0, 250, 200, 25);
    launchLevelButton.setToggleState(Settings::lauchAtZeroDB, juce::NotificationType::dontSendNotification);
    launchLevelButton.addListener(this);

    addAndMakeVisible(&mouseWheelControlButton);
    mouseWheelControlButton.setButtonText("Mouse wheel control volume");
    mouseWheelControlButton.setBounds(200, 250, 200, 25);
    mouseWheelControlButton.setToggleState(Settings::mouseWheelControlVolume, juce::NotificationType::dontSendNotification);
    mouseWheelControlButton.addListener(this);

    //AUDIO OUTPUT MODE
    addAndMakeVisible(audioOutputModeLabel);
    audioOutputModeLabel.setBounds(0, 350, 200, 25);
    audioOutputModeLabel.setText("Audio Output Mode", juce::NotificationType::sendNotification);

    addAndMakeVisible(audioOutputModeListbox);
    audioOutputModeListbox.setBounds(200, 350, 399, 25);
    audioOutputModeListbox.addItem("Mono (Left -> Output, Right -> Cue)", 1);
    audioOutputModeListbox.addItem("Stereo 2 Outputs (Output & Cue on same stereo Output)", 2);
    audioOutputModeListbox.addItem("Stereo 4 Outputs(Output on first pair, Cue on second)", 3);
    audioOutputModeListbox.setSelectedId(Settings::audioOutputMode);
    if (Settings::outputChannelsNumber <= 4)
        audioOutputModeListbox.setItemEnabled(3, false);
    audioOutputModeListbox.addListener(this);



    makeIpAdress();
    setOptions();

    juce::Process::makeForegroundProcess();
    toFront(true);
    }

Settings::~Settings()
{

}

void Settings::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour(45, 56, 61));   // clear the background
    g.setColour(juce::Colour(40, 134, 189));
    g.drawRect(0, 0, 600, 470);

}

void Settings::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}

void Settings::selectFFmpeg()
{
    juce::FileChooser chooser("Select the FFmpeg Executable", juce::File::getSpecialLocation(juce::File::currentApplicationFile), "*FFmpeg.exe");
    if (chooser.browseForFileToOpen())
    {
        juce::File ffmpegExe;
        ffmpegExe = chooser.getResult();

        Settings::FFmpegPath = ffmpegExe.getFullPathName();
        properties.getUserSettings()->setValue("FFmpeg Path", Settings::FFmpegPath);
    }
    ffmpegPathLabel.setText(Settings::FFmpegPath, juce::NotificationType::dontSendNotification);
    setOptions();
    settingsFile.save();
    //saveOptions();
    //DBG(FFmpegPath);
}

juce::String Settings::getFFmpegPath()
{

    return FFmpegPath;
}

void Settings::selectExiftool()
{
    juce::FileChooser chooser("Select the Exiftool Executable", juce::File::getSpecialLocation(juce::File::currentApplicationFile), "*exiftool.exe");
    if (chooser.browseForFileToOpen())
    {
        juce::File exiftoolexe;
        exiftoolexe = chooser.getResult();

        Settings::exiftoolPath = exiftoolexe.getFullPathName();
        properties.getUserSettings()->setValue("Exiftool Path", Settings::exiftoolPath);
    }
    exiftoolLabel.setText(Settings::exiftoolPath, juce::NotificationType::dontSendNotification);

    //saveOptions();
    //DBG(FFmpegPath);
}

juce::String Settings::getExiftoolPath()
{

    return exiftoolPath;
}

void Settings::selectSoundsFolder()
{
    juce::FileChooser chooser("Select the Directory for converted sounds", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory));
    if (chooser.browseForDirectory())
    {
        juce::File SoundsPath;
        SoundsPath = chooser.getResult();
        
        Settings::convertedSoundsPath = SoundsPath.getFullPathName();
        properties.getUserSettings()->setValue("Converted Sound Path", Settings::convertedSoundsPath);
    }
    convertedSoundsLabel.setText(Settings::convertedSoundsPath, juce::NotificationType::dontSendNotification);
    properties.saveIfNeeded();
    settingsFile.save();
    //saveOptions();
    //DBG(FFmpegPath);
}

void Settings::setOptions()
{
}

void Settings::saveOptions()
{
    properties.saveIfNeeded();
    settingsFile.save();
    getParentComponent()->exitModalState(0);
}


void Settings::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &skewFactorSlider)
    {
        Settings::skewFactorGlobal = skewFactorSlider.getValue();
        properties.getUserSettings()->setValue("Skew Factor", Settings::skewFactorGlobal);
    }
    else if (slider == &maxFaderValueSlider)
    {
        Settings::maxFaderValueGlobal = maxFaderValueSlider.getValue();
        properties.getUserSettings()->setValue("Max Fader Value", Settings::maxFaderValueGlobal);
        Settings::maxFaderValue = maxFaderValueSlider.getValue();
    }
    properties.saveIfNeeded();
    settingsFile.save();
}

void Settings::setPreferedMidiDevice(int midiDeviceIndex)
{
    Settings::preferedMidiDeviceIndex = midiDeviceIndex;
    properties.getUserSettings()->setValue("Midi Device", Settings::preferedMidiDeviceIndex);
    properties.saveIfNeeded();
    settingsFile.save();
}

int Settings::getPreferedMidiDevice()
{
    return Settings::preferedMidiDeviceIndex;

}

void Settings::labelTextChanged(juce::Label* labelThatHasChanged)
{
    if (labelThatHasChanged == &midiShiftValue)
    {
        Settings::midiShift = (midiShiftValue.getTextValue()).toString().getIntValue();
        properties.getUserSettings()->setValue("Midi Shift", Settings::midiShift);
        makeIpAdress();
    }
    if (labelThatHasChanged == &faderTempValue)
    {
        Settings::faderTempTime = (faderTempValue.getTextValue()).toString().getIntValue();
        properties.getUserSettings()->setValue("Fader Temp", Settings::faderTempTime);
    }
    else if (labelThatHasChanged == &ipAdress1)
    {
        Settings::adress1 = ipAdress1.getText();
        properties.getUserSettings()->setValue("IP Adress 1", Settings::adress1);
        makeIpAdress();
    }
    else if (labelThatHasChanged == &ipAdress2)
    {
        Settings::adress2 = ipAdress2.getText();
        properties.getUserSettings()->setValue("IP Adress 2", Settings::adress2);
        makeIpAdress();
    }
    else if (labelThatHasChanged == &ipAdress3)
    {
        Settings::adress3 = ipAdress3.getText();
        properties.getUserSettings()->setValue("IP Adress 3", Settings::adress3);
        makeIpAdress();
    }
    else if (labelThatHasChanged == &ipAdress4)
    {
        Settings::adress4 = ipAdress4.getText();
        properties.getUserSettings()->setValue("IP Adress 4", Settings::adress4);
        makeIpAdress();
    }
    else if (labelThatHasChanged == &oscOutPort)
    {
        Settings::outOscPort = oscOutPort.getText().getIntValue();
        properties.getUserSettings()->setValue("Out OSC Port", Settings::outOscPort);
    }
    else if (labelThatHasChanged == &oscInPort)
    {
        Settings::inOscPort = oscInPort.getText().getIntValue();
        properties.getUserSettings()->setValue("In OSC Port", Settings::inOscPort);
    }
    properties.saveIfNeeded();
    settingsFile.save();
}

void Settings::makeIpAdress()
{
    Settings::ipAdress = juce::String(Settings::adress1 + "." + Settings::adress2 + "." + Settings::adress3 + "." + Settings::adress4);
    properties.getUserSettings()->setValue("IP Adress", Settings::ipAdress);
    properties.saveIfNeeded();
    settingsFile.save();
}

juce::String Settings::getIpAdress()
{

    return Settings::ipAdress;
}

int Settings::getOutPort()
{
    return Settings::outOscPort;
}

int Settings::getInPort()
{
    return Settings::inOscPort;
}


void Settings::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &audioOutputModeListbox)
    {

        Settings::audioOutputMode = audioOutputModeListbox.getSelectedId();
        Settings::audioOutputModeValue = audioOutputModeListbox.getSelectedId();
        properties.getUserSettings()->setValue("Audio Output Mode", Settings::audioOutputMode);
        properties.saveIfNeeded();
        settingsFile.save();
    }
}

void Settings::setPreferedAudioDeviceType(juce::String audioDeviceType)
{
    Settings::preferedAudioDeviceType = audioDeviceType;
    properties.getUserSettings()->setValue("Audio Device Type", Settings::preferedAudioDeviceType);
    properties.saveIfNeeded();
    settingsFile.save();
}
juce::String Settings::getPreferedAudioDeviceType()
{
    return Settings::preferedAudioDeviceType;
}
void Settings::setPreferedAudioDevice(juce::AudioDeviceManager::AudioDeviceSetup audioDevice)
{
    Settings::preferedAudioDevice = audioDevice;
    properties.getUserSettings()->setValue("outputDeviceName", Settings::preferedAudioDevice.outputDeviceName);
    properties.getUserSettings()->setValue("inputDeviceName", Settings::preferedAudioDevice.inputDeviceName);
    properties.getUserSettings()->setValue("sampleRate", Settings::preferedAudioDevice.sampleRate);
    properties.getUserSettings()->setValue("bufferSize", Settings::preferedAudioDevice.bufferSize);
    properties.getUserSettings()->setValue("inputChannels", Settings::preferedAudioDevice.inputChannels.toInt64());
    properties.getUserSettings()->setValue("useDefaultInputChannels", Settings::preferedAudioDevice.useDefaultInputChannels);
    properties.getUserSettings()->setValue("outputChannels", Settings::preferedAudioDevice.outputChannels.toInt64());
    properties.getUserSettings()->setValue("useDefaultOutputsChannels", Settings::preferedAudioDevice.useDefaultOutputChannels);
    properties.saveIfNeeded();
    settingsFile.save();
}
juce::AudioDeviceManager::AudioDeviceSetup Settings::getPreferedAudioDevice()
{
    Settings::preferedAudioDevice.outputDeviceName = properties.getUserSettings()->getValue("outputDeviceName").toStdString();
    Settings::preferedAudioDevice.inputDeviceName = properties.getUserSettings()->getValue("inputDeviceName");
    Settings::preferedAudioDevice.sampleRate = properties.getUserSettings()->getValue("sampleRate").getDoubleValue();
    Settings::preferedAudioDevice.bufferSize = properties.getUserSettings()->getValue("bufferSize").getIntValue();
    Settings::preferedAudioDevice.inputChannels = properties.getUserSettings()->getValue("inputChannels").getLargeIntValue();
    Settings::preferedAudioDevice.useDefaultInputChannels = properties.getUserSettings()->getValue("useDefaultInputChannels").getIntValue();
    Settings::preferedAudioDevice.outputChannels = properties.getUserSettings()->getValue("outputChannels").getLargeIntValue();
    Settings::preferedAudioDevice.useDefaultOutputChannels = properties.getUserSettings()->getValue("useDefaultOutputsChannels").getIntValue();



    return Settings::preferedAudioDevice;
}

void Settings::setPreferedAudioDeviceName(juce::String audioDeviceName)
{
    Settings::preferedAudioDeviceName = audioDeviceName;
    properties.getUserSettings()->setValue("audioDeviceName", Settings::preferedAudioDeviceName);
    properties.saveIfNeeded();
    settingsFile.save();
}

juce::String Settings::getPreferedAudioDeviceName()
{
    return Settings::preferedAudioDeviceName;
}

void Settings::updateSampleRateValue(double sampleRate)
{
    Settings::sampleRateValue = sampleRate;
    Settings::sampleRate = sampleRate;
    properties.saveIfNeeded();
    settingsFile.save();

}


int Settings::getAudioOutputMode()
{
    return properties.getUserSettings()->getValue("Audio Output Mode").getIntValue();
}


void Settings::buttonClicked(juce::Button* button)
{
    if (button == &launchLevelButton)
    {
        Settings::lauchAtZeroDB = button->getToggleState();
        properties.getUserSettings()->setValue("Launchatzero", (int)Settings::lauchAtZeroDB);
        properties.saveIfNeeded();
        settingsFile.save();
    }
    else if (button == &mouseWheelControlButton)
    {
        Settings::mouseWheelControlVolume = button->getToggleState();
        properties.getUserSettings()->setValue("MouseWheelControl", (int)Settings::mouseWheelControlVolume);
        properties.saveIfNeeded();
        settingsFile.save();
    }
    if (button == &normalizeButton)
    {
        Settings::autoNormalize = button->getToggleState();
        properties.getUserSettings()->setValue("AutoNormalize", (int)Settings::autoNormalize);
        properties.saveIfNeeded();
        settingsFile.save();
    }
}