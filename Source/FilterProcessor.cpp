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

//FilterEditor* FilterProcessor::getFilterEditor()
//{
//    return filterEditor;
//}

void FilterProcessor::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    actualSampleRate = sampleRate;
    actualSamplesPerBlockExpected = samplesPerBlockExpected;
    filterBuffer = std::make_unique<juce::AudioBuffer<float>>(2, actualSamplesPerBlockExpected);
    updateParameters();
}

void FilterProcessor::getNextAudioBlock(juce::AudioBuffer<float>* buffer)
{
    juce::dsp::AudioBlock<float> block(*buffer);

    lowBand.process(juce::dsp::ProcessContextReplacing<float>(block));
    middleLowBand.process(juce::dsp::ProcessContextReplacing<float>(block));
    middleHighBand.process(juce::dsp::ProcessContextReplacing<float>(block));
    highBand.process(juce::dsp::ProcessContextReplacing<float>(block));
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
    middleLowBand.prepare(spec);
    middleHighBand.prepare(spec);
    highBand.prepare(spec);


    *lowBand.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(actualSampleRate, 
                            lowBandParams[0], lowBandParams[1], lowBandParams[2]);
    *middleLowBand.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(actualSampleRate, 
                            middleLowBandParams[0], middleLowBandParams[1], middleLowBandParams[2]);
    *middleHighBand.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(actualSampleRate, 
                            middleHighBandParams[0], middleHighBandParams[1], middleHighBandParams[2]);
    *highBand.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(actualSampleRate, 
                            highBandParams[0], highBandParams[1], highBandParams[2]);
    //update parameters
}


juce::String FilterProcessor::sayHelloWorld()
{
    return "Hello, world ! ";
}

std::array<float, 3> FilterProcessor::getFilterParameters(int filterBand)
{
    std::array<float, 3> result;
    switch (filterBand)
    {
    case 0:
        result = lowBandParams;
        break;
    case 1:
        result = middleLowBandParams;
        break;
    case 2:
        result = middleHighBandParams;
        break;
    case 3:
        result = highBandParams;
        break;
    }
    return result;
}

void FilterProcessor::setFilterParameters(int filterBand, std::array<float, 3> filterParams)
{
    /*lowBand.reset();
    middleLowBand.reset();
    middleHighBand.reset();
    highBand.reset();*/

    switch (filterBand)
    {
    case 0:
        lowBandParams = filterParams;
        *lowBand.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(actualSampleRate, 
                                lowBandParams[0], lowBandParams[1], lowBandParams[2]);
        break;
    case 1:
        middleLowBandParams = filterParams;
        *middleLowBand.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(actualSampleRate, 
                                middleLowBandParams[0], middleLowBandParams[1], middleLowBandParams[2]);
        break;
    case 2:
        middleHighBandParams = filterParams;
        *middleHighBand.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(actualSampleRate, 
                                middleHighBandParams[0], middleHighBandParams[1], middleHighBandParams[2]);
        break;
    case 3:
        highBandParams = filterParams;
        *highBand.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(actualSampleRate, 
                                highBandParams[0], highBandParams[1], highBandParams[2]);
        break;
    }
}