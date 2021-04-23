/*
  ==============================================================================

    InputsControl.h
    Created: 21 Apr 2021 1:00:24pm
    Author:  DPR

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "MixerInput.h"
#include "InputPanel.h"
//==============================================================================
/*
*/
class InputsControl  : public juce::Component, public juce::ChangeListener
{
public:
    InputsControl(InputPanel& panel);
    ~InputsControl() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void getNextAudioBlock(juce::AudioBuffer<float>* inputBuffer, juce::AudioBuffer<float>* outputBuffer);
    void setDeviceManagerInfos(juce::AudioDeviceManager& devicemanager);
    void updateInputSelectors(int inputId);
    void updateInputSelectorsState();
    void changeListenerCallback(juce::ChangeBroadcaster* source);

    juce::OwnedArray<MixerInput> inputs;

private:
    void addInput(MixerInput::Mode inputMode);
    void rearrangeInputs();
    void setSelectedMixerInput(int selectedInput);

    double actualSampleRate;
    int actualSamplesPerBlockExpected;

    bool isSelectedInput = false;

    std::unique_ptr<juce::AudioBuffer<float>> mixerBuffer;
    std::unique_ptr<juce::AudioBuffer<float>> selectedInputBuffer;

    int mixerInputWidth = 100;
    int selectedMixerInput = -1;
    int numInputsChannels;

    juce::StringArray inputsChannelsName;
    juce::AudioDeviceManager* deviceManager;
    juce::Array<bool> selectedInputs;
    bool defaultInputsInitialized = false;


    InputPanel* inputPanel = 0;

    juce::TextButton addButton;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InputsControl)
};
