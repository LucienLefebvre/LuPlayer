/*
  ==============================================================================

    Mixer.cpp
    Created: 3 Apr 2021 12:32:50pm
    Author:  DPR

  ==============================================================================
*/

#include <JuceHeader.h>
#include "Mixer.h"

//==============================================================================
Mixer::Mixer()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    //addAndMakeVisible(&remoteInput1);
    //addInput(MixerInput::Mode::Mono);
}

Mixer::~Mixer()
{
}

void Mixer::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
}

void Mixer::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    
}

void Mixer::resized()
{
    //remoteInput1.setBounds(0, 0, 200, getHeight());

}

void Mixer::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    actualSampleRate = sampleRate;
    actualSamplesPerBlockExpected = samplesPerBlockExpected;

    //remoteInput1.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void Mixer::addInput(MixerInput::Mode inputMode)
{
    //inputs.add(new MixerInput(inputMode));
}