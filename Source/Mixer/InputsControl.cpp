/*
  ==============================================================================

    InputsControl.cpp
    Created: 21 Apr 2021 1:00:24pm
    Author:  DPR

  ==============================================================================
*/

#include <JuceHeader.h>
#include "InputsControl.h"

//==============================================================================
InputsControl::InputsControl(InputPanel& panel) : inputPanel(&panel)
{
    addAndMakeVisible(&addButton);
    addButton.setButtonText("+");
    addButton.onClick = [this] {addButtonClicked(); };
}

InputsControl::~InputsControl()
{
}

void InputsControl::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
}

void InputsControl::resized()
{
    rearrangeInputs();
    //inputPanel->setBounds(inputs.getLast()->getRight(), 0, getWidth() - inputs.getLast()->getRight(), getHeight());

}

void InputsControl::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    actualSampleRate = sampleRate;
    actualSamplesPerBlockExpected = samplesPerBlockExpected;


    mixerBuffer = std::make_unique<juce::AudioBuffer<float>>(2, actualSamplesPerBlockExpected);
    selectedInputBuffer = std::make_unique<juce::AudioBuffer<float>>(2, actualSamplesPerBlockExpected);
    mixerBuffer->setSize(2, actualSamplesPerBlockExpected);

    if (inputs.size() < 1)
        addInput(MixerInput::Mode::Mono);

    inputPanel->prepareToPlay(actualSamplesPerBlockExpected, actualSampleRate);
    setSelectedMixerInput(0);
}

void InputsControl::getNextAudioBlock(juce::AudioBuffer<float>* inputBuffer, juce::AudioBuffer<float>* outputBuffer)
{
    mixerBuffer->clear();
    //Copy inputs into mixerBuffer
    mixerBuffer->copyFrom(0, 0, *inputBuffer, 0, 0, actualSamplesPerBlockExpected);
    mixerBuffer->copyFrom(1, 0, *inputBuffer, 1, 0, actualSamplesPerBlockExpected);



    //Copy mixer Buffer into Outputs
    outputBuffer->clear();
    
    for (auto vca : VCAs)
        vca->updateLevel();
    for (auto i = 0; i < inputs.size(); i++)
    {
        //VCA
        if (!VCAs.isEmpty() && inputs[i]->isVCAAssigned())
            inputs[i]->setVCALevel(VCAs[0]->getLevel());
        //send buffer to channel
        inputs[i]->getNextAudioBlock(mixerBuffer.get(), outputBuffer);

        if (i == selectedMixerInput && inputs[i]->getSelectedInput() != -1) //send the selected input buffer to the input panel for meter measuring
        {
            selectedInputBuffer->clear();
            selectedInputBuffer->copyFrom(0, 0, *inputBuffer, inputs[i]->getSelectedInput(), 0, inputBuffer->getNumSamples());
            selectedInputBuffer->copyFrom(1, 0, *inputBuffer, inputs[i]->getSelectedInput(), 0, inputBuffer->getNumSamples());
            inputPanel->getNextAudioBlock(selectedInputBuffer.get());
        }
    }
    //outputBuffer->copyFrom(0, 0, *mixerBuffer, 0, 0, actualSamplesPerBlockExpected);
    //outputBuffer->copyFrom(1, 0, *mixerBuffer, 1, 0, actualSamplesPerBlockExpected);

}

void InputsControl::addButtonClicked()
{
    addMenu.clear();
    addMenu.addItem(1, "Mono Input", true, false);
    addMenu.addItem(2, "Stereo Input", true, false);
    if (VCAs.size() < 1)
        addMenu.addItem(3, "VCA", true, false);

    auto result = addMenu.show();

    switch (result)
    {
    case 1 :
        addInput(MixerInput::Mode::Mono);
        break;
    case 2 : 
        addInput(MixerInput::Mode::Stereo);
        break;
    case 3 :
        addVCA();
        break;
    }
}
void InputsControl::addInput(MixerInput::Mode inputMode)
{
    inputs.add(new MixerInput(inputMode));
    inputs.getLast()->inputEdited->addChangeListener(this);
    addAndMakeVisible(inputs.getLast());
    int inputIndex = inputs.size() - 1;
    inputs.getLast()->setInputIndex(inputIndex);
    updateInputSelectors(inputIndex);
    inputs.getLast()->selectButton.onClick = [this, inputIndex] {setSelectedMixerInput(inputIndex); };
    inputs.getLast()->prepareToPlay(actualSamplesPerBlockExpected, actualSampleRate);
    rearrangeInputs();
    setSelectedMixerInput(inputIndex);
}
void InputsControl::addVCA()
{
    VCAs.add(new VCA);
    addAndMakeVisible(VCAs.getLast());
    rearrangeInputs();
}

