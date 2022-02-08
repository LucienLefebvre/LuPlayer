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
#include "DataBaseBrowser.h"
#include "DistantDataBaseBrowser.h"
#include "DataBaseImport.h"
#include "Mixer/Mixer.h"
#include "nanodbc/nanodbc.h"
#include "ClipEffect.h"
#include "Clip Editor/ClipEditor.h"
#include "Settings/KeyMapper.h"
//==============================================================================
/*
*/
class BottomComponent : public juce::TabbedComponent,
    public juce::ChangeListener,
    public juce::ChangeBroadcaster
{
public:
    BottomComponent();

    ~BottomComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void BottomComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void BottomComponent::tabSelected();
    void BottomComponent::stopCue();
    bool keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent, KeyMapper* keyMapper);
    void setStart();
    void setStop();
    void spaceBarPressed();
    void setOrDeleteStart(bool setOrDelete);
    void setOrDeleteStop(bool setOrDelete);
    AudioPlaybackDemo audioPlaybackDemo;
    Recorder recorderComponent;
    DataBaseBrowser dbBrowser;
    DataBaseImport dbImport;
    DistantDataBaseBrowser distantDbBrowser;
    ClipEditor clipEditor;
    ClipEffect clipEffect;
    juce::MixerAudioSource myMixer;
    juce::Label noDbLabel{ "No DataBase Found" };

    nanodbc::connection conn;

    juce::ChangeBroadcaster* cuePlay;

private:
    void BottomComponent::changeListenerCallback(juce::ChangeBroadcaster* source);
    void BottomComponent::currentTabChanged(int newCurrentTabIndex, const juce::String& newCurrentTabName);

    juce::TextButton soundBrowserButton;
    int tabBarHeight = 25;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BottomComponent)
};
