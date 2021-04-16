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
    for (auto i = 0; i < 4; i++)
    {
        addInput(MixerInput::Mode::Mono);
    }

    //assign default input by order
    for (auto i = 0; i < inputs.size(); i++)
        inputs[i]->setInputNumber(i);

}

Mixer::~Mixer()
{
    
}

void Mixer::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
    //g.fillAll(juce::Colours::white);
}

void Mixer::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{

}

void Mixer::setInputBuffer(const juce::AudioSourceChannelInfo& inputBuffer)
{

}

juce::AudioBuffer<float>* Mixer::getOutputBuffer()
{
    return mixerBuffer.get();
}

void Mixer::resized()
{
    //remoteInput1.setBounds(0, 0, 200, getHeight());
    for (auto i = 0; i < inputs.size(); i++)
    {
        inputs[i]->setBounds(i * mixerInputWidth, 0, mixerInputWidth, getHeight());
    }
}

void Mixer::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    actualSampleRate = sampleRate;
    actualSamplesPerBlockExpected = samplesPerBlockExpected;





    //DBG("mixer buffer channels " << mixerBuffer->getNumChannels());
    //remoteInput1.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void Mixer::setDeviceManagerInfos(juce::AudioDeviceManager& devicemanager)
{
    deviceManager = &devicemanager;
    numInputsChannels = deviceManager->getCurrentAudioDevice()->getActiveInputChannels().getHighestBit() + 1;
    inputsChannelsName = deviceManager->getCurrentAudioDevice()->getInputChannelNames();
    updateInputSelectors();

    int bufferChannelsToset = (numInputsChannels < 2) ? 2 : numInputsChannels;
    mixerBuffer = std::make_unique<juce::AudioBuffer<float>>(bufferChannelsToset, actualSamplesPerBlockExpected);
    mixerBuffer->setSize(bufferChannelsToset, actualSamplesPerBlockExpected);

}

void Mixer::updateInputSelectors()
{
    for (auto i = 0; i < inputs.size(); i++)
    {
        inputs[i]->clearInputSelector();
        inputs[i]->feedInputSelector(1, "None", true);
        for (auto g = 0; g < numInputsChannels; g++)
        {
            inputs[i]->feedInputSelector(g + 2, inputsChannelsName[g], true);//g + 2 because 0 is non-selected, 1 is none
        }
        if (i < numInputsChannels)
            inputs[i]->selectDefaultInput(i + 1);//select idefaults inputs
        else
            inputs[i]->selectDefaultInput(0);
    }
    updateInputSelectorsState();
}

void Mixer::updateInputSelectorsState()
{
    //feed an array of the inputs states (selected or not)
    selectedInputs.clear();
    selectedInputs.resize(numInputsChannels);
    for (auto i = 0; i < selectedInputs.size(); i++)
        selectedInputs.set(i, false);
    for (auto i = 0; i < inputs.size(); i++)
    {
        if (inputs[i]->getSelectedInput() != -1)
        selectedInputs.set(inputs[i]->getSelectedInput(), true);
    }

    //update input selector
    for (auto i = 0; i < inputs.size(); i++)
    {
        for (auto g = 0; g < numInputsChannels; g++)
        {
            if (selectedInputs[g] && inputs[i]->getSelectedInput() != g)
                inputs[i]->updateComboboxItemsState(g + 2, false);
            else
                inputs[i]->updateComboboxItemsState(g + 2, true);
        }
    }

}
void Mixer::addInput(MixerInput::Mode inputMode)
{
    inputs.add(new MixerInput(inputMode));
    inputs.getLast()->comboboxChanged->addChangeListener(this);
    addAndMakeVisible(inputs.getLast());
}


void Mixer::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    updateInputSelectorsState();
}