/*
  ==============================================================================

    BottomComponent.h
    Created: 15 Mar 2021 6:43:01pm
    Author:  Lucien Lefebvre

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "AudioBrowser.h"
#include "Recorder.h"
#include "../Mixer/Mixer.h"
#include "ClipEffect.h"
#include "../BottomComponent/ClipEditor.h"
#include "../Settings/KeyMapper.h"
#include "TextEditorTab.h"
#include "LuplayerUpload.h"

#if RFBUILD
#include "../RF/DataBaseBrowser.h"
#include "../RF/DistantDataBaseBrowser.h"
#include "../RF/DataBaseImport.h"
#endif
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
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void tabSelected();
    void stopCue();
    void setStart();
    void setStop();
    void spaceBarPressed();
    void setOrDeleteStart(bool setOrDelete);
    void setOrDeleteStop(bool setOrDelete);
    AudioPlaybackDemo audioPlaybackDemo;
    Recorder recorderComponent;
    ClipEditor clipEditor;
    ClipEffect clipEffect;
    TextEditorTab textEditor;
    LuplayerUpload luplayerUpload;

    juce::MixerAudioSource myMixer;

    juce::ChangeBroadcaster* cuePlay;

#if RFBUILD
    DataBaseBrowser dbBrowser;
    DataBaseImport dbImport;
    DistantDataBaseBrowser distantDbBrowser;
#endif

private:
    void changeListenerCallback(juce::ChangeBroadcaster* source);
    void currentTabChanged(int newCurrentTabIndex, const juce::String& newCurrentTabName);

    juce::TextButton soundBrowserButton;
    int tabBarHeight = 28;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BottomComponent)
};
