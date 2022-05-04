/*
  ==============================================================================

    Mixer.cpp
    Created: 3 Apr 2021 12:32:50pm
    Author:  Lucien Lefebvre

  ==============================================================================
*/

#include <JuceHeader.h>
#include "Mixer.h"

//==============================================================================
Mixer::Mixer() : inputsControl(inputPanel)
{
    addAndMakeVisible(&inputsControl);
    addAndMakeVisible(&inputPanel);
    addAndMakeVisible(&inputsViewport);
    inputsViewport.setViewedComponent(&inputsControl, false);

    verticalDividerBar.reset(new juce::StretchableLayoutResizerBar(&myLayout, 1, true));
    addAndMakeVisible(verticalDividerBar.get());
    //Layout


    myLayout.setItemLayout(0,       //Mixer Inputs   
        100, -1.0,
        -0.2);
    myLayout.setItemLayout(1, 4, 4, 4); //Vertical Bar
    myLayout.setItemLayout(2,       //Input Panel
        800, -1.0,
        -0.8);

    inputPanel.channelEditor.deleteBroadcaster.addChangeListener(this);
}

Mixer::~Mixer()
{

}

void Mixer::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
    g.setColour(juce::Colour(40, 134, 189));
    //horizontalDividerBar.get()->paint(g);
    g.fillRect(verticalDividerBar.get()->getBounds());
}

void Mixer::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    actualSampleRate = sampleRate;
    actualSamplesPerBlockExpected = samplesPerBlockExpected;

    inputPanel.prepareToPlay(actualSamplesPerBlockExpected, actualSampleRate);
    inputsControl.prepareToPlay(actualSamplesPerBlockExpected, actualSampleRate);
}

void Mixer::getNextAudioBlock(juce::AudioBuffer<float>* inputBuffer, juce::AudioBuffer<float>* outputBuffer)
{
    inputsControl.getNextAudioBlock(inputBuffer, outputBuffer);
}




void Mixer::resized()
{
    inputsViewport.setSize(200, getHeight());
    inputsControl.setSize(inputsViewport.getWidth(), inputsViewport.getHeight());
    inputPanel.setSize(getWidth() - 202, getHeight());
    verticalDividerBar.get()->setSize(4, getHeight());

    Component* comps[] = { &inputsViewport, verticalDividerBar.get(), &inputPanel };
    myLayout.layOutComponents(comps, 3, 0, 0, getWidth(), getHeight(), false, false);
    
    if (inputsControl.getWidth() > inputsViewport.getWidth())
        inputsControl.setSize(inputsViewport.getWidth(), getHeight() - 10);
    else
        inputsControl.setSize(inputsViewport.getWidth(), inputsViewport.getHeight());
}



void Mixer::setDeviceManagerInfos(juce::AudioDeviceManager* devicemanager)
{
    inputsControl.setDeviceManagerInfos(*deviceManager);
}

void Mixer::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &inputPanel.channelEditor.deleteBroadcaster)
    {
        inputsControl.deleteSelectedInput();
    }
}