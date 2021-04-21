/*
  ==============================================================================

    FilterProcessor.cpp
    Created: 16 Apr 2021 1:50:05am
    Author:  DPR

  ==============================================================================
*/
#pragma once
#include <JuceHeader.h>
#include "FilterProcessor.h"
FilterProcessor::FilterProcessor()
{

    for (auto i = 0; i < 4; i++)
    {
        processors.add(new juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>);
        filtersParams.add(new FilterParameters);
    }
    filtersParams[0]->frequency = 100;
    filtersParams[1]->frequency = 400;
    filtersParams[2]->frequency = 2000;
    filtersParams[3]->frequency = 5000;
}

FilterProcessor::~FilterProcessor()
{

}

void FilterProcessor::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    actualSampleRate = sampleRate;
    actualSamplesPerBlockExpected = samplesPerBlockExpected;
    filterBuffer = std::make_unique<juce::AudioBuffer<float>>(2, actualSamplesPerBlockExpected);
    analyser.prepareToPlay(samplesPerBlockExpected, sampleRate);
    initializeParameters();
}

void FilterProcessor::getNextAudioBlock(juce::AudioBuffer<float>* buffer)
{
    juce::dsp::AudioBlock<float> block(*buffer);
    analyser.addAudioData(*buffer);
    for (auto i = 0; i < processors.size(); i++)
    {
        if (!isFilterBypassed)
        processors[i]->process(juce::dsp::ProcessContextReplacing<float>(block));
    }
}

void FilterProcessor::initializeParameters()
{
    for (auto i = 0; i < processors.size(); i++)
    {
        processors[i]->reset();
    }
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = actualSampleRate;
    spec.maximumBlockSize = actualSamplesPerBlockExpected;
    spec.numChannels = 2;
    for (auto i = 0; i < processors.size(); i++)
    {
        processors[i]->prepare(spec);
        createFilters(*processors[i], *filtersParams[i]);
    }
}

void FilterProcessor::createFilters(juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>& processor, 
                                    FilterParameters params)
{
   switch (params.type)
   {
   case 0 :
       *processor.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(actualSampleRate,
                            params.frequency, params.Q, params.gain);
       break;
   case 1 :
       *processor.state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(actualSampleRate,
                            params.frequency, params.Q, params.gain);
       break;
   case 2 :
       *processor.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(actualSampleRate,
                            params.frequency, params.Q, params.gain);
       break;
   case 3 :
       *processor.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(actualSampleRate,
                            params.frequency);
       break;
   case 4 :
       *processor.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(actualSampleRate,
                            params.frequency);
       break;
   }
}

juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> FilterProcessor::getFilterCoefs(int band)
{
    return processors[band]->state;
}

FilterProcessor::FilterParameters& FilterProcessor::getFilterParameters(int filterBand)
{
    
    return *filtersParams[filterBand];
}

FilterProcessor::FilterTypes FilterProcessor::getFilterTypes(int filterBand)
{
    return HPF;
}

void FilterProcessor::setFilterTypes(int filterBand, FilterTypes filterType)
{

}
void FilterProcessor::setFilterParameters(int filterBand, FilterParameters params)
{
    filtersParams[filterBand]->frequency = params.frequency;
    filtersParams[filterBand]->gain = params.gain;
    filtersParams[filterBand]->Q = params.Q;
    filtersParams[filterBand]->type = params.type;
    createFilters(*processors[filterBand], params);
}

void FilterProcessor::setBypassed(bool bypassed)
{
    isFilterBypassed = bypassed;
}
bool FilterProcessor::isBypassed()
{
    return isFilterBypassed;
}