/*
  ==============================================================================

    PlayerProcessor.cpp
    Created: 21 Apr 2021 10:30:29am
    Author:  DPR

  ==============================================================================
*/

#include <JuceHeader.h>
#include "PlayerProcessor.h"

//==============================================================================
PlayerProcessor::PlayerProcessor()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

PlayerProcessor::~PlayerProcessor()
{
}

void PlayerProcessor::prepareToPlay(double newSampleRate, int newSamplesPerBlock)
{
    actualSampleRate = newSampleRate;
    actualSamplesPerBlockExpected = newSamplesPerBlock;
}
void PlayerProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{

}
void PlayerProcessor::releaseResources()
{

}

const juce::String PlayerProcessor::getName() const
{
    return("PlayerProcessor");
}
bool PlayerProcessor::acceptsMidi() const
{
    return false;
}
bool PlayerProcessor::producesMidi() const
{
    return false;
}
bool PlayerProcessor::isMidiEffect() const
{
    return false;
}
double PlayerProcessor::getTailLengthSeconds() const
{
    return 0.0;
}
int PlayerProcessor::getNumPrograms()
{
    return 1;
}
int PlayerProcessor::getCurrentProgram()
{
    return 0;
}
void PlayerProcessor::setCurrentProgram(int index)
{

}
const juce::String PlayerProcessor::getProgramName(int index)
{
    return {};
}
void PlayerProcessor::changeProgramName(int index, const juce::String& newName)
{

}
void PlayerProcessor::getStateInformation(juce::MemoryBlock& destData)
{

}
void PlayerProcessor::setStateInformation(const void* data, int sizeInBytes)
{

}
juce::AudioProcessorEditor* PlayerProcessor::createEditor()
{
    return new PlayerEditor(*this);
}
bool PlayerProcessor::hasEditor() const
{
    return true;
}