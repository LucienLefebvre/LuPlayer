/*
  ==============================================================================

    InputsControl.cpp
    Created: 21 Apr 2021 1:00:24pm
    Author:  Lucien Lefebvre

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
    for (auto input : inputs)
        input->prepareToPlay(samplesPerBlockExpected, sampleRate);
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
    }
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
}

void InputsControl::updateInputSelectors(int inputId)
{
}

void InputsControl::updateInputSelectorsState()
{
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
    if (selectedInput >= 0)
    {
        selectedMixerInput = selectedInput;
        inputPanel->setEditedInput(*inputs[selectedMixerInput]);

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
}

void InputsControl::deleteSelectedInput()
{
    setSelectedMixerInput(selectedMixerInput - 1);
    inputs.remove(selectedMixerInput + 1);

    rearrangeInputs();
}