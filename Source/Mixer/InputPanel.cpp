/*
  ==============================================================================

    InputPanel.cpp
    Created: 21 Apr 2021 12:16:58pm
    Author:  DPR

  ==============================================================================
*/

#include <JuceHeader.h>
#include "InputPanel.h"

//==============================================================================
InputPanel::InputPanel()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    addAndMakeVisible(&filterEditor);
    addAndMakeVisible(&compEditor);
    addAndMakeVisible(&channelEditor);
    addAndMakeVisible(&meter);
    lnf.setColour(foleys::LevelMeter::lmBackgroundColour, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    lnf.setColour(foleys::LevelMeter::lmMeterGradientLowColour, juce::Colours::green);
    lnf.setColour(foleys::LevelMeter::lmMeterGradientMaxColour, juce::Colours::red);
    lnf.setColour(foleys::LevelMeter::lmMeterGradientMidColour, juce::Colours::orange);
    meter.setLookAndFeel(&lnf);
}

InputPanel::~InputPanel()
{
}

void InputPanel::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
    g.setColour(juce::Colour(40, 134, 189));
    g.drawRect(channelEditor.getRight() + 1, 0, 2, getHeight());
    g.drawRect(filterEditor.getRight(), 0, 2, getHeight());
}

void InputPanel::resized()
{
    channelEditor.setBounds(0, 0, 100, getHeight());
    meter.setBounds(getWidth() - 100, 0, 100, getHeight());
    compEditor.setSize(200, getHeight());
    filterEditor.setBounds(channelEditor.getRight() + 3, 0, getWidth() - meter.getWidth() - compEditor.getWidth() - channelEditor.getWidth(), getHeight());
    compEditor.setTopLeftPosition(filterEditor.getRight() + 3, 0);
}

void InputPanel::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    actualSampleRate = sampleRate;
    actualSamplesPerBlockExpected = samplesPerBlockExpected;

    meterSource.resize(2, sampleRate * 0.1 / samplesPerBlockExpected);
    meter.setMeterSource(&meterSource);

    filterEditor.prepareToPlay(samplesPerBlockExpected, sampleRate);

    channelEditor.setEditedEditors(filterEditor);
}

void InputPanel::getNextAudioBlock(juce::AudioBuffer<float>* buffer)
{
    meterSource.measureBlock(*buffer);
}

void InputPanel::updateInputInfo()
{
    channelEditor.updateInputInfo();
}