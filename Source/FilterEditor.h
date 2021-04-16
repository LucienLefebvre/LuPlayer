/*
  ==============================================================================

    FilterEditor.h
    Created: 16 Apr 2021 1:50:43am
    Author:  DPR

  ==============================================================================
*/
#include <JuceHeader.h>
#pragma once
class FilterEditor : public juce::Component
{
public:
    FilterEditor();
    ~FilterEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;


private:
    juce::Colour colour;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilterEditor)
};
