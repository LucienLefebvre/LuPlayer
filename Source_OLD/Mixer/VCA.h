/*
  ==============================================================================

    VCA.h
    Created: 23 Apr 2021 1:39:12pm
    Author:  DPR

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class VCA  : public juce::Component, public juce::Slider::Listener
{
public:
    VCA()
    {
        addAndMakeVisible(&vcaLabel);
        vcaLabel.setColour(juce::Label::ColourIds::textColourId, juce::Colours::red);
        vcaLabel.setText("VCA", juce::NotificationType::dontSendNotification);
        vcaLabel.setJustificationType(juce::Justification::centred);

        addAndMakeVisible(volumeSlider);
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
        volumeSlider.setColour(juce::Slider::ColourIds::trackColourId, juce::Colours::red);
        level.setValue(juce::Decibels::decibelsToGain(volumeSlider.getValue()));

    }

    ~VCA() override
    {
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
        g.setColour(juce::Colours::lightgrey);
        g.setOpacity(0.8f);
        g.drawLine(getWidth(), 0, getWidth(), getHeight(), 1);
    }

    void resized() override
    {
        vcaLabel.setBounds(0, 0, getWidth(), 20);
        volumeSlider.setBounds(0, vcaLabel.getBottom() + 20, getWidth(), getHeight() - vcaLabel.getHeight() - 40);

    }
    float getLevel()
    {
        return level.getCurrentValue();
    }
    void updateLevel()
    {
        level.getNextValue();
    }
private:
    void sliderValueChanged(juce::Slider* slider)
    {
        if (slider == &volumeSlider)
        {
            level.setTargetValue(juce::Decibels::decibelsToGain(volumeSlider.getValue()));
        }
    }
    juce::LinearSmoothedValue<float> level;
    juce::Label vcaLabel;
    juce::Slider volumeSlider;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VCA)
};