void InputsControl::setDeviceManagerInfos(juce::AudioDeviceManager& devicemanager)
{
    //deviceManager = &devicemanager;
    //numInputsChannels = deviceManager->getCurrentAudioDevice()->getActiveInputChannels().getHighestBit() + 1;
    //inputsChannelsName = deviceManager->getCurrentAudioDevice()->getInputChannelNames();
    //for (auto i = 0; i < inputs.size(); i++)
    //{
    //    updateInputSelectors(i);
    //}

    //int bufferChannelsToset = (numInputsChannels < 2) ? 2 : numInputsChannels;
}

void InputsControl::updateInputSelectors(int inputId)
{

        //inputs[inputId]->clearInputSelector();
        //inputs[inputId]->feedInputSelector(1, "None", true);
        //for (auto g = 0; g < numInputsChannels; g++)
        //{
        //    inputs[inputId]->feedInputSelector(g + 2, inputsChannelsName[g], true);//g + 2 because 0 is non-selected, 1 is none
        //}
        //if (inputId < numInputsChannels)
        //    inputs[inputId]->selectDefaultInput(inputId + 1);//select idefaults inputs
        //else
        //    inputs[inputId]->selectDefaultInput(0);

}

void InputsControl::updateInputSelectorsState()
{
    ////feed an array of the inputs states (selected or not)
    //selectedInputs.clear();
    //selectedInputs.resize(numInputsChannels);
    //for (auto i = 0; i < selectedInputs.size(); i++)
    //    selectedInputs.set(i, false);
    //for (auto i = 0; i < inputs.size(); i++)
    //{
    //    if (inputs[i]->getSelectedInput() != -1)
    //        selectedInputs.set(inputs[i]->getSelectedInput(), true);
    //}

    ////update input selector
    //for (auto i = 0; i < inputs.size(); i++)
    //{
    //    for (auto g = 0; g < numInputsChannels; g++)
    //    {
    //        if (selectedInputs[g] && inputs[i]->getSelectedInput() != g)
    //            inputs[i]->updateComboboxItemsState(g + 2, false);
    //        else
    //            inputs[i]->updateComboboxItemsState(g + 2, true);
    //    }
    //}

}


void InputsControl::rearrangeInputs()
{
    for (auto i = 0; i < inputs.size(); i++)
    {
        inputs[i]->setBounds(i * mixerInputWidth, 0, mixerInputWidth, getHeight());
    }
    for (auto i = 0; i < VCAs.size(); i++)
    {
        VCAs[i]->setBounds(inputs.getLast()->getRight() + i * mixerInputWidth, 0, mixerInputWidth, getHeight());
    }
    if (VCAs.isEmpty())
        addButton.setBounds(inputs.getLast()->getRight(), 0, 20, 20);
    else
        addButton.setBounds(VCAs.getLast()->getRight(), 0, 20, 20);
    setSize(addButton.getRight(), getHeight());
}

void InputsControl::changeListenerCallback(juce::ChangeBroadcaster* source)
{
   // updateInputSelectorsState();
    if (isSelectedInput)
    inputPanel->updateInputInfo();
}

void InputsControl::setSelectedMixerInput(int selectedInput)
{
    selectedMixerInput = selectedInput;
    inputPanel->filterEditor.setEditedFilterProcessor(inputs[selectedMixerInput]->filterProcessor);
    inputPanel->compEditor.setEditedCompProcessor(inputs[selectedMixerInput]->compProcessor);
    inputPanel->channelEditor.setEditedProcessors(*inputs[selectedMixerInput]);
    for (auto i = 0; i < inputs.size(); i++)
        if (i == selectedMixerInput)
        {
            inputs[i]->selectButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colour(40, 134, 189));
            isSelectedInput = true;
        }
        else
        {
            inputs[i]->selectButton.setColour(juce::TextButton::ColourIds::buttonColourId,
                getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
            isSelectedInput = false;
        }
}