/*
  ==============================================================================

    Mixer.h
    Created: 3 Apr 2021 12:32:50pm
    Author:  DPR

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "RemoteInput.h"
#include "MixerInput.h"
//==============================================================================
/*
*/
class Mixer  : public juce::Component,
    public juce::ChangeListener
{
public:
    Mixer();
    ~Mixer() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    //RemoteInput remoteInput1;
    void Mixer::prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void Mixer::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill);
    void Mixer::setInputBuffer(const juce::AudioSourceChannelInfo& inputBuffer);
    juce::AudioBuffer<float>* Mixer::getOutputBuffer();
    void Mixer::setDeviceManagerInfos(juce::AudioDeviceManager& devicemanager);
    void Mixer::updateInputSelectors();
    void Mixer::updateInputSelectorsState();
    void Mixer::changeListenerCallback(juce::ChangeBroadcaster* source);

    juce::OwnedArray<MixerInput> inputs;

private:
    void Mixer::addInput(MixerInput::Mode inputMode);

    std::unique_ptr<juce::AudioBuffer<float>> mixerBuffer;

    double actualSampleRate;
    int actualSamplesPerBlockExpected;

    int mixerInputWidth = 100;
    
    int numInputsChannels;
    juce::StringArray inputsChannelsName;
    juce::AudioDeviceManager* deviceManager;
    juce::Array<bool> selectedInputs;
    bool defaultInputsInitialized = false;
   
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Mixer)
};
