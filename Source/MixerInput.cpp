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

    addAndMakeVisible(&inputSelector);
    inputSelector.addListener(this);

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
    volumeSlider.setBounds(0, inputSelector.getHeight(), getWidth(), getHeight() - inputSelector.getHeight());

}

void MixerInput::getNextAudioBlock(juce::AudioBuffer<float>* buffer)
{
    buffer->applyGain(juce::Decibels::decibelsToGain(volumeSlider.getValue()));
}

void MixerInput::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    actualSampleRate = sampleRate;
    actualSamplesPerBlockExpected = samplesPerBlockExpected;
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

void MixerInput::setInputNumber(int number)
{
    inputNumber = number;
}

int MixerInput::getSelectedInput()
{
    return selectedInput;
}

void MixerInput::updateComboboxItemsState(int itemId, bool isEnabled)
{
    inputSelector.setItemEnabled(itemId, isEnabled);
}