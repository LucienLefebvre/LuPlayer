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
InputPanel::InputPanel() : inputMeter(Meter::Mode::Mono), outputMeter(Meter::Mode::Mono_ReductionGain)
{
    juce::Timer::startTimer(50);
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    addAndMakeVisible(&inputMeter);
    addAndMakeVisible(&outputMeter);
    addAndMakeVisible(&filterEditor);
    addAndMakeVisible(&dynamicsEditor);
    addAndMakeVisible(&channelEditor);
    //addAndMakeVisible(&meter);
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
    //inputMeter.setBounds(channelEditor.getRight(), 0, 50, getHeight());
    outputMeter.setBounds(getWidth() - 50, 0, 50, getHeight());
    //meter.setBounds(getWidth() - 100, 0, 100, getHeight());
    dynamicsEditor.setSize(190, getHeight());
    filterEditor.setBounds(channelEditor.getRight() + 3, 0, getWidth() - outputMeter.getWidth() - dynamicsEditor.getWidth() - channelEditor.getWidth() - 6, getHeight());
    dynamicsEditor.setTopLeftPosition(filterEditor.getRight() + 3, 0);
}

void InputPanel::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    actualSampleRate = sampleRate;
    actualSamplesPerBlockExpected = samplesPerBlockExpected;

    meterSource.resize(2, sampleRate * 0.1 / samplesPerBlockExpected);
    meter.setMeterSource(&meterSource);

    filterEditor.prepareToPlay(samplesPerBlockExpected, sampleRate);

    channelEditor.setEditedEditors(filterEditor, dynamicsEditor);
    channelEditor.prepareToPlay();
    outputMeter.prepareToPlay(samplesPerBlockExpected, sampleRate);

    
}

void InputPanel::setEditedInput(MixerInput& i)
{
    editedMixerInput = &i;
    filterEditor.setEditedFilterProcessor(editedMixerInput->filterProcessor);
    dynamicsEditor.setEditedCompProcessor(editedMixerInput->compProcessor);
    channelEditor.setEditedProcessors(*editedMixerInput);
    switch (editedMixerInput->getInputMode())
    {
    case MixerInput::Mode::Mono :
        inputMeter.setMeterMode(Meter::Mode::Mono);
        outputMeter.setMeterMode(Meter::Mode::Mono);
        break;
    case MixerInput::Mode::Stereo : 
        inputMeter.setMeterMode(Meter::Mode::Stereo);
        outputMeter.setMeterMode(Meter::Mode::Stereo);
    }
}

void InputPanel::getNextAudioBlock(juce::AudioBuffer<float>* buffer)
{
    //meterSource.measureBlock(*buffer);
    //inputMeter.measureBlock(buffer);
}

void InputPanel::updateInputInfo()
{
    channelEditor.updateInputInfo();
}

void InputPanel::timerCallback()
{
    inputMeter.setMeterData(editedMixerInput->inputMeter->getMeterData());
    outputMeter.setMeterData(editedMixerInput->outputMeter->getMeterData());
    outputMeter.setReductionGain(editedMixerInput->compProcessor.getCompReductionDB());
}