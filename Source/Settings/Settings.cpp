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
bool Settings::enableFaderStart;
int Settings::midiShift;
int Settings::faderTempTime;
int Settings::faderDelayTime;
int Settings::lastSecondsTime;
int Settings::preferedMidiDeviceIndex;
bool Settings::OSCEnabled;
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
double Settings::normTarget;
bool Settings::showClock;
bool Settings::showTimer;
bool Settings::showMeter;
juce::Value Settings::showMeterValue;
int Settings::outputChannelsNumber;
juce::Value Settings::sampleRateValue;

juce::StringArray Settings::tempFiles;

int Settings::draggedPlaylist;
int Settings::draggedPlayer;
int Settings::editedPlayer;
int Settings::editedPlaylist;
int Settings::preferedSoundPlayerMode;

bool Settings::showEnveloppe;
bool Settings::viewLastPlayedSound;
bool Settings::showInBottomPanel;

int Settings::keyboardLayout;
int Settings::keyMappedSoundboardRows;
int Settings::keyMappedSoundboardColumns;

bool Settings::autoCheckNewUpdate;
//==============================================================================
Settings::Settings() : settingsFile(options)
{
    loadFromXML();
    
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
    skewFactorSlider.setBounds(200, maxFaderValueSlider.getBottom() + spacer, 300, 25);
    skewFactorSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    skewFactorSlider.setRange(0.1, 1.0, 0.1);
    skewFactorSlider.addListener(this);
    skewFactorSlider.setValue(Settings::skewFactorGlobal);
    skewFactorSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(skewFactorSlider);

    skewFactorLabel.setBounds(0, maxFaderValueSlider.getBottom() + spacer, 130, 25);
    addAndMakeVisible(skewFactorLabel);
    skewFactorLabel.setText("Fader Acceleration", juce::NotificationType::dontSendNotification);
    skewFactorLabelLeft.setBounds(130, maxFaderValueSlider.getBottom() + spacer, 70, 25);
    addAndMakeVisible(skewFactorLabelLeft);
    skewFactorLabelLeft.setText("Logarithmic", juce::NotificationType::dontSendNotification);
    skewFactorLabelLeft.setJustificationType(juce::Justification::right);
    skewFactorLabelRight.setBounds(500, maxFaderValueSlider.getBottom() + spacer, 100, 25);
    addAndMakeVisible(skewFactorLabelRight);
    skewFactorLabelRight.setText("Linear", juce::NotificationType::dontSendNotification);

    //FADER START
    faderStartButton.setToggleState(Settings::enableFaderStart, juce::dontSendNotification);
    faderStartButton.setButtonText("Enable fader start");
    faderStartButton.addListener(this);
    faderStartButton.setBounds(0, skewFactorLabel.getBottom() + spacer, 200, 25);
    addAndMakeVisible(&faderStartButton);

    //FADER DELAY
    faderDelayLabel.setBounds(300, skewFactorLabel.getBottom() + spacer, 200, 25);
    faderDelayLabel.setText("Fader start delay (ms)", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(faderDelayLabel);
    faderDelayValue.setBounds(500, skewFactorLabel.getBottom() + spacer, 50, 25);
    faderDelayValue.setText(juce::String(Settings::faderDelayTime), juce::NotificationType::dontSendNotification);
    addAndMakeVisible(faderDelayValue);
    faderDelayValue.setEditable(true, true, false);
    faderDelayValue.addListener(this);
    faderDelayValue.setJustificationType(juce::Justification::centred);
    faderDelayValue.setColour(juce::Label::outlineColourId, juce::Colours::black);

    //FADER TEMP
    faderTempLabel.setBounds(0, faderStartButton.getBottom() + spacer, 200, 25);
    faderTempLabel.setText("Fader Temporisation (ms)", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(faderTempLabel);
    faderTempValue.setBounds(200, faderStartButton.getBottom() + spacer, 50, 25);
    faderTempValue.setText(juce::String(Settings::faderTempTime), juce::NotificationType::dontSendNotification);
    addAndMakeVisible(faderTempValue);
    faderTempValue.setEditable(true, true, false);
    faderTempValue.addListener(this);
    faderTempValue.setJustificationType(juce::Justification::centred);
    faderTempValue.setColour(juce::Label::outlineColourId, juce::Colours::black);

    //AUTO NORMALIZE
    addAndMakeVisible(&normalizeButton);
    normalizeButton.setButtonText("Auto normalize sounds LU");
    normalizeButton.setBounds(0, faderTempValue.getBottom() + spacer, 200, 25);
    normalizeButton.setToggleState(Settings::autoNormalize, juce::NotificationType::dontSendNotification);
    normalizeButton.addListener(this);

    //NORMALIZE TARGET
    normalizeTargetLabel.setBounds(300, faderTempValue.getBottom() + spacer, 200, 25);
    normalizeTargetLabel.setText("Normalization target (LU)", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(normalizeTargetLabel);
    normalizeTargetValue.setBounds(500, faderTempValue.getBottom() + spacer, 50, 25);
    normalizeTargetValue.setText(juce::String(Settings::normTarget), juce::NotificationType::dontSendNotification);
    addAndMakeVisible(normalizeTargetValue);
    normalizeTargetValue.setEditable(true, true, false);
    normalizeTargetValue.addListener(this);
    normalizeTargetValue.setJustificationType(juce::Justification::centred);
    normalizeTargetValue.setColour(juce::Label::outlineColourId, juce::Colours::black);

    //LAST SECONDS WARNING
    lastSecondsLabel.setBounds(300, normalizeButton.getBottom() + spacer, 200, 25);
    lastSecondsLabel.setText("Last seconds warning (s)", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(lastSecondsLabel);
    lastSecondsValue.setBounds(500, normalizeButton.getBottom() + spacer, 50, 25);
    lastSecondsValue.setText(juce::String(Settings::lastSecondsTime), juce::NotificationType::dontSendNotification);
    addAndMakeVisible(lastSecondsValue);
    lastSecondsValue.setEditable(true, true, false);
    lastSecondsValue.addListener(this);
    lastSecondsValue.setJustificationType(juce::Justification::centred);
    lastSecondsValue.setColour(juce::Label::outlineColourId, juce::Colours::black);

    //SHOW METER
    addAndMakeVisible(&meterButton);
    meterButton.setButtonText("Show individual meter");
    meterButton.setBounds(0, normalizeButton.getBottom() + spacer, 200, 25);
    meterButton.setToggleState(Settings::showMeter, juce::NotificationType::dontSendNotification);
    meterButton.addListener(this);

    //LAUNCH LEVEL
    addAndMakeVisible(&launchLevelButton);
    launchLevelButton.setButtonText("Always launch sounds at 0dB");
    launchLevelButton.setBounds(0, meterButton.getBottom() + spacer, 200, 25);
    launchLevelButton.setToggleState(Settings::lauchAtZeroDB, juce::NotificationType::dontSendNotification);
    launchLevelButton.addListener(this);

    //MOUSE WHEEL CONTROL
    addAndMakeVisible(&mouseWheelControlButton);
    mouseWheelControlButton.setButtonText("Mouse wheel control volume");
    mouseWheelControlButton.setBounds(0, launchLevelButton.getBottom() + spacer, 200, 25);
    mouseWheelControlButton.setToggleState(Settings::mouseWheelControlVolume, juce::NotificationType::dontSendNotification);
    mouseWheelControlButton.addListener(this);

    //KEY MAPPED SOUNDBOARD SIZE
    addAndMakeVisible(&keyMappedSizeLabel);
    keyMappedSizeLabel.setBounds(0, mouseWheelControlButton.getBottom() + spacer, 200, 25);
    keyMappedSizeLabel.setText("Key mapped soundboard colums", juce::dontSendNotification);

    addAndMakeVisible(&keyMappedColumsValue);
    keyMappedColumsValue.setBounds(200, mouseWheelControlButton.getBottom() + spacer, 50, 25);
    keyMappedColumsValue.setText(juce::String(Settings::keyMappedSoundboardColumns), juce::dontSendNotification);
    keyMappedColumsValue.setEditable(true);
    keyMappedColumsValue.addListener(this);
    keyMappedColumsValue.setColour(juce::Label::outlineColourId, juce::Colours::black);

    addAndMakeVisible(&keyMappedRowsLabel);
    keyMappedRowsLabel.setBounds(250, mouseWheelControlButton.getBottom() + spacer, 50, 25);
    keyMappedRowsLabel.setText("rows", juce::dontSendNotification);

    addAndMakeVisible(&keyMappedRowsValue);
    keyMappedRowsValue.setBounds(300, mouseWheelControlButton.getBottom() + spacer, 50, 25);
    keyMappedRowsValue.setText(juce::String(Settings::keyMappedSoundboardRows), juce::dontSendNotification);
    keyMappedRowsValue.setEditable(true);
    keyMappedRowsValue.addListener(this);
    keyMappedRowsValue.setColour(juce::Label::outlineColourId, juce::Colours::black);

   //OUT
    addAndMakeVisible(oscOutPortLabel);
    oscOutPortLabel.setText("OSC Outgoing port", juce::NotificationType::dontSendNotification);
    oscOutPortLabel.setBounds(0, keyMappedSizeLabel.getBottom() + spacer, leftColumnWidth, 25);

    addAndMakeVisible(oscOutPort);
    oscOutPort.setText(juce::String(Settings::outOscPort), juce::NotificationType::dontSendNotification);
    oscOutPort.setBounds(leftColumnWidth, keyMappedSizeLabel.getBottom() + spacer, 50, 25);
    oscOutPort.setEditable(true, true, false);
    oscOutPort.addListener(this);
    oscOutPort.setJustificationType(juce::Justification::centred);
    oscOutPort.setColour(juce::Label::outlineColourId, juce::Colours::black);

    //IN
    addAndMakeVisible(oscInPortLabel);
    oscInPortLabel.setText("OSC Incoming port", juce::NotificationType::dontSendNotification);
    oscInPortLabel.setBounds(0, oscOutPort.getBottom() + spacer, leftColumnWidth, 25);

    addAndMakeVisible(oscInPort);
    oscInPort.setText(juce::String(Settings::inOscPort), juce::NotificationType::dontSendNotification);
    oscInPort.setBounds(leftColumnWidth, oscOutPort.getBottom() + spacer, 50, 25);
    oscInPort.setEditable(true, true, false);
    oscInPort.addListener(this);
    oscInPort.setJustificationType(juce::Justification::centred);
    oscInPort.setColour(juce::Label::outlineColourId, juce::Colours::black);

    //OSC IP Adress Destination
    addAndMakeVisible(ipAdressLabel);
    ipAdressLabel.setBounds(300, keyMappedSizeLabel.getBottom() + spacer, 50, 25);
    ipAdressLabel.setText("OSC Ip", juce::NotificationType::dontSendNotification);

    addAndMakeVisible(ipAdress1);
    ipAdress1.setBounds(350, keyMappedSizeLabel.getBottom() + spacer, 50, 25);
    ipAdress1.setText(Settings::adress1, juce::NotificationType::sendNotification);
    ipAdress1.setEditable(true, true, false);
    ipAdress1.addListener(this);
    ipAdress1.setJustificationType(juce::Justification::centred);
    ipAdress1.setColour(juce::Label::outlineColourId, juce::Colours::black);
    addAndMakeVisible(ipAdress2);
    ipAdress2.setBounds(400, keyMappedSizeLabel.getBottom() + spacer, 50, 25);
    ipAdress2.setText(Settings::adress2, juce::NotificationType::sendNotification);
    ipAdress2.setEditable(true, true, false);
    ipAdress2.addListener(this);
    ipAdress2.setJustificationType(juce::Justification::centred);
    ipAdress2.setColour(juce::Label::outlineColourId, juce::Colours::black);
    addAndMakeVisible(ipAdress3);
    ipAdress3.setBounds(450, keyMappedSizeLabel.getBottom() + spacer, 50, 25);
    ipAdress3.setText(Settings::adress3, juce::NotificationType::sendNotification);
    ipAdress3.setEditable(true, true, false);
    ipAdress3.addListener(this);
    ipAdress3.setJustificationType(juce::Justification::centred);
    ipAdress3.setColour(juce::Label::outlineColourId, juce::Colours::black);
    addAndMakeVisible(ipAdress4);
    ipAdress4.setBounds(500, keyMappedSizeLabel.getBottom() + spacer, 50, 25);
    ipAdress4.setText(Settings::adress4, juce::NotificationType::sendNotification);
    ipAdress4.setEditable(true, true, false);
    ipAdress4.addListener(this);
    ipAdress4.setJustificationType(juce::Justification::centred);
    ipAdress4.setColour(juce::Label::outlineColourId, juce::Colours::black);

    //AUDIO OUTPUT MODE
    addAndMakeVisible(audioOutputModeLabel);
    audioOutputModeLabel.setBounds(0, oscInPortLabel.getBottom() + spacer, 200, 25);
    audioOutputModeLabel.setText("Audio Output Mode", juce::NotificationType::sendNotification);

    addAndMakeVisible(audioOutputModeListbox);
    audioOutputModeListbox.setBounds(200, oscInPortLabel.getBottom() + spacer, 399, 25);
    audioOutputModeListbox.addItem("Mono (Left -> Output, Right -> Cue)", 1);
    audioOutputModeListbox.addItem("Stereo 2 Outputs (Output & Cue on same stereo Output)", 2);
    audioOutputModeListbox.addItem("Stereo 4 Outputs(Output on first pair, Cue on second)", 3);

    //if (Settings::outputChannelsNumber <= 4)
    //    audioOutputModeListbox.setItemEnabled(3, false);

    audioOutputModeListbox.setSelectedId(Settings::audioOutputMode);
    audioOutputModeListbox.addListener(this);

    keyboardLayoutBroadcaster.reset(new juce::ChangeBroadcaster);

#if RFBUILD
    //CONVERTED SOUNDS PATH 
    convertedSoundsLabel.setBounds(200, audioOutputModeLabel.getBottom() + spacer, 400, 25);
    addAndMakeVisible(convertedSoundsLabel);
    convertedSoundsLabel.setText(Settings::convertedSoundsPath, juce::NotificationType::dontSendNotification);

    convertedSoundsButtons.setBounds(1, audioOutputModeLabel.getBottom() + spacer, 199, 25);
    convertedSoundsButtons.setButtonText("Select sounds folder");
    addAndMakeVisible(convertedSoundsButtons);
    convertedSoundsButtons.onClick = [this] { selectSoundsFolder(); };
#endif

    //SAVE & CLOSE BUTTONS
    saveButton.setBounds(250, audioOutputModeLabel.getBottom() + 50 + spacer, 100, 50);
    addAndMakeVisible(saveButton);
    saveButton.setButtonText("Save & Close");
    saveButton.onClick = [this] { setOptions();
    makeIpAdress(); saveOptions(); };

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
    g.fillAll (juce::Colour(45, 56, 61));
    g.setColour(juce::Colour(40, 134, 189));
    g.drawRect(0, 0, 600, 470);

    g.setOpacity(0.57f);
    g.drawHorizontalLine(faderTempLabel.getBottom() + spacer / 2, 0, getWidth()); 
    g.drawHorizontalLine(mouseWheelControlButton.getBottom() + spacer / 2, 0, getWidth());
    g.drawHorizontalLine(keyMappedSizeLabel.getBottom() + spacer / 2, 0, getWidth());
    g.drawHorizontalLine(oscInPortLabel.getBottom() + spacer / 2, 0, getWidth());

#if RFBUILD
    g.drawHorizontalLine(convertedSoundsLabel.getBottom() + spacer / 2, 0, getWidth());
#else
    g.drawHorizontalLine(convertedSoundsLabel.getBottom() + spacer / 2, 0, getWidth());
#endif

}

void Settings::resized()
{
}

void Settings::loadFromXML()
{
    //OPTIONS
    options.applicationName = ProjectInfo::projectName;
    options.filenameSuffix = ".settings";

    options.folderName = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("LuPlayer").getFullPathName();
    options.storageFormat = juce::PropertiesFile::storeAsXML;
    properties.setStorageParameters(options);



    //LOAD PROPERTIES
    Settings::FFmpegPath = properties.getUserSettings()->getValue("FFmpeg Path");
    Settings::exiftoolPath = properties.getUserSettings()->getValue("Exiftool Path");
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
    Settings::preferedSoundPlayerMode = properties.getUserSettings()->getValue("PreferedSoundPlayerMode").getIntValue();

    if (properties.getUserSettings()->getValue("Converted Sound Path").isEmpty())
    {
        juce::File directory = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("LuPlayer/Sounds");
        if (directory.createDirectory())
            Settings::convertedSoundsPath = directory.getFullPathName();
    }
    else
        Settings::convertedSoundsPath = properties.getUserSettings()->getValue("Converted Sound Path");

    if (properties.getUserSettings()->getValue("Skew Factor").isEmpty())
        Settings::skewFactorGlobal = 0.5;
    else
        Settings::skewFactorGlobal = properties.getUserSettings()->getValue("Skew Factor").getFloatValue();

    if (properties.getUserSettings()->getValue("faderStart").isEmpty())
        Settings::enableFaderStart = 1;
    else
        Settings::enableFaderStart = properties.getUserSettings()->getValue("faderStart").getIntValue();

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

    if (properties.getUserSettings()->getValue("FaderDelay").isEmpty())
        Settings::faderDelayTime = 40;
    else
        Settings::faderDelayTime = properties.getUserSettings()->getValue("FaderDelay").getIntValue();

    if (properties.getUserSettings()->getValue("LastSeconds").isEmpty())
        Settings::lastSecondsTime = 5;
    else
        Settings::lastSecondsTime = properties.getUserSettings()->getValue("LastSeconds").getIntValue();

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

    if (properties.getUserSettings()->getValue("NormTarget").isEmpty())
        Settings::normTarget = 0;
    else
        Settings::normTarget = properties.getUserSettings()->getValue("NormTarget").getIntValue();

    if (properties.getUserSettings()->getValue("ShowMeter").isEmpty())
        Settings::showMeter = true;
    else
        Settings::showMeter = properties.getUserSettings()->getValue("ShowMeter").getIntValue();

    if (properties.getUserSettings()->getValue("ShowEnveloppe").isEmpty())
        Settings::showEnveloppe = false;
    else
        Settings::showEnveloppe = properties.getUserSettings()->getValue("ShowEnveloppe").getIntValue();

    if (properties.getUserSettings()->getValue("viewLastPlayedSound").isEmpty())
        Settings::viewLastPlayedSound = false;
    else
        Settings::viewLastPlayedSound = properties.getUserSettings()->getValue("viewLastPlayedSound").getIntValue();

    if (properties.getUserSettings()->getValue("showInBottomPanel").isEmpty())
        Settings::showInBottomPanel = true;
    else
        Settings::showInBottomPanel = properties.getUserSettings()->getValue("showInBottomPanel").getIntValue();

    if (properties.getUserSettings()->getValue("keyMappedRows").isEmpty())
        Settings::keyMappedSoundboardRows = 2;
    else
        Settings::keyMappedSoundboardRows = properties.getUserSettings()->getValue("keyMappedRows").getIntValue();

    if (properties.getUserSettings()->getValue("keyMappedColums").isEmpty())
        Settings::keyMappedSoundboardColumns = 6;
    else
        Settings::keyMappedSoundboardColumns = properties.getUserSettings()->getValue("keyMappedColums").getIntValue();

    HKL currentLayout = GetKeyboardLayout(0);
    unsigned int x = (unsigned int)currentLayout & 0x0000FFFF;
    if (x == 1033)
        Settings::keyboardLayout = 1;
    else if (x == 1036)
        Settings::keyboardLayout = 2;
    else if (x == 1031)
        Settings::keyboardLayout = 3;
    else if (properties.getUserSettings()->getValue("keyboardLayout").isEmpty())
        Settings::keyboardLayout = 1;
    else
        Settings::keyboardLayout = properties.getUserSettings()->getValue("keyboardLayout").getIntValue();

    if (properties.getUserSettings()->getValue("autoCheckUpdate").isEmpty())
        Settings::autoCheckNewUpdate = 1;
    else
        Settings::autoCheckNewUpdate = properties.getUserSettings()->getValue("autoCheckUpdate").getIntValue();

    if (properties.getUserSettings()->getValue("OSCEnabled").isEmpty())
        Settings::OSCEnabled = 0;
    else
        Settings::OSCEnabled = properties.getUserSettings()->getValue("OSCEnabled").getIntValue();

    if (properties.getUserSettings()->getValue("ShowClock").isEmpty())
        Settings::showClock = true;
    else
        Settings::showClock = properties.getUserSettings()->getValue("ShowClock").getIntValue();

    if (properties.getUserSettings()->getValue("ShowTimer").isEmpty())
        Settings::showTimer = false;
    else
        Settings::showTimer = properties.getUserSettings()->getValue("ShowTimer").getIntValue();
}

juce::String Settings::getFFmpegPath()
{
    return FFmpegPath;
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
    else if (labelThatHasChanged == &faderTempValue)
    {
        int value = (faderTempValue.getTextValue()).toString().getIntValue();
        if (value >= 0)
        {
            Settings::faderTempTime = value;
            properties.getUserSettings()->setValue("Fader Temp", Settings::faderTempTime);
        }
        else
            faderTempValue.setText(juce::String(Settings::faderTempTime), juce::dontSendNotification);
    }
    else if (labelThatHasChanged == &faderDelayValue)
    {
        int value = (faderDelayValue.getTextValue()).toString().getIntValue();
        if (value >= 0 && value <= 70)
        {
            Settings::faderDelayTime = value;
            properties.getUserSettings()->setValue("FaderDelay", Settings::faderDelayTime);
        }
        else
            faderDelayValue.setText(juce::String(Settings::faderDelayTime), juce::dontSendNotification);
    }
    else if (labelThatHasChanged == &lastSecondsValue)
    {
        int value = (lastSecondsValue.getTextValue()).toString().getIntValue();
        if (value >= 0)
        {
            Settings::lastSecondsTime = value;
            properties.getUserSettings()->setValue("LastSeconds", Settings::lastSecondsTime);
        }
        else
            lastSecondsValue.setText(juce::String(Settings::lastSecondsTime), juce::dontSendNotification);
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
    else if (labelThatHasChanged == &keyMappedColumsValue)
    { 
        Settings::keyMappedSoundboardColumns = juce::jlimit<int>(1, 10, keyMappedColumsValue.getText().getIntValue());
        keyMappedColumsValue.setText(juce::String(Settings::keyMappedSoundboardColumns), juce::dontSendNotification);
        properties.getUserSettings()->setValue("keyMappedColums", Settings::keyMappedSoundboardColumns);
        keyMappedSoundboardSize->sendChangeMessage();
    }
    else if (labelThatHasChanged == &keyMappedRowsValue)
    {
        Settings::keyMappedSoundboardRows = juce::jlimit<int>(1, 3, keyMappedRowsValue.getText().getIntValue());
        keyMappedRowsValue.setText(juce::String(Settings::keyMappedSoundboardRows), juce::dontSendNotification);
        properties.getUserSettings()->setValue("keyMappedRows", Settings::keyMappedSoundboardRows);
        keyMappedSoundboardSize->sendChangeMessage();
    }
    else if (labelThatHasChanged == &normalizeTargetValue)
    {
        Settings::normTarget = juce::jlimit<double>(-24, +24, normalizeTargetValue.getText().getIntValue());
        normalizeTargetValue.setText(juce::String(Settings::normTarget), juce::dontSendNotification);
        properties.getUserSettings()->setValue("NormTarget", Settings::normTarget);
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

void Settings::setPreferedSoundPlayerMode(int p)
{
    Settings::preferedSoundPlayerMode = p;
    properties.getUserSettings()->setValue("PreferedSoundPlayerMode", Settings::preferedSoundPlayerMode);
    properties.saveIfNeeded();
    settingsFile.save();
}

int Settings::getPreferedSoundPlayerMode()
{
    return properties.getUserSettings()->getValue("PreferedSoundPlayerMode").getIntValue();
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
    else if (button == &normalizeButton)
    {
        Settings::autoNormalize = button->getToggleState();
        properties.getUserSettings()->setValue("AutoNormalize", (int)Settings::autoNormalize);
        properties.saveIfNeeded();
        settingsFile.save();
    }
    else if (button == &meterButton)
    {
        setShowMeters(button->getToggleState());
    }
    else if (button == &faderStartButton)
    {
        Settings::enableFaderStart = button->getToggleState();
        properties.getUserSettings()->setValue("faderStart", (int)Settings::enableFaderStart);
        properties.saveIfNeeded();
        settingsFile.save();
    }
}

void Settings::setKeyMapping(juce::Array<int> c, juce::StringArray s)
{
    for (int i = 0; i < c.size(); i++)
    {
        properties.getUserSettings()->setValue(s[i], c[i]);
    }
    properties.saveIfNeeded();
    settingsFile.save();
}

juce::Array<int> Settings::getKeyMapping(juce::StringArray s)
{
    juce::Array<int> r;
    for (int i = 0; i < s.size(); i++)
    {
        r.set(i, properties.getUserSettings()->getValue(s[i]).getIntValue());
    }
    return r;
}

void Settings::setShowMeters(bool show)
{
    Settings::showMeter = show;
    Settings::showMeterValue = show;
    properties.getUserSettings()->setValue("ShowMeter", (int)Settings::showMeter);
    properties.saveIfNeeded();
    settingsFile.save();
}

void Settings::setShowEnveloppe(bool show)
{
    Settings::showEnveloppe = show;
    properties.getUserSettings()->setValue("ShowEnveloppe", (int)Settings::showEnveloppe);
    properties.saveIfNeeded();
    settingsFile.save();
}

void Settings::setViewLastPlayed(bool show)
{
    Settings::viewLastPlayedSound = show;
    properties.getUserSettings()->setValue("viewLastPlayedSound", (int)Settings::viewLastPlayedSound);
    properties.saveIfNeeded();
    settingsFile.save();
}

void Settings::setShowInBottomPanel(bool show)
{
    Settings::showInBottomPanel = show;
    properties.getUserSettings()->setValue("showInBottomPanel", (int)Settings::showInBottomPanel);
    properties.saveIfNeeded();
    settingsFile.save();
}

void Settings::setKeyboardLayout(int layout)
{
    Settings::keyboardLayout = layout;
    properties.getUserSettings()->setValue("keyboardLayout", (int)Settings::keyboardLayout);
    properties.saveIfNeeded();
    settingsFile.save();
    keyboardLayoutBroadcaster->sendChangeMessage();
}

void Settings::setAutoCheckUpdate(bool check)
{
    Settings::autoCheckNewUpdate = check;
    properties.getUserSettings()->setValue("autoCheckUpdate", (int)Settings::autoCheckNewUpdate);
    properties.saveIfNeeded();
    settingsFile.save();
}

void Settings::setOSCEnabled(bool enabled)
{
    Settings::OSCEnabled = enabled;
    properties.getUserSettings()->setValue("OSCEnabled", (int)Settings::OSCEnabled);
    properties.saveIfNeeded();
    settingsFile.save();
}

void Settings::setShowClock(bool show)
{
    Settings::showClock = show;
    properties.getUserSettings()->setValue("ShowClock", (int)Settings::showClock);
    properties.saveIfNeeded();
    settingsFile.save();
}

void Settings::setShowTimer(bool show)
{
    Settings::showTimer = show;
    properties.getUserSettings()->setValue("ShowTimer", (int)Settings::showTimer);
    properties.saveIfNeeded();
    settingsFile.save();
}

juce::StringArray Settings::getAcceptedFileFormats()
{
    juce::StringArray formats;
    formats.add(".wav");
    formats.add(".WAV");
    formats.add(".aiff");
    formats.add(".AIFF");
    formats.add(".aif");
    formats.add(".AIF");
    formats.add(".flac");
    formats.add(".FLAC");    
    formats.add(".opus");
    formats.add(".OPUS");
    formats.add(".mp3");
    formats.add(".MP3");
#if RFBUILD
    formats.add(".bwf");
    formats.add(".BWF");
#endif
    return formats;
}