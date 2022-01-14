/*
  ==============================================================================

    FilterProcessor.h
    Created: 16 Apr 2021 1:50:05am
    Author:  DPR

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
//#include "FFTAnalyser.h"
//#include "filterHelpers.h"
//#include "FilterEditor.h"

class FilterProcessor : public juce::Component
{
public:
    enum FilterTypes
    {
        Bell = 0,
        LowShelf = 1,
        HighShelf = 2,
        HPF = 3,
        LPF = 4
    };

    struct FilterParameters
    {
        float frequency = 1000.0f;
        float gain = 1.0;
        float Q = 1.0f;
        FilterTypes type = FilterTypes::Bell;
    };

    struct GlobalParameters
    {
        FilterParameters lowBand;
        FilterParameters lowMidBand;
        FilterParameters highMidBand;
        FilterParameters highBand;
    };

    FilterProcessor();
    ~FilterProcessor() override;
    //FFTAnalyser analyser;
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void getNextAudioBlock(juce::AudioBuffer<float>* buffer);

    FilterParameters& FilterProcessor::getFilterParameters(int filterBand);
    void setFilterParameters(int filterBand, FilterParameters params);
    FilterTypes getFilterTypes(int filterBand);
    void setFilterTypes(int filterBand, FilterTypes filterType);
    void createFilters(juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>& processor, FilterParameters params);
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> getFilterCoefs(int band);
    void setBypassed(bool bypassed);
    bool isBypassed();
    FilterProcessor::GlobalParameters& getGlobalFilterParameters();
    void setGlobalFilterParameters(FilterProcessor::GlobalParameters p);
    static FilterProcessor::GlobalParameters makeDefaultFilter();
    juce::StringArray getFilterParametersAsArray();
    void setFilterParametersAsArray(juce::StringArray p);
    int displaynumber = 0;

private:
    void initializeParameters();

    std::unique_ptr<juce::AudioBuffer<float>> filterBuffer;
    double actualSampleRate;
    int actualSamplesPerBlockExpected;

    juce::OwnedArray< juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>> processors;
    juce::OwnedArray<FilterParameters> filtersParams;
    bool isFilterBypassed = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilterProcessor)
};

FilterProcessor::GlobalParameters makeDefaultFilter();
