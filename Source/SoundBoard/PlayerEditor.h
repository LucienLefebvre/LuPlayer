/*
  ==============================================================================

    PlayerEditor.h
    Created: 21 Apr 2021 10:30:59am
    Author:  DPR

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PlayerProcessor.h"

//==============================================================================
/*
*/
class PlayerEditor  : public juce::AudioProcessorEditor
{
public:
    enum PanelMode
    {
        Normal,
        Narrow,
        Simple
    };
    PlayerEditor(juce::AudioProcessor& p);
    ~PlayerEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlayerEditor)
};
