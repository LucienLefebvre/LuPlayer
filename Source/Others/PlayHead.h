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
    PlayHead()
    {
        colour = juce::Colours::white;
    };

    ~PlayHead() override
    {

    };

    void PlayHead::paint(juce::Graphics& g)
    {
        g.fillAll(colour);
    };

    void PlayHead::resized()
    {

    };


    void setColour(juce::Colour color)
    {
        colour = color;
    };

private:
    juce::Colour colour;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlayHead)
};
