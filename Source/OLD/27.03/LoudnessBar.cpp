/*
  ==============================================================================

    LoudnessMeter.cpp
    Created: 22 Mar 2021 3:57:49pm
    Author:  Lucien

  ==============================================================================
*/

#include <JuceHeader.h>
#include "LoudnessBar.h"

//==============================================================================
LoudnessBar::LoudnessBar()
{
    juce::Timer::startTimer(25);
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

LoudnessBar::~LoudnessBar()
{

}

void LoudnessBar::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
    float minusThreeDbLoudness = 0.96066;
    juce::NormalisableRange<float>loudnessMeterRange(-300, 1, 0.001, 3, false);
    float loudnessNormalized = loudnessMeterRange.convertTo0to1(shortTermLoudness);

    //DBG(loudnessMeterRange.convertTo0to1(-23.));
    // - 23 DBG normalized = 0.779363


    // DRAW LOUDNESS METER
    int loudnessMeterHeight = loudnessNormalized * (getHeight() - meterNumericDisplayHeight - 15);
    if (shortTermLoudness >= -23.)
        g.setColour(juce::Colours::orange);
    else
        g.setColour(juce::Colour(40, 134, 189));
    g.fillRoundedRectangle(0, getHeight() - getHeight() + (getHeight() - loudnessMeterHeight) - meterNumericDisplayHeight, 25, loudnessMeterHeight, 5);
    if (shortTermLoudness >= -23.)
    {
        float minusTwentyThreeDbLoudness = 0.779363;
        int loudnessMeterHeightMinusEighteen = minusTwentyThreeDbLoudness * (getHeight() - meterNumericDisplayHeight - 15);
        g.setColour(juce::Colour(40, 134, 189));
        g.fillRoundedRectangle(0, getHeight() - getHeight() + (getHeight() - loudnessMeterHeightMinusEighteen) - meterNumericDisplayHeight, 25, loudnessMeterHeightMinusEighteen, 5);
    }
}

void LoudnessBar::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}

void LoudnessBar::setLoudness(float l)
{

    shortTermLoudness = l;



}

void LoudnessBar::timerCallback()
{
    repaint();
}

