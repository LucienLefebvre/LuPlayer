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

    addAndMakeVisible(&filterEditor);
    addAndMakeVisible(&meter);
    lnf.setColour(foleys::LevelMeter::lmBackgroundColour, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    lnf.setColour(foleys::LevelMeter::lmMeterGradientLowColour, juce::Colours::green);
    lnf.setColour(foleys::LevelMeter::lmMeterGradientMaxColour, juce::Colours::red);
    lnf.setColour(foleys::LevelMeter::lmMeterGradientMidColour, juce::Colours::orange);
    meter.setLookAndFeel(&lnf);
}

Mixer::~Mixer()
{
    
}

void Mixer::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
    //g.fillAll(juce::Colours::white);
}

void Mixer::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    actualSampleRate = sampleRate;
    actualSamplesPerBlockExpected = samplesPerBlockExpected;


    mixerBuffer = std::make_unique<juce::AudioBuffer<float>>(2, actualSamplesPerBlockExpected);
    mixerBuffer->setSize(2, actualSamplesPerBlockExpected);

    meterSource.resize(2, sampleRate * 0.1 / samplesPerBlockExpected);
    meter.setMeterSource(&meterSource);

    filterEditor.prepareToPlay(samplesPerBlockExpected, sampleRate);

    for (auto i = 0; i < inputs.size(); i++)
    {
        inputs[i]->prepareToPlay(samplesPerBlockExpected, sampleRate);
    }

    setSelectedMixerInput(0);

}

void Mixer::getNextAudioBlock(juce::AudioBuffer<float>* inputBuffer, juce::AudioBuffer<float>* outputBuffer)
{
    mixerBuffer->clear();
    //Copy inputs into mixerBuffer
    mixerBuffer->copyFrom(0, 0, *inputBuffer, 0, 0, actualSamplesPerBlockExpected);
    mixerBuffer->copyFrom(1, 0, *inputBuffer, 1, 0, actualSamplesPerBlockExpected);



    //Copy mixer Buffer into Outputs
    outputBuffer->clear();
    for (auto i = 0; i < inputs.size(); i++)
    {
        inputs[i]->getNextAudioBlock(mixerBuffer.get(), outputBuffer);
    }
    //outputBuffer->copyFrom(0, 0, *mixerBuffer, 0, 0, actualSamplesPerBlockExpected);
    //outputBuffer->copyFrom(1, 0, *mixerBuffer, 1, 0, actualSamplesPerBlockExpected);
    meterSource.measureBlock(*outputBuffer);
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
    meter.setBounds(getWidth() / 2 - 200, 0, 100, getHeight());
    filterEditor.setBounds(meter.getRight(), 0, getWidth() - meter.getRight(), getHeight());
}



void Mixer::setDeviceManagerInfos(juce::AudioDeviceManager& devicemanager)
{
    deviceManager = &devicemanager;
    numInputsChannels = deviceManager->getCurrentAudioDevice()->getActiveInputChannels().getHighestBit() + 1;
    inputsChannelsName = deviceManager->getCurrentAudioDevice()->getInputChannelNames();
    updateInputSelectors();

    int bufferChannelsToset = (numInputsChannels < 2) ? 2 : numInputsChannels;


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
    int inputIndex = inputs.size() - 1;
    inputs.getLast()->setInputIndex(inputIndex);

    inputs.getLast()->selectButton.onClick = [this, inputIndex] {setSelectedMixerInput(inputIndex); };
}


void Mixer::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    updateInputSelectorsState();
}

void Mixer::setSelectedMixerInput(int selectedInput)
{
    selectedMixerInput = selectedInput;
    filterEditor.setEditedFilterProcessor(inputs[selectedMixerInput]->filterProcessor);
    for (auto i = 0; i < inputs.size(); i++)
        if (i == selectedMixerInput)
        {
            inputs[i]->selectButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colour(40, 134, 189));
        }
        else
            inputs[i]->selectButton.setColour(juce::TextButton::ColourIds::buttonColourId, 
                               getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}