/*
  ==============================================================================

    Settings.h
    Created: 12 Feb 2021 10:47:21am
    Author:  Lucien

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Playlist.h"

//==============================================================================
/*
*/
class Settings  :   public juce::Component,
                    public juce::Slider::Listener,
                    public juce::Label::Listener,
                    public juce::ComboBox::Listener
        
{

public:
    Settings::Settings(int ouputChannelsAvalaible);
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


    juce::TextButton saveButton;
    int maxFaderValue = 0;

    int outputNumChannels = 0;

    static float Settings::maxFaderValueGlobal;
    static float Settings::skewFactorGlobal;
    static int Settings::midiShift;
    static int Settings::faderTempTime;
    static int Settings::preferedMidiDeviceIndex;
    static int Settings::inOscPort;
    static int Settings::outOscPort;
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
    

private:

    
    juce::TextButton selectFFmpegButton;
    juce::Label ffmpegPathLabel;

    juce::TextButton selectExiftoolButton;
    juce::Label exiftoolLabel;

    juce::TextButton convertedSoundsButtons;
    juce::Label convertedSoundsLabel;

    juce::Slider skewFactorSlider;
    juce::Label skewFactorLabel;
    juce::Label skewFactorLabelLeft;
    juce::Label skewFactorLabelRight;

    juce::Slider maxFaderValueSlider;
    juce::Label maxFaderValueLabel;

    juce::Label midiShiftLabel;
    juce::Label midiShiftValue;

    juce::Label faderTempLabel;
    juce::Label faderTempValue;

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

    juce::Label audioOutputModeLabel;
    juce::ComboBox audioOutputModeListbox;



    void Settings::selectFFmpeg();
    void Settings::selectSoundsFolder();
    void Settings::selectExiftool();
    juce::String Settings::getExiftoolPath();

    void Settings::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;




    



    juce::ApplicationProperties properties;

    juce::PropertiesFile::Options options;
    juce::PropertiesFile settingsFile;


    void Settings::makeIpAdress();

    void labelTextChanged(juce::Label* labelThatHasChanged) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Settings)
};
