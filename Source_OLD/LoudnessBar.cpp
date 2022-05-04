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
    juce::Timer::startTimer(50);
}

LoudnessBar::~LoudnessBar()
{

}

void LoudnessBar::paint (juce::Graphics& g)
{

    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
    float minusThreeDbLoudness = 0.96066;
    juce::NormalisableRange<float>loudnessMeterRange(-300, 1, 0.001, 3, false);
    float loudnessNormalized = loudnessMeterRange.convertTo0to1(shortTermLoudness);

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
}

void LoudnessBar::setLoudness(float l)
{
    shortTermLoudness = l;
}

void LoudnessBar::timerCallback()
{
    repaint();
}

