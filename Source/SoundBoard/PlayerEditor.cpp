/*
  ==============================================================================

    PlayerEditor.cpp
    Created: 21 Apr 2021 10:30:59am
    Author:  DPR

  ==============================================================================
*/

#include <JuceHeader.h>
#include "PlayerEditor.h"

//==============================================================================
PlayerEditor::PlayerEditor(juce::AudioProcessor& p) : AudioProcessorEditor(&p)
{
}

PlayerEditor::~PlayerEditor()
{
}

void PlayerEditor::paint (juce::Graphics& g)
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
    g.drawText ("PlayerEditor", getLocalBounds(),
                juce::Justification::centred, true);   // draw some placeholder text
}

void PlayerEditor::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}
