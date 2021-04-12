/*
  ==============================================================================

    PlayHead.h
    Created: 22 Mar 2021 3:13:36pm
    Author:  Lucien

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class PlayHead  : public juce::Component
{
public:
    PlayHead();
    ~PlayHead() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void PlayHead::setColour(juce::Colour color);

private:
    juce::Colour colour;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlayHead)
};
