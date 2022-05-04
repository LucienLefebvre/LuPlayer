/*
  ==============================================================================

    DeesserEditor.h
    Created: 27 Apr 2021 3:36:21pm
    Author:  DPR

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CompProcessor.h"
//==============================================================================
/*
*/
class DeesserEditor  : public juce::Component
{
public:
    DeesserEditor()
    {
    }

    ~DeesserEditor() override
    {
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

        g.setColour (juce::Colours::grey);
        g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

        g.setColour (juce::Colours::white);
        g.setFont (14.0f);
        g.drawText ("DeesserEditor", getLocalBounds(),
                    juce::Justification::centred, true);   // draw some placeholder text
    }

    void resized() override
    {

    }

    void setEditedCompProcessor(CompProcessor& processor)
    {
        editedCompProcessor = &processor;

    }
private:
    CompProcessor* editedCompProcessor = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DeesserEditor)
};
