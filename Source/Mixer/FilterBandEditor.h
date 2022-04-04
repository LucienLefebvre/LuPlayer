/*
  ==============================================================================

    FilterBandEditor.h
    Created: 18 Apr 2021 6:10:08pm
    Author:  Lucien Lefebvre

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
        addMouseListener(this, true);
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

        /*addAndMakeVisible(&filterTypeSelector);
        filterTypeSelector.addItem("Bell", 1);
        filterTypeSelector.addItem("Low Shelf", 2);
        filterTypeSelector.addItem("High Shelf", 3);
        filterTypeSelector.addItem("HPF", 4);
        filterTypeSelector.addItem("LPF", 5);
        filterTypeSelector.setSelectedItemIndex(0);
        filterTypeSelector.addListener(this);*/

        addAndMakeVisible(&filterTypeButton);
        filterTypeButton.onClick = [this] {popupMenuClicked(); };

        filterTypeMenu.addItem(1, "Bell", true, false);
        filterTypeMenu.addItem(2, "Low Shelf", true, false);
        filterTypeMenu.addItem(3, "High Shelf", true, false);
        filterTypeMenu.addItem(4, "HPF", true, false);
        filterTypeMenu.addItem(5, "LPF", true, false);

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
        filterTypeButton.setBounds(0, 0, knobWidth - 24, 20);
        qSlider.setBounds(-12, filterTypeButton.getBottom(), knobWidth, knobHeight);
        frequencySlider.setBounds(-12, qSlider.getBottom(), knobWidth, knobHeight);
        gainSlider.setBounds(-12, frequencySlider.getBottom(), knobWidth, knobHeight);
    }

    void setFilterType(FilterProcessor::FilterTypes type)
    {
        filterType = type;
        switch (filterType)
        {
        case FilterProcessor::FilterTypes::Bell :
            filterTypeButton.setButtonText("Bell");
            setVisibleSlidersForHpfLpf(false);
            break;
        case FilterProcessor::FilterTypes::LowShelf:
            filterTypeButton.setButtonText("Low Shelf");
            setVisibleSlidersForHpfLpf(false);
            break;
        case FilterProcessor::FilterTypes::HighShelf:
            filterTypeButton.setButtonText("High Shelf");
            setVisibleSlidersForHpfLpf(false);
            break;
        case FilterProcessor::FilterTypes::HPF:
            filterTypeButton.setButtonText("HPF");
            setVisibleSlidersForHpfLpf(true);
            break;
        case FilterProcessor::FilterTypes::LPF:
            filterTypeButton.setButtonText("LPF");
            setVisibleSlidersForHpfLpf(true);
            break;
        }
    }
    
    void popupMenuClicked()
    {
        int result = filterTypeMenu.show();
        switch (result)
        {
        case 1:
            filterType = FilterProcessor::FilterTypes::Bell;
            filterTypeButton.setButtonText("Bell");
            setVisibleSlidersForHpfLpf(false);
            break;
        case 2:
            filterType = FilterProcessor::FilterTypes::LowShelf;
            filterTypeButton.setButtonText("Low Shelf");
            setVisibleSlidersForHpfLpf(false);
            break;
        case 3:
            filterType = FilterProcessor::FilterTypes::HighShelf;
            filterTypeButton.setButtonText("High Shelf");
            setVisibleSlidersForHpfLpf(false);
            break;
        case 4:
            filterType = FilterProcessor::FilterTypes::HPF;
            filterTypeButton.setButtonText("HPF");
            setVisibleSlidersForHpfLpf(true);
            break;
        case 5:
            filterType = FilterProcessor::FilterTypes::LPF;
            filterTypeButton.setButtonText("LPF");
            setVisibleSlidersForHpfLpf(true);
            break;

        }
        comboBoxBroadcaster.sendChangeMessage();
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


    void enableControl(bool isEnabled)
    {
        qSlider.setEnabled(isEnabled);
        frequencySlider.setEnabled(isEnabled);
        gainSlider.setEnabled(isEnabled);
        filterTypeButton.setEnabled(isEnabled);
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
    juce::ChangeBroadcaster mouseEnterBandBroacaster;
    juce::ChangeBroadcaster mouseExitBandBroadcaster;

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

    juce::PopupMenu filterTypeMenu;


    juce::TextButton filterTypeButton;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterBandEditor)
};
