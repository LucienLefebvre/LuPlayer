/*
  ==============================================================================

    FilterProcessor.cpp
    Created: 16 Apr 2021 1:50:05am
    Author:  Lucien Lefebvre

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
    //analyser.prepareToPlay(samplesPerBlockExpected, sampleRate);
    initializeParameters();
}

void FilterProcessor::getNextAudioBlock(juce::AudioBuffer<float>* buffer)
{
    juce::dsp::AudioBlock<float> block(*buffer);
    //analyser.addAudioData(*buffer);
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
    if (processors[band] != nullptr)
        return processors[band]->state;
}

FilterProcessor::FilterParameters& FilterProcessor::getFilterParameters(int filterBand)
{
    FilterParameters fp;
    if (filterBand >= 0)
    {
        if (filtersParams[filterBand] != nullptr)
            fp = *filtersParams[filterBand];
    }
    return fp;
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

FilterProcessor::GlobalParameters& FilterProcessor::getGlobalFilterParameters()
{
    FilterProcessor::GlobalParameters globalParams;
    globalParams.lowBand = getFilterParameters(0);
    globalParams.lowMidBand = getFilterParameters(1);
    globalParams.highMidBand = getFilterParameters(2);
    globalParams.highBand = getFilterParameters(3);
    return globalParams;
}

void FilterProcessor::setGlobalFilterParameters(FilterProcessor::GlobalParameters p)
{
    setFilterParameters(0, p.lowBand);
    setFilterParameters(1, p.lowMidBand);
    setFilterParameters(2, p.highMidBand);
    setFilterParameters(3, p.highBand);
}


FilterProcessor::GlobalParameters FilterProcessor::makeDefaultFilter()
{
    FilterProcessor::GlobalParameters defaultParams;

    defaultParams.lowBand.frequency = 100.0f;
    defaultParams.lowBand.gain = 1.0f;
    defaultParams.lowBand.Q = 1.0f;
    defaultParams.lowBand.type = FilterProcessor::FilterTypes::Bell;

    defaultParams.lowMidBand.frequency = 400.0f;
    defaultParams.lowMidBand.gain = 1.0f;
    defaultParams.lowMidBand.Q = 1.0f;
    defaultParams.lowMidBand.type = FilterProcessor::FilterTypes::Bell;

    defaultParams.lowBand.frequency = 2000.0f;
    defaultParams.lowBand.gain = 1.0f;
    defaultParams.lowBand.Q = 1.0f;
    defaultParams.lowBand.type = FilterProcessor::FilterTypes::Bell;

    defaultParams.lowBand.frequency = 5000.0f;
    defaultParams.lowBand.gain = 1.0f;
    defaultParams.lowBand.Q = 1.0f;
    defaultParams.lowBand.type = FilterProcessor::FilterTypes::Bell;

    return defaultParams;
}

juce::StringArray FilterProcessor::getFilterParametersAsArray()
{
    juce::StringArray params;
    for (auto* par : filtersParams)
    {
        params.add(juce::String(par->frequency));
        params.add(juce::String(par->gain));
        params.add(juce::String(par->Q));
        params.add(juce::String(par->type));
    }
    return params;
}

void FilterProcessor::setFilterParametersAsArray(juce::StringArray p)
{
    for (int i = 0; i < filtersParams.size(); i++)
    {
        filtersParams[i]->frequency = p[i * 4].getFloatValue();
        filtersParams[i]->gain = p[(i * 4) + 1].getFloatValue();
        filtersParams[i]->Q = p[(i * 4) + 2].getFloatValue();
        switch (p[(i * 4) + 3].getIntValue())
        {
        case 0:
            filtersParams[i]->type = FilterProcessor::FilterTypes::Bell;
            break;
        case 1:
            filtersParams[i]->type = FilterProcessor::FilterTypes::LowShelf;
            break;
        case 2:
            filtersParams[i]->type = FilterProcessor::FilterTypes::HighShelf;
            break;
        case 3:
            filtersParams[i]->type = FilterProcessor::FilterTypes::HPF;
            break;
        case 4:
            filtersParams[i]->type = FilterProcessor::FilterTypes::LPF;
            break;
        }
    }

}