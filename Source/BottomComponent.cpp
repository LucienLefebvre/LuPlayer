/*
  ==============================================================================

    BottomComponent.cpp
    Created: 15 Mar 2021 6:43:01pm
    Author:  Lucien Lefebvre

  ==============================================================================
*/

#include <JuceHeader.h>
#include "BottomComponent.h"

//==============================================================================
BottomComponent::BottomComponent()
    : TabbedComponent(juce::TabbedButtonBar::Orientation::TabsAtTop)
{

    addTab("Sound Browser", getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), &audioPlaybackDemo, false);
    addTab("Clip Editor", getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), &clipEditor, false);
    addTab("Clip Effect", getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), &clipEffect, false);
    addTab("Recorder", getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), &recorderComponent, false);
    addTab("Text Editor", getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), &textEditor, false);
    setTabBarDepth(tabBarHeight);
    setCurrentTabIndex(0);
    getTabbedButtonBar().setWantsKeyboardFocus(false);
    getTabbedButtonBar().addChangeListener(this);
    myMixer.addInputSource(&audioPlaybackDemo.transportSource, false);
    myMixer.addInputSource(&recorderComponent.cueTransport, false);
    unfocusAllComponents();
    setWantsKeyboardFocus(false);
    
    recorderComponent.recordingBroadcaster->addChangeListener(this);
    audioPlaybackDemo.cuePlay->addChangeListener(this);

    recorderComponent.setName("recorder");
    audioPlaybackDemo.setName("browser");


    cuePlay = new juce::ChangeBroadcaster();
}


BottomComponent::~BottomComponent()
{
    delete cuePlay;
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
    recorderComponent.setBounds(0, 0, getWidth(), getHeight() - 25);
    textEditor.setBounds(0, 0, getWidth(), getHeight() - 30);
    TabbedComponent::resized();
    getTabbedButtonBar().setBounds(0, 0, getWidth(), 25);
}

void BottomComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    clipEffect.prepareToPlay(samplesPerBlockExpected, sampleRate);
    myMixer.prepareToPlay(samplesPerBlockExpected, sampleRate);
    recorderComponent.prepareToPlay(samplesPerBlockExpected, sampleRate);
    clipEditor.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void BottomComponent::tabSelected()
{
    unfocusAllComponents();
    setWantsKeyboardFocus(false);
}

void BottomComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    //Color tab button in red if recording
    if (source == recorderComponent.recordingBroadcaster)
    {
        if (recorderComponent.isRecording())
        {
            getTabbedButtonBar().setTabBackgroundColour(3, juce::Colours::red);
        }
        else
            getTabbedButtonBar().setTabBackgroundColour(3, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    }
    //stop the cue on the others components of the panel and send change message to maincomponent
    else if (source == audioPlaybackDemo.cuePlay)
    {
        cuePlay->sendChangeMessage();
    }
}

void BottomComponent::currentTabChanged(int newCurrentTabIndex, const juce::String& newCurrentTabName)
{

}

void BottomComponent::stopCue() //stop cues on this panel when a cue is launched on the soundplayer
{
    audioPlaybackDemo.transportSource.stop();
}

//bool BottomComponent::keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent, KeyMapper* keyMapper)
//{
//
//}

void BottomComponent::setStart()
{
    recorderComponent.setStart();
    clipEditor.setStart();
}

void BottomComponent::setStop()
{
    recorderComponent.setStart();
    clipEditor.setStop();
}

void BottomComponent::spaceBarPressed()
{
    juce::String selectedTab = getTabbedButtonBar().getCurrentTabName();
    if (selectedTab.equalsIgnoreCase("Sound Browser"))
        audioPlaybackDemo.spaceBarPressed();
    else if (selectedTab.equalsIgnoreCase("Clip Editor"))
     clipEditor.spaceBarPressed();
    else if (selectedTab.equalsIgnoreCase("Recorder"))
        recorderComponent.cueButtonClicked();
}

void BottomComponent::setOrDeleteStart(bool setOrDelete)
{
    juce::String selectedTab = getTabbedButtonBar().getCurrentTabName();
    if (selectedTab.equalsIgnoreCase("Clip Editor"))
        clipEditor.getEnveloppeEditor()->setOrDeleteStart(setOrDelete);
    else if (selectedTab.equalsIgnoreCase("Recorder"))
        recorderComponent.setOrDeleteStart(setOrDelete);
}

void BottomComponent::setOrDeleteStop(bool setOrDelete)
{
    juce::String selectedTab = getTabbedButtonBar().getCurrentTabName();
    if (selectedTab.equalsIgnoreCase("Clip Editor"))
        clipEditor.getEnveloppeEditor()->setOrDeleteStop(setOrDelete);
    else if (selectedTab.equalsIgnoreCase("Recorder"))
        recorderComponent.setOrDeleteStop(setOrDelete);
}