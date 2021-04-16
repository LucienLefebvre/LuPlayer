/*
  ==============================================================================

    FilterProcessor.h
    Created: 16 Apr 2021 1:50:05am
    Author:  DPR

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "FilterEditor.h"

class FilterProcessor : public juce::Component
{
public:
    FilterProcessor();
    ~FilterProcessor() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void getNextAudioBlock(juce::AudioBuffer<float>* buffer);

    FilterEditor* FilterProcessor::getFilterEditor();

private:
    std::unique_ptr<juce::AudioBuffer<float>> filterBuffer;
    double actualSampleRate;
    int actualSamplesPerBlockExpected;

    void updateParameters();



    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> lowBand;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> middleLowBand;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> middleHighBand;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> highBand;

    float filtersFrequencies[4] = { 200.0f, 800.0f, 2000.0f, 8000.0f };
    float filtersQs[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float filtersGains[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    //FilterEditor* filterEditor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilterProcessor)
};