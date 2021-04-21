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
    if (mode == Stereo)
    {

    }

    addAndMakeVisible(&volumeSlider);
    volumeSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    volumeSlider.setRange(-100, +12);
    volumeSlider.setValue(0.);
    volumeSlider.setSkewFactor(2, false);
    volumeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 15);
    volumeSlider.setNumDecimalPlacesToDisplay(1);
    volumeSlider.setWantsKeyboardFocus(false);
    volumeSlider.setDoubleClickReturnValue(true, 0.);
    volumeSlider.setScrollWheelEnabled(false);
    volumeSlider.setTextValueSuffix("dB");
    volumeSlider.addListener(this);
    volumeSlider.setColour(juce::Slider::ColourIds::textBoxOutlineColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
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

    addAndMakeVisible(&selectButton);
    selectButton.setButtonText("Select");

    addAndMakeVisible(&inputLabel);
    inputLabel.setEditable(true);

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
    inputLabel.setBounds(0, 0, getWidth(), 20);
    panKnob.setBounds(0, inputLabel.getBottom(), getWidth(), 25);
    selectButton.setBounds(15, getBottom() - 20, 70, 20);

    volumeSlider.setBounds(0, panKnob.getBottom(), getWidth(), getHeight() - panKnob.getBottom() - inputLabel.getHeight());
    
}

void MixerInput::getNextAudioBlock(juce::AudioBuffer<float>* inputBuffer, juce::AudioBuffer<float>* outputBuffer)
{
    if (selectedInput != -1)
    {
        channelBuffer->clear();//clear buffer channel



        channelBuffer->copyFrom(0, 0, inputBuffer->getReadPointer(selectedInput), inputBuffer->getNumSamples());//copy selected input into buffer channel
        channelBuffer->copyFrom(1, 0, inputBuffer->getReadPointer(selectedInput), inputBuffer->getNumSamples());

        filterProcessor.getNextAudioBlock(channelBuffer.get());
        compProcessor.getNextAudioBlock(channelBuffer.get());

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
    compProcessor.prepareToPlay(samplesPerBlockExpected, sampleRate);
}



void MixerInput::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    /*selectedInput = comboBoxThatHasChanged->getSelectedId() - 2;
    comboboxChanged->sendChangeMessage();*/
}

void MixerInput::setSelectedInput(int input)
{
    selectedInput = input;
}
int MixerInput::getSelectedInput()
{
    return selectedInput;
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
    inputLabel.setText(juce::String("Input " + juce::String(index + 1)), juce::NotificationType::dontSendNotification);
    inputLabel.setJustificationType(juce::Justification::centred);
}

int MixerInput::getInputIndex()
{
    return inputIndex;
}

