/*
  ==============================================================================

    LoudnessBar.h
    Created: 22 Mar 2021 3:57:49pm
    Author:  Lucien

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class LoudnessBar  : public juce::Component,
    public juce::Timer
{
public:
    LoudnessBar();
    ~LoudnessBar() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void LoudnessBar::setLoudness(float l);
    void LoudnessBar::timerCallback();
private:

    float shortTermLoudness = -100;
    int meterNumericDisplayHeight = 28;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LoudnessBar)
};
