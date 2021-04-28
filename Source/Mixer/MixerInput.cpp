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
MixerInput::MixerInput(Mode mode) : compProcessor((mode == Mono) ? CompProcessor::Mode::Mono : CompProcessor::Mode::Stereo)
{
    switch (mode) 
    {
    case Mono :
        inputMode = Mono;
        inputMeter = std::make_unique<Meter>(Meter::Mode::Mono_ReductionGain);
        outputMeter = std::make_unique<Meter>(Meter::Mode::Mono_ReductionGain);
        break;
    case Stereo:
        inputMode = Stereo;
        inputMeter = std::make_unique<Meter>(Meter::Mode::Stereo_ReductionGain);
        outputMeter = std::make_unique<Meter>(Meter::Mode::Stereo_ReductionGain);
        break;
    }

    inputColour = juce::Colour(juce::uint8(50), 62, 68, 1.0f);

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
    inputLabel.addListener(this);
    inputEdited = std::make_unique<juce::ChangeBroadcaster>();

    addAndMakeVisible(inputMeter.get());

    trimLevel.setValue(1.0);
}

MixerInput::~MixerInput()
{
}

void MixerInput::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
    //g.setColour(inputColour);
    //g.fillRect(0, 0, getWidth(), getHeight());
    g.setColour(juce::Colours::lightgrey);
    g.setOpacity(0.8f);
    g.drawLine(getWidth(), 0, getWidth(), getHeight(), 1);
}

void MixerInput::resized()
{
    inputLabel.setBounds(0, 0, getWidth(), 20);
    panKnob.setBounds(0, inputLabel.getBottom(), getWidth(), 25);
    selectButton.setBounds(15, getBottom() - 20, 70, 20);

    volumeSlider.setBounds(0, panKnob.getBottom(), getWidth() / 2, getHeight() - panKnob.getBottom() - inputLabel.getHeight());
    inputMeter->setBounds(getWidth() / 2, panKnob.getBottom() + 5, getWidth() / 2 - 1, getHeight() - panKnob.getBottom() - inputLabel.getHeight() - 10);
    
}

void MixerInput::getNextAudioBlock(juce::AudioBuffer<float>* inputBuffer, juce::AudioBuffer<float>* outputBuffer)
{
    if (selectedInput != -1)
    {
        channelBuffer->clear();//clear buffer channel

        trimLevel.getNextValue();

        channelBuffer->copyFrom(0, 0, inputBuffer->getReadPointer(selectedInput), inputBuffer->getNumSamples(), trimLevel.getCurrentValue());//copy selected input into buffer channel
        if (inputMode == Stereo)
            channelBuffer->copyFrom(1, 0, inputBuffer->getReadPointer(selectedInput + 1), inputBuffer->getNumSamples(), trimLevel.getCurrentValue());

        inputMeter->measureBlock(channelBuffer.get());

        filterProcessor.getNextAudioBlock(channelBuffer.get());
        compProcessor.getNextAudioBlock(channelBuffer.get());

        inputMeter->setReductionGain(compProcessor.getGeneralReductionDB());
        outputMeter->measureBlock(channelBuffer.get());
        level.getNextValue();//compute next fader level value
        pan.getNextValue();
        panL = juce::jmin(1 - pan.getCurrentValue(), 1.0f);
        panR = juce::jmin(1 + pan.getCurrentValue(), 1.0f);
        outputBuffer->addFrom(0, 0, *channelBuffer, 0, 0, channelBuffer->getNumSamples(), level.getCurrentValue() * panL * vcaLevel);//copy into output buffer * gain 
        if (inputMode == Mono)
            outputBuffer->addFrom(1, 0, *channelBuffer, 0, 0, channelBuffer->getNumSamples(), level.getCurrentValue() * panR * vcaLevel);
        else if (inputMode == Stereo)
            outputBuffer->addFrom(1, 0, *channelBuffer, 1, 0, channelBuffer->getNumSamples(), level.getCurrentValue() * panR * vcaLevel);
    }
}

void MixerInput::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    actualSampleRate = sampleRate;
    actualSamplesPerBlockExpected = samplesPerBlockExpected;
    channelBuffer = std::make_unique<juce::AudioBuffer<float>>(2, actualSamplesPerBlockExpected);
    channelBuffer->setSize(2, actualSamplesPerBlockExpected, false, true, false);

    inputMeter->prepareToPlay(samplesPerBlockExpected, sampleRate);
    outputMeter->prepareToPlay(samplesPerBlockExpected, sampleRate);
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
    DBG("selected input" << input);
    selectedInput = input;
}
int MixerInput::getSelectedInput()
{
    return selectedInput;
}

void MixerInput::setName(juce::String s)
{
    name = s;
    inputLabel.setText(name, juce::NotificationType::dontSendNotification);
}

juce::String MixerInput::getName()
{
    return name;
}

void MixerInput::setTrimLevel(float l)
{
    trimLevel.setTargetValue(l);
}

float MixerInput::getTrimLevel()
{
    return trimLevel.getTargetValue();
}

void MixerInput::setVCAAssigned(bool isAssigned)
{
    vcaAssigned = isAssigned;
    if (!vcaAssigned)
    {
        volumeSlider.setColour(juce::Slider::ColourIds::thumbColourId, getLookAndFeel().findColour(juce::Slider::ColourIds::thumbColourId));
        vcaLevel = 1.0f;
    }
    else
    {
        volumeSlider.setColour(juce::Slider::ColourIds::thumbColourId, juce::Colours::red);
    }
}

bool MixerInput::isVCAAssigned()
{
    return vcaAssigned;
}

void MixerInput::setVCALevel(float l)
{
    vcaLevel = l;
}

void MixerInput::setInputColour(juce::Colour c)
{
    inputColour = c;
    inputLabel.setColour(juce::Label::ColourIds::backgroundColourId, inputColour);
    //volumeSlider.setColour(juce::Slider::ColourIds::trackColourId, inputColour);
    //selectButton.setColour(juce::TextButton::ColourIds::buttonColourId, inputColour);
    //inputMeter->setColour(c);
    repaint();
}

juce::Colour MixerInput::getInputColour()
{
    return inputColour;
}
void MixerInput::labelTextChanged(juce::Label* labelThatHasChanged)
{
    if (labelThatHasChanged == &inputLabel)
        name = inputLabel.getText();
    inputEdited->sendChangeMessage();
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
    inputLabel.setText(juce::String("Input " + juce::String(index + 1)), juce::NotificationType::sendNotification);
    inputLabel.setJustificationType(juce::Justification::centred);
}

int MixerInput::getInputIndex()
{
    return inputIndex;
}

MixerInput::Mode MixerInput::getInputMode()
{
    return inputMode;
}

MixerInput::InputParams MixerInput::getInputParams()
{
    MixerInput::InputParams params;
    params.mode = inputMode;
    params.name = name;
    params.selectedInput = selectedInput;
    params.inputIndex = inputIndex;
    params.level = level.getTargetValue();
    params.trimLevel = trimLevel.getTargetValue();
    params.colour = inputColour;
    params.vcaAssigned = vcaAssigned;
    return params;
}