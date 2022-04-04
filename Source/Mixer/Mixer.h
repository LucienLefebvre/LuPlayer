/*
  ==============================================================================

    Mixer.h
    Created: 3 Apr 2021 12:32:50pm
    Author:  Lucien Lefebvre

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "RemoteInput.h"
#include "MixerInput.h"
#include "FilterEditor.h"
#include "CompEditor.h"
#include <ff_meters\ff_meters.h>
#include "InputPanel.h"
#include "InputsControl.h"

//==============================================================================
/*
*/
class Mixer : public juce::Component, public juce::ChangeListener
{
public:
    Mixer();
    ~Mixer() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    //RemoteInput remoteInput1;
    void Mixer::prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void Mixer::getNextAudioBlock(juce::AudioBuffer<float>* inputBuffer, juce::AudioBuffer<float>* outputBuffer);
    void Mixer::setDeviceManagerInfos(juce::AudioDeviceManager* devicemanager);

    juce::OwnedArray<MixerInput> inputs;


    InputPanel inputPanel;
    InputsControl inputsControl;

private:
    void changeListenerCallback(juce::ChangeBroadcaster* source);
    double actualSampleRate;
    int actualSamplesPerBlockExpected;

    juce::AudioDeviceManager* deviceManager;
    std::unique_ptr<juce::StretchableLayoutResizerBar> verticalDividerBar;
    juce::StretchableLayoutManager myLayout;
    juce::Viewport inputsViewport;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Mixer)
};
