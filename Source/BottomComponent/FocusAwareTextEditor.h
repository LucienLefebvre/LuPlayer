/*
* Thanks to "connorreviere" from Juce forum for this
  ==============================================================================

    FocusAwareTextEditor.h
    Created: 9 Apr 2022 2:12:52pm
    Author:  Lucien Lefebvre

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class FocusAwareTextEditor : public juce::TextEditor
{
public :
    FocusAwareTextEditor() = default;

    void parentHierarchyChanged() override
    {
        if (getParentComponent())
        {
            juce::Desktop::getInstance().addGlobalMouseListener(this);
        }
        else
        {
            juce::Desktop::getInstance().removeGlobalMouseListener(this);
        }
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        if (getScreenBounds().contains(e.getMouseDownScreenPosition()))
        {
            // Delegate mouse clicks inside the editor to the TextEditor
            // class so as to not break its functionality.
            juce::TextEditor::mouseDown(e);
            textFocusGainedBroadcaster->sendChangeMessage();
        }
        else
        {
            textFocusLostBroadcaster->sendChangeMessage();
        }
    }

    void mouseDrag(const juce::MouseEvent& e) override
    {
        if (getScreenBounds().contains(e.getMouseDownScreenPosition()))
        {
            juce::TextEditor::mouseDrag(e);
            textFocusGainedBroadcaster->sendChangeMessage();
        }
    }
    void escapePressed() override
    {
        textFocusLostBroadcaster->sendChangeMessage();
    }

    std::unique_ptr<juce::ChangeBroadcaster> textFocusLostBroadcaster = std::make_unique<juce::ChangeBroadcaster>();
    std::unique_ptr<juce::ChangeBroadcaster> textFocusGainedBroadcaster = std::make_unique<juce::ChangeBroadcaster>();
};