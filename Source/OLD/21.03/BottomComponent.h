/*
  ==============================================================================

    BottomComponent.h
    Created: 15 Mar 2021 6:43:01pm
    Author:  Lucien

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "AudioBrowser.h"
#include "Recorder.h"

//==============================================================================
/*
*/
class BottomComponent  : public juce::TabbedComponent,
    public juce::ChangeListener
{
public:
    BottomComponent();

    ~BottomComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void BottomComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void BottomComponent::tabSelected();

    AudioPlaybackDemo audioPlaybackDemo;
    Recorder recorderComponent;
    juce::MixerAudioSource myMixer;


private:
    void BottomComponent::changeListenerCallback(juce::ChangeBroadcaster* source);
    void BottomComponent::currentTabChanged(int newCurrentTabIndex, const juce::String& newCurrentTabName);

    juce::TextButton soundBrowserButton;
    int tabBarHeight = 25;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BottomComponent)
};
