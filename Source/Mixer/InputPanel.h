/*
  ==============================================================================

    InputPanel.h
    Created: 21 Apr 2021 12:16:58pm
    Author:  DPR

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "FilterEditor.h"
#include "CompEditor.h"
#include "ChannelControlPannel.h"
#include "MixerInput.h"
//==============================================================================
/*
*/
class InputPanel  : public juce::Component, public juce::Timer
{
public:
    InputPanel();
    ~InputPanel() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void getNextAudioBlock(juce::AudioBuffer<float>* buffer);
    void updateInputInfo();
    void setEditedInput(MixerInput& i);
    FilterEditor filterEditor;
    CompEditor compEditor;
    ChannelControlPannel channelEditor;
private:
    void timerCallback();
    double actualSampleRate;
    int actualSamplesPerBlockExpected;

    MixerInput* editedMixerInput;

    Meter inputMeter;
    Meter outputMeter;

    foleys::LevelMeterLookAndFeel lnf;
    foleys::LevelMeter meter{ foleys::LevelMeter::Default }; // See foleys::LevelMeter::MeterFlags for options
    foleys::LevelMeterSource meterSource;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InputPanel)
};
