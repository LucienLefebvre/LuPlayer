/*
  ==============================================================================

    PlayHead.cpp
    Created: 22 Mar 2021 3:13:36pm
    Author:  Lucien

  ==============================================================================
*/

#include <JuceHeader.h>
#include "PlayHead.h"

//==============================================================================
PlayHead::PlayHead()
{
    colour = juce::Colours::white;
}

PlayHead::~PlayHead()
{

}

void PlayHead::paint (juce::Graphics& g)
{
    g.fillAll (colour);   // clear the background
}

void PlayHead::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}

void PlayHead::setColour(juce::Colour color)
{
    colour = color;
}