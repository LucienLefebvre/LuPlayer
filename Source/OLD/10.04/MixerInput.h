/*
  ==============================================================================

    MixerInput.h
    Created: 9 Apr 2021 9:18:19am
    Author:  DPR

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class MixerInput  : public juce::Component
{
public:
    enum Mode
    {
        Mono,
        Stereo
    };

    MixerInput(Mode mode);
    ~MixerInput() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void MixerInput::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill);
    void MixerInput::prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    

private:

    Mode inputMode;
    juce::ComboBox inputSelector;

    double actualSampleRate;
    int actualSamplesPerBlockExpected;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MixerInput)
};
