/*
  ==============================================================================

    FilterProcessor.cpp
    Created: 16 Apr 2021 1:50:05am
    Author:  DPR

  ==============================================================================
*/
#include <JuceHeader.h>
#include "FilterProcessor.h"
FilterProcessor::FilterProcessor()
{
    /*lfFilterParameters.algorithm = filterAlgorithm::kLowShelf;
    lfFilterParameters.boostCut_dB = -20.;
    lfFilterParameters.fc = 400;
    lfFilterParameters.Q = 0.5;*/
}

FilterProcessor::~FilterProcessor()
{

}

FilterEditor* FilterProcessor::getFilterEditor()
{
    return nullptr;
}

void FilterProcessor::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    actualSampleRate = sampleRate;
    actualSamplesPerBlockExpected = samplesPerBlockExpected;
    filterBuffer = std::make_unique<juce::AudioBuffer<float>>(2, actualSamplesPerBlockExpected);
    //filterBuffer->setSize(2, actualSamplesPerBlockExpected);



    updateParameters();

}

void FilterProcessor::getNextAudioBlock(juce::AudioBuffer<float>* buffer)
{
    juce::dsp::AudioBlock<float> block(*buffer);

    lowBand.process(juce::dsp::ProcessContextReplacing<float>(block));
    /*middleLowBand.process(juce::dsp::ProcessContextReplacing<float>(block));
    middleHighBand.process(juce::dsp::ProcessContextReplacing<float>(block));
    highBand.process(juce::dsp::ProcessContextReplacing<float>(block));*/
}

void FilterProcessor::updateParameters()
{
    lowBand.reset();
    middleLowBand.reset();
    middleHighBand.reset();
    highBand.reset();

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = actualSampleRate;
    spec.maximumBlockSize = actualSamplesPerBlockExpected;
    spec.numChannels = 2;
    lowBand.prepare(spec);
    /*middleLowBand.prepare(spec);
    middleHighBand.prepare(spec);
    highBand.prepare(spec);*/

    DBG("filter frequency " << filtersFrequencies[0]);
    DBG("filter gain " << filtersGains[0]);
    DBG("filter Qs " << filtersQs[0]);
    lowBand.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(actualSampleRate, 1000);
    /*middleLowBand.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(actualSampleRate, filtersFrequencies[1], filtersQs[1], filtersGains[1]);
    middleHighBand.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(actualSampleRate, filtersFrequencies[2], filtersQs[2], filtersGains[2]);
    highBand.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(actualSampleRate, filtersFrequencies[3], filtersQs[3], filtersGains[3]);*/
    //update parameters
}
