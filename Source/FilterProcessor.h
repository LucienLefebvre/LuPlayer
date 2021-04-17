/*
  ==============================================================================

    FilterProcessor.h
    Created: 16 Apr 2021 1:50:05am
    Author:  DPR

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
//#include "FilterEditor.h"

class FilterProcessor : public juce::Component
{
public:
    FilterProcessor();
    ~FilterProcessor() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void getNextAudioBlock(juce::AudioBuffer<float>* buffer);

    juce::String sayHelloWorld();
    std::array<float, 3> FilterProcessor::getFilterParameters(int filterBand);
    void FilterProcessor::setFilterParameters(int filterBand, std::array<float, 3>);

    //FilterEditor* FilterProcessor::getFilterEditor();
    int displaynumber = 0;
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

    std::array<float, 3> lowBandParams = { 100.0f, 1.0f, 1.0f };
    std::array<float, 3> middleLowBandParams = { 400.0f, 1.0f, 1.0f };
    std::array<float, 3> middleHighBandParams = { 2000.0f, 1.0f, 1.0f };
    std::array<float, 3> highBandParams = { 8000.0f, 1.0f, 1.0f };
    float result[3] = {};

    //FilterEditor* filterEditor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilterProcessor)
};