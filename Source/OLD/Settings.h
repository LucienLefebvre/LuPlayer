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
class Settings  : public juce::Component,
                    public juce::Slider::Listener,
                public juce::Label::Listener
        
{

public:
    Settings::Settings(Playlist* p, Playlist* pbis, juce::OSCSender* sender);
    ~Settings() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    
    juce::String Settings::getFFmpegPath();

    void Settings::setOptions(Playlist* p, Playlist* pbis);
    void Settings::saveOptions();
    void Settings::getOptions();


    void Settings::sliderValueChanged(juce::Slider* slider) override;

    void Settings::setPreferedMidiDevice(int midiDeviceIndex);
    int Settings::getPreferedMidiDevice();
    juce::String Settings::getIpAdress();
    int Settings::getInPort();
    int Settings::getOutPort();

private:
    juce::TextButton saveButton;
    
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

    juce::Label ipAdressLabel;
    juce::Label ipAdress1;
    juce::Label ipAdress2;
    juce::Label ipAdress3;
    juce::Label ipAdress4;
    juce::String adress1 = "0";
    juce::String adress2 = "0";
    juce::String adress3 = "0";
    juce::String adress4 = "0";
    juce::String ipAdress;

    juce::Label oscPorts;
    juce::Label oscOutPortLabel;
    juce::Label oscOutPort;
    juce::Label oscInPortLabel;
    juce::Label oscInPort;

    int outOscPort = 9000;
    int inOscPort = 9001;

    void Settings::selectFFmpeg(Playlist* p, Playlist* pbis);
    void Settings::selectSoundsFolder(Playlist* p, Playlist* pbis);
    void Settings::selectExiftool(Playlist* p, Playlist* pbis);
    juce::String Settings::getExiftoolPath();

    juce::String FFmpegPath;
    juce::String exiftoolPath;
    juce::String convertedSoundsPath;
    float skewFactor = 0.5;
    int maxFaderValue = 0;
    int preferedMidiDeviceIndex;
    int midiShift = 0;


    juce::ApplicationProperties properties;

    juce::PropertiesFile::Options options;
    juce::PropertiesFile settingsFile;


    void Settings::connectOSC(juce::OSCSender* sender);

    void labelTextChanged(juce::Label* labelThatHasChanged) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Settings)
};
