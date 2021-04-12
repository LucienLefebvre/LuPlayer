/*
  ==============================================================================

    BottomComponent.cpp
    Created: 15 Mar 2021 6:43:01pm
    Author:  Lucien

  ==============================================================================
*/

#include <JuceHeader.h>
#include "BottomComponent.h"

//==============================================================================
BottomComponent::BottomComponent()
    : TabbedComponent(juce::TabbedButtonBar::Orientation::TabsAtTop)
{

    addTab("Sound Browser", juce::Colours::transparentBlack, &audioPlaybackDemo, false);
    setTabBarDepth(tabBarHeight);

    myMixer.addInputSource(&audioPlaybackDemo.transportSource, false);

}


BottomComponent::~BottomComponent()
{
}

void BottomComponent::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

}

void BottomComponent::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
    audioPlaybackDemo.setBounds(0, 0, getWidth(), getHeight() - 25);
    TabbedComponent::resized();
    getTabbedButtonBar().setBounds(getTabbedButtonBar().getBounds());
}

void BottomComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    myMixer.prepareToPlay(samplesPerBlockExpected, sampleRate);
}