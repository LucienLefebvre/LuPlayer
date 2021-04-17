/*
  ==============================================================================

    MixerInput.cpp
    Created: 9 Apr 2021 9:18:19am
    Author:  DPR

  ==============================================================================
*/

#include <JuceHeader.h>
#include "MixerInput.h"

//==============================================================================
MixerInput::MixerInput(Mode mode)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    if (mode == Stereo)
    {

    }

    addAndMakeVisible(&volumeSlider);
    volumeSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    volumeSlider.setRange(-100, +12);
    volumeSlider.setValue(0.);
    //volumeSlider.addListener(this);
    volumeSlider.setSkewFactor(2, false);
    volumeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 15);
    volumeSlider.setNumDecimalPlacesToDisplay(1);
    volumeSlider.setWantsKeyboardFocus(false);
    volumeSlider.setDoubleClickReturnValue(true, 0.);
    volumeSlider.setScrollWheelEnabled(false);
    volumeSlider.setTextValueSuffix("dB");
    volumeSlider.addListener(this);
    level.setValue(juce::Decibels::decibelsToGain(volumeSlider.getValue()));

    addAndMakeVisible(panKnob);
    panKnob.setRange(-1.0f, 1.0f);
    panKnob.setSliderStyle(juce::Slider::LinearHorizontal);
    panKnob.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    panKnob.setNumDecimalPlacesToDisplay(1);
    panKnob.setDoubleClickReturnValue(true, 0.);
    panKnob.setScrollWheelEnabled(false);
    panKnob.setWantsKeyboardFocus(false);
    panKnob.addListener(this);
    pan.setValue(panKnob.getValue());

    addAndMakeVisible(&inputSelector);
    inputSelector.addListener(this);

    addAndMakeVisible(&selectButton);
    selectButton.setButtonText("Select");

    comboboxChanged = std::make_unique<juce::ChangeBroadcaster>();

}

MixerInput::~MixerInput()
{
}

void MixerInput::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

}

void MixerInput::resized()
{
    inputSelector.setBounds(0, 0, getWidth(), 25);
    panKnob.setBounds(0, inputSelector.getBottom(), getWidth(), 25);
    selectButton.setBounds(getWidth() / 4, panKnob.getBottom(), getWidth() / 2, 20);
    volumeSlider.setBounds(0, selectButton.getBottom(), getWidth(), getHeight() - selectButton.getBottom());
}

void MixerInput::getNextAudioBlock(juce::AudioBuffer<float>* inputBuffer, juce::AudioBuffer<float>* outputBuffer)
{
    if (selectedInput != -1)
    {
        channelBuffer->clear();//clear buffer channel



        channelBuffer->copyFrom(0, 0, inputBuffer->getReadPointer(selectedInput), inputBuffer->getNumSamples());//copy selected input into buffer channel
        channelBuffer->copyFrom(1, 0, inputBuffer->getReadPointer(selectedInput), inputBuffer->getNumSamples());

        filterProcessor.getNextAudioBlock(channelBuffer.get());

        level.getNextValue();//compute next fader level value
        pan.getNextValue();
        panL = juce::jmin(1 - pan.getCurrentValue(), 1.0f);
        panR = juce::jmin(1 + pan.getCurrentValue(), 1.0f);
        outputBuffer->addFrom(0, 0, *channelBuffer, 0, 0, channelBuffer->getNumSamples(), level.getCurrentValue() * panL);//copy into output buffer * gain 
        outputBuffer->addFrom(1, 0, *channelBuffer, 1, 0, channelBuffer->getNumSamples(), level.getCurrentValue() * panR);
    }
}

void MixerInput::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    actualSampleRate = sampleRate;
    actualSamplesPerBlockExpected = samplesPerBlockExpected;

    channelBuffer = std::make_unique<juce::AudioBuffer<float>>(2, actualSamplesPerBlockExpected);
    channelBuffer->setSize(2, actualSamplesPerBlockExpected);

    filterProcessor.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MixerInput::clearInputSelector()
{
    inputSelector.clear();
}

void MixerInput::feedInputSelector(int channel, juce::String name, bool isSelectable)
{
    inputSelector.addItem(name, channel );
    inputSelector.setItemEnabled(channel, isSelectable);
}

void MixerInput::selectDefaultInput(int defaultInput)
{
    inputSelector.setSelectedId(defaultInput + 1);
}

void MixerInput::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    selectedInput = comboBoxThatHasChanged->getSelectedId() - 2;
    //comboboxChanged->reset(new juce::ChangeBroadcaster());

    comboboxChanged->sendChangeMessage();
}


int MixerInput::getSelectedInput()
{
    return selectedInput;
}

void MixerInput::updateComboboxItemsState(int itemId, bool isEnabled)
{
    inputSelector.setItemEnabled(itemId, isEnabled);
}

void MixerInput::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &volumeSlider)
    {
        level.setTargetValue(juce::Decibels::decibelsToGain(volumeSlider.getValue()));
    }
    else if (slider == &panKnob)
    {
        pan.setTargetValue(panKnob.getValue());
    }
}

void MixerInput::setInputIndex(int index)
{
    inputIndex = index;
    filterProcessor.displaynumber = inputIndex;
}

int MixerInput::getInputIndex()
{
    return inputIndex;
}

