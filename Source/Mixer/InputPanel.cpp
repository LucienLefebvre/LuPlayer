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
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

    g.setColour (juce::Colours::grey);
    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

    g.setColour (juce::Colours::white);
    g.setFont (14.0f);
    g.drawText ("InputPanel", getLocalBounds(),
                juce::Justification::centred, true);   // draw some placeholder text
}

void InputPanel::resized()
{
    channelEditor.setBounds(0, 0, 100, getHeight());
    meter.setBounds(getWidth() - 100, 0, 100, getHeight());
    compEditor.setSize(200, getHeight());
    filterEditor.setBounds(channelEditor.getRight(), 0, getWidth() - meter.getWidth() - compEditor.getWidth() - channelEditor.getWidth(), getHeight());
    compEditor.setTopLeftPosition(filterEditor.getRight(), 0);
}

void InputPanel::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    actualSampleRate = sampleRate;
    actualSamplesPerBlockExpected = samplesPerBlockExpected;

    meterSource.resize(2, sampleRate * 0.1 / samplesPerBlockExpected);
    meter.setMeterSource(&meterSource);

    filterEditor.prepareToPlay(samplesPerBlockExpected, sampleRate);

}

void InputPanel::getNextAudioBlock(juce::AudioBuffer<float>* buffer)
{
    meterSource.measureBlock(*buffer);
}
