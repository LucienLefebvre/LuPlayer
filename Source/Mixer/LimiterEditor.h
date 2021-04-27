/*
  ==============================================================================

    LimiterEditor.h
    Created: 27 Apr 2021 3:36:10pm
    Author:  DPR

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CompProcessor.h"
//==============================================================================
/*
*/
class LimiterEditor  : public juce::Component
{
public:
    LimiterEditor()
    {
        // In your constructor, you should add any child components, and
        // initialise any special settings that your component needs.

    }

    ~LimiterEditor() override
    {
    }

    void paint (juce::Graphics& g) override
    {
        /* This demo code just fills the component's background and
           draws some placeholder text to get you started.

           You should replace everything in this method with your own
           drawing code..
        */

        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

        g.setColour (juce::Colours::grey);
        g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

        g.setColour (juce::Colours::white);
        g.setFont (14.0f);
        g.drawText ("LimiterEditor", getLocalBounds(),
                    juce::Justification::centred, true);   // draw some placeholder text
    }

    void resized() override
    {
        // This method is where you should set the bounds of any child
        // components that your component contains..

    }

    void setEditedCompProcessor(CompProcessor& processor)
    {
        editedCompProcessor = &processor;

    }

private:
    CompProcessor* editedCompProcessor = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LimiterEditor)
};
