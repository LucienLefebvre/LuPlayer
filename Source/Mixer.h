/*
  ==============================================================================

    Mixer.h
    Created: 3 Apr 2021 12:32:50pm
    Author:  DPR

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "RemoteInput.h"
#include "MixerInput.h"
//==============================================================================
/*
*/
class Mixer  : public juce::Component
{
public:
    Mixer();
    ~Mixer() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    //RemoteInput remoteInput1;
    void Mixer::prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void Mixer::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill);
    juce::OwnedArray<MixerInput> inputs;

private:
    void Mixer::addInput(MixerInput::Mode inputMode);

    double actualSampleRate;
    int actualSamplesPerBlockExpected;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Mixer)
};
