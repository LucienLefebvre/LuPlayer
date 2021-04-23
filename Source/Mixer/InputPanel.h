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
//==============================================================================
/*
*/
class InputPanel  : public juce::Component
{
public:
    InputPanel();
    ~InputPanel() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void getNextAudioBlock(juce::AudioBuffer<float>* buffer);
    void updateInputInfo();
    FilterEditor filterEditor;
    CompEditor compEditor;
    ChannelControlPannel channelEditor;
private:

    double actualSampleRate;
    int actualSamplesPerBlockExpected;



    foleys::LevelMeterLookAndFeel lnf;
    foleys::LevelMeter meter{ foleys::LevelMeter::Default }; // See foleys::LevelMeter::MeterFlags for options
    foleys::LevelMeterSource meterSource;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InputPanel)
};
