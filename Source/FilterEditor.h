/*
  ==============================================================================

    FilterEditor.h
    Created: 16 Apr 2021 1:50:43am
    Author:  DPR

  ==============================================================================
*/
#include <JuceHeader.h>
#include "FilterProcessor.h"
#pragma once
class FilterEditor : public juce::Component,
    public juce::Slider::Listener
{
public:
    FilterEditor();
    ~FilterEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void setEditedFilterProcessor(FilterProcessor& processor);

private:
    void addFilterBand(juce::OwnedArray<juce::Slider>* band);
    void sliderValueChanged(juce::Slider* slider) override;
    int numFilterBands = 4;
    int knobWidth = 80;
    int knobHeight = 80;

    juce::OwnedArray<juce::Slider> lowBandSliders;
    juce::OwnedArray<juce::Slider> middleLowBandSliders;
    juce::OwnedArray<juce::Slider> middleHighBandSliders;
    juce::OwnedArray<juce::Slider> highBandSliders;

    juce::OwnedArray<juce::ComboBox> filterTypeSelectors;

    juce::OwnedArray<juce::Slider>* bandPtr[4];

    FilterProcessor* editedFilterProcessor;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilterEditor)
};
