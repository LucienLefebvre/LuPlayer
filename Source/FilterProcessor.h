/*
  ==============================================================================

    FilterProcessor.h
    Created: 16 Apr 2021 1:50:05am
    Author:  DPR

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class FilterProcessor : public juce::Component
{
public:
    FilterProcessor();
    ~FilterProcessor() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void getNextAudioBlock(juce::AudioBuffer<float>* buffer);

private:
    double actualSampleRate;
    int actualSamplesPerBlockExpected;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilterProcessor)
};