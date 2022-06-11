/*
  ==============================================================================

    Settings.h
    Created: 12 Feb 2021 10:47:21am
    Author:  Lucien

  ==============================================================================
*/
#pragma once

#include <JuceHeader.h>
#include <Windows.h>

class Settings  :   public juce::Component,
                    public juce::Slider::Listener,
                    public juce::Label::Listener,
                    public juce::ComboBox::Listener,
                    public juce::Button::Listener
{

public:
    Settings::Settings();
    ~Settings() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    juce::String Settings::getFFmpegPath();

    void Settings::setOptions();
    void Settings::saveOptions();
    void Settings::getOptions();

    void Settings::sliderValueChanged(juce::Slider* slider) override;

    void Settings::setPreferedMidiDevice(int midiDeviceIndex);
    int Settings::getPreferedMidiDevice();
    juce::String Settings::getIpAdress();
    int Settings::getInPort();
    int Settings::getOutPort();
    int Settings::getMaxFaderValue();
    void Settings::setPreferedAudioDeviceType(juce::String audioDeviceType);
    juce::String Settings::getPreferedAudioDeviceType();
    void Settings::setPreferedAudioDevice(juce::AudioDeviceManager::AudioDeviceSetup audioDevice);
    juce::AudioDeviceManager::AudioDeviceSetup Settings::getPreferedAudioDevice();
    void Settings::setPreferedAudioDeviceName(juce::String audioDeviceName);
    juce::String Settings::getPreferedAudioDeviceName();
    void Settings::updateSampleRateValue(double sampleRate);
    int Settings::getAudioOutputMode();
    void Settings::setPreferedSoundPlayerMode(int p);
    int Settings::getPreferedSoundPlayerMode();
    void setKeyMapping(juce::Array<int> c, juce::StringArray s);
    juce::Array<int> getKeyMapping(juce::StringArray s);
    void setShowMeters(bool show);
    void setShowEnveloppe(bool show);
    void setViewLastPlayed(bool show);
    void setKeyboardLayout(int layout);
    void setAutoCheckUpdate(bool check);
    void setOSCEnabled(bool enabled);
    static juce::StringArray getAcceptedFileFormats();
    juce::TextButton saveButton;

    int outputNumChannels = 0;

    static float Settings::maxFaderValueGlobal;
    static juce::Value Settings::maxFaderValue;
    static bool Settings::enableFaderStart;
    static float Settings::skewFactorGlobal;
    static int Settings::midiShift;
    static int Settings::faderTempTime;
    static int Settings::faderDelayTime;
    static int Settings::preferedMidiDeviceIndex;
    static int Settings::inOscPort;
    static int Settings::outOscPort;
    static bool Settings::OSCEnabled;
    static juce::String Settings::adress1;
    static juce::String Settings::adress2;
    static juce::String Settings::adress3;
    static juce::String Settings::adress4;
    static juce::String Settings::ipAdress;
    static juce::String Settings::FFmpegPath;
    static juce::String Settings::exiftoolPath;
    static juce::String Settings::convertedSoundsPath;
    static int Settings::audioOutputMode;
    static juce::String Settings::preferedAudioDeviceType;
    static juce::String Settings::preferedAudioDeviceName;
    static juce::AudioDeviceManager::AudioDeviceSetup Settings::preferedAudioDevice;
    static juce::String Settings::outputDeviceName;
    static juce::String Settings::inputDeviceName;
    static double Settings::sampleRate;
    static int Settings::bufferSize;
    static juce::BigInteger Settings::inputChannels;
    static bool Settings::useDefaultInputChannels;
    static juce::BigInteger Settings::outputChannels;
    static bool Settings::useDefaultOutputsChannels;
    static juce::Value Settings::audioOutputModeValue;
    static juce::StringArray Settings::tempFiles;
    
    static int Settings::outputChannelsNumber;
    static juce::Value Settings::sampleRateValue;

    static bool Settings::lauchAtZeroDB;
    static bool Settings::mouseWheelControlVolume;
    static bool Settings::autoNormalize;
    static bool Settings::showMeter;
    static juce::Value Settings::showMeterValue;
    static bool Settings::viewLastPlayedSound;
    static bool Settings::showEnveloppe;

    static int Settings::draggedPlaylist;
    static int Settings::draggedPlayer;
    static int Settings::editedPlayer;
    static int Settings::editedPlaylist;

    static int Settings::preferedSoundPlayerMode;

    static int Settings::keyboardLayout;
    static int Settings::keyMappedSoundboardRows;
    static int Settings::keyMappedSoundboardColumns;

    static bool Settings::autoCheckNewUpdate;

    std::unique_ptr<juce::ChangeBroadcaster> keyboardLayoutBroadcaster;
    std::unique_ptr<juce::ChangeBroadcaster> keyMappedSoundboardSize = std::make_unique<juce::ChangeBroadcaster>();

    juce::OSCSender sender;
    bool oscConnected = true;



private:
    juce::TextButton convertedSoundsButtons;
    juce::Label convertedSoundsLabel;

    juce::Slider skewFactorSlider;
    juce::Label skewFactorLabel;
    juce::Label skewFactorLabelLeft;
    juce::Label skewFactorLabelRight;

    juce::Slider maxFaderValueSlider;
    juce::Label maxFaderValueLabel;

    juce::ToggleButton faderStartButton;

    juce::Label midiShiftLabel;
    juce::Label midiShiftValue;

    juce::Label faderTempLabel;
    juce::Label faderTempValue;

    juce::Label faderDelayLabel;
    juce::Label faderDelayValue;

    juce::Label localIpAdressLabel;
    juce::Label localIpAdressValue;

    juce::Label ipAdressLabel;
    juce::Label ipAdress1;
    juce::Label ipAdress2;
    juce::Label ipAdress3;
    juce::Label ipAdress4;


    juce::Label oscPorts;
    juce::Label oscOutPortLabel;
    juce::Label oscOutPort;
    juce::Label oscInPortLabel;
    juce::Label oscInPort;

    juce::Label normalizeLabel;
    juce::ToggleButton normalizeButton;

    juce::Label meterLabel;
    juce::ToggleButton meterButton;

    juce::Label audioOutputModeLabel;
    juce::ComboBox audioOutputModeListbox;

    juce::Slider timerSlider;

    juce::ToggleButton launchLevelButton;
    juce::ToggleButton mouseWheelControlButton;

    juce::Label keyMappedSizeLabel;
    juce::Label keyMappedRowsLabel;
    juce::Label keyMappedRowsValue;
    juce::Label keyMappedColumsValue;

    void Settings::selectFFmpeg();
    void Settings::selectSoundsFolder();
    void Settings::selectExiftool();
    juce::String Settings::getExiftoolPath();

    void Settings::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;

    void Settings::buttonClicked(juce::Button* button) override;

    juce::ApplicationProperties properties;

    juce::PropertiesFile::Options options;
    juce::PropertiesFile settingsFile;

    void Settings::makeIpAdress();

    void labelTextChanged(juce::Label* labelThatHasChanged) override;

    int spacer = 5;
    int leftColumnWidth = 200;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Settings)
};
