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
class BottomComponent  : public juce::TabbedComponent
{
public:
    BottomComponent();

    ~BottomComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void BottomComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate);

    AudioPlaybackDemo audioPlaybackDemo;
    Recorder recorderComponent;
    juce::MixerAudioSource myMixer;


private:
    juce::TextButton soundBrowserButton;
    int tabBarHeight = 25;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BottomComponent)
};
