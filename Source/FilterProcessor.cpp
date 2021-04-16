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

}

FilterProcessor::~FilterProcessor()
{

}

void FilterProcessor::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    actualSampleRate = sampleRate;
    actualSamplesPerBlockExpected = samplesPerBlockExpected;
}

void FilterProcessor::getNextAudioBlock(juce::AudioBuffer<float>* buffer)
{

}