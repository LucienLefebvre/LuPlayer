/*
  ==============================================================================

    ClipEffect.cpp
    Created: 12 Jan 2022 4:22:43pm
    Author:  DPR

  ==============================================================================
*/

#include <JuceHeader.h>
#include "ClipEffect.h"

//==============================================================================
ClipEffect::ClipEffect()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    inputMeter = &initializationMeter;
    outputMeter = &initializationMeter;
    compMeter = &initializationMeter;
    addAndMakeVisible(&filterEditor);
    addAndMakeVisible(&displayInputMeter);
    addAndMakeVisible(&displayOutputMeter);
    addAndMakeVisible(&displayCompMeter);
    addAndMakeVisible(&compEditor);
    juce::Timer::startTimer(50);
}

ClipEffect::~ClipEffect()
{
}

void ClipEffect::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    filterEditor.prepareToPlay(samplesPerBlockExpected, sampleRate);
    displayInputMeter.prepareToPlay(samplesPerBlockExpected, sampleRate);
    displayOutputMeter.prepareToPlay(samplesPerBlockExpected, sampleRate);
    displayCompMeter.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void ClipEffect::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
}

void ClipEffect::resized()
{
    displayInputMeter.setBounds(0, 0, meterSize, getHeight());
    displayOutputMeter.setBounds(getWidth() - meterSize, 0, meterSize, getHeight());
    compEditor.setBounds(displayOutputMeter.getX() - spaceBetweenComponents - compWidth - 50, 0, compWidth, getHeight());
    displayCompMeter.setReductionGainWidth(30);
    displayCompMeter.setBounds(displayOutputMeter.getX() - meterSize, 0, meterSize, getHeight());
    filterEditor.setBounds(displayInputMeter.getRight() + spaceBetweenComponents, 0, getWidth() - ((2 * meterSize) + (3 * spaceBetweenComponents) + compWidth), getHeight());
}

void ClipEffect::setEditedFilterProcessor(FilterProcessor& fp)
{
    filterEditor.setEditedFilterProcessor(fp);
}

void ClipEffect::setEditedCompProcessor(CompProcessor& cp)
{
    compEditor.setEditedCompProcessor(cp);
}
void ClipEffect::setEditedBuffer(std::unique_ptr<juce::AudioBuffer<float>>& buffer)
{
    editedBuffer = &buffer;
}

void ClipEffect::setMeters(Meter& inputM, Meter& outputM, Meter& compM)
{
    inputMeter = &inputM;
    outputMeter = &outputM;
    compMeter = &compM;
}

void ClipEffect::timerCallback()
{
    displayInputMeter.setMeterData(inputMeter->getMeterData());
    displayOutputMeter.setMeterData(outputMeter->getMeterData());
    displayCompMeter.setReductionGain(compMeter->getReductionGain());
}