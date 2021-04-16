/*
  ==============================================================================

    FilterEditor.cpp
    Created: 16 Apr 2021 1:50:43am
    Author:  DPR

  ==============================================================================
*/
#include <JuceHeader.h>
#include "FilterEditor.h"
FilterEditor::FilterEditor()
{
    colour = juce::Colours::white;
}

FilterEditor::~FilterEditor()
{

}

void FilterEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void FilterEditor::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}
