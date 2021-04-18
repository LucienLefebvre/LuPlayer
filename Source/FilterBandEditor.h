/*
  ==============================================================================

    FilterBandEditor.h
    Created: 18 Apr 2021 6:10:08pm
    Author:  DPR

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "FilterProcessor.h"

//==============================================================================
/*
*/
class FilterBandEditor : public juce::Component,
    public juce::ComboBox::Listener,
    public juce::ChangeBroadcaster
{
public:
    FilterBandEditor(int b) : comboBoxBroadcaster()
    {
        bandId = b;

        addAndMakeVisible(&qSlider);
        qSlider.setRange(0.1f, 10.0f);
        qSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        qSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, knobWidth, 20);
        qSlider.setSkewFactor(0.3f);
        qSlider.setNumDecimalPlacesToDisplay(1);
        qSlider.setDoubleClickReturnValue(true, 1.);
        qSlider.setColour(juce::Slider::ColourIds::textBoxOutlineColourId,
            getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        qSlider.setComponentID("Q");

        //GAIN sliders
        addAndMakeVisible(&gainSlider);
        gainSlider.setRange(-12.0f, +12.0f);
        gainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, knobWidth, 20);
        gainSlider.setNumDecimalPlacesToDisplay(1);
        gainSlider.setTextValueSuffix("dB");
        gainSlider.setDoubleClickReturnValue(true, 0.);
        gainSlider.setColour(juce::Slider::ColourIds::textBoxOutlineColourId,
            getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        gainSlider.setComponentID("GAIN");

        //FREQUENCY sliders
        addAndMakeVisible(&frequencySlider);
        frequencySlider.setRange(20.0f, 20000.0f);
        frequencySlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        frequencySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, knobWidth, 20);
        frequencySlider.setSkewFactor(0.3f);
        frequencySlider.setNumDecimalPlacesToDisplay(0);
        frequencySlider.setTextValueSuffix("Hz");
        frequencySlider.setColour(juce::Slider::ColourIds::textBoxOutlineColourId,
            getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        frequencySlider.setComponentID("FREQUENCY");

        addAndMakeVisible(&filterTypeSelector);
        filterTypeSelector.addItem("Bell", 1);
        filterTypeSelector.addItem("Low Shelf", 2);
        filterTypeSelector.addItem("High Shelf", 3);
        filterTypeSelector.addItem("HPF", 4);
        filterTypeSelector.addItem("LPF", 5);
        filterTypeSelector.setSelectedItemIndex(0);
        filterTypeSelector.addListener(this);
    }

    ~FilterBandEditor() override
    {
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    }

    void resized() override
    {
        filterTypeSelector.setBounds(0, 0, knobWidth, 20);
        qSlider.setBounds(0, filterTypeSelector.getBottom(), knobWidth, knobHeight);
        frequencySlider.setBounds(0, qSlider.getBottom(), knobWidth, knobHeight);
        gainSlider.setBounds(0, frequencySlider.getBottom(), knobWidth, knobHeight);
    }

    void setFilterType(FilterProcessor::FilterTypes type)
    {
        filterType = type;
        switch (filterType)
        {
        case FilterProcessor::FilterTypes::Bell :
            filterTypeSelector.setSelectedItemIndex(0);
            break;
        case FilterProcessor::FilterTypes::LowShelf:
            filterTypeSelector.setSelectedItemIndex(1);
            break;
        case FilterProcessor::FilterTypes::HighShelf:
            filterTypeSelector.setSelectedItemIndex(2);
            break;
        case FilterProcessor::FilterTypes::HPF:
            filterTypeSelector.setSelectedItemIndex(3);
            break;
        case FilterProcessor::FilterTypes::LPF:
            filterTypeSelector.setSelectedItemIndex(4);
            break;
        }
    }

    void setSlidersThumbColours(juce::Colour colour)
    {
        frequencySlider.setColour(juce::Slider::ColourIds::thumbColourId, colour);
        qSlider.setColour(juce::Slider::ColourIds::thumbColourId, colour);
        gainSlider.setColour(juce::Slider::ColourIds::thumbColourId, colour);
    }

    void setVisibleSlidersForHpfLpf(bool isHpfOrLpf)
    {
        if (isHpfOrLpf)
        {
            gainSlider.setVisible(false);
            qSlider.setVisible(false);
        }
        else
        {
            gainSlider.setVisible(true);
            qSlider.setVisible(true);
        }
    }

    FilterProcessor::FilterTypes getFilterType()
    {
        return filterType;
    }

    juce::Slider gainSlider;
    juce::Slider qSlider;
    juce::Slider frequencySlider;
    juce::ComboBox filterTypeSelector;

    juce::ChangeBroadcaster comboBoxBroadcaster;

private:
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
    {
        if (comboBoxThatHasChanged == &filterTypeSelector)
        {
            int comboboxId = filterTypeSelector.getSelectedId();
            switch (comboboxId)
            {
            case 1:
                filterType = FilterProcessor::FilterTypes::Bell;
                setVisibleSlidersForHpfLpf(false);
                break;
            case 2:
                filterType = FilterProcessor::FilterTypes::LowShelf;
                setVisibleSlidersForHpfLpf(false);
                break;
            case 3:
                filterType = FilterProcessor::FilterTypes::HighShelf;
                setVisibleSlidersForHpfLpf(false);
                break;
            case 4:
                filterType = FilterProcessor::FilterTypes::HPF;
                setVisibleSlidersForHpfLpf(true);
                break;
            case 5:
                filterType = FilterProcessor::FilterTypes::LPF;
                setVisibleSlidersForHpfLpf(true);
                break;

            }
            comboBoxBroadcaster.sendChangeMessage();
        }
    }

    int bandId = 0;
    int knobWidth = 80;
    int knobHeight = 80;
    FilterProcessor::FilterTypes filterType;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterBandEditor)
};
