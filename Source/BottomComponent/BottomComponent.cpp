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

#if RFBUILD
    addTab("Netia DataBase", getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), &dbBrowser, false);
    addTab("Distant DataBase", getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), &distantDbBrowser, false);
    addTab("Netia Database Import", getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), &dbImport, false);
    myMixer.addInputSource(&dbBrowser.resampledSource, false);
    myMixer.addInputSource(&dbImport.resampledSource, false);
    myMixer.addInputSource(&distantDbBrowser.resampledSource, false);    
    setCurrentTabIndex(5);
#endif
}


BottomComponent::~BottomComponent()
{
    delete cuePlay;
}

void BottomComponent::paint (juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void BottomComponent::resized()
{
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