/*
  ==============================================================================

    PlayerProcessor.h
    Created: 21 Apr 2021 10:30:29am
    Author:  DPR

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PlayerEditor.h"
//==============================================================================
/*
*/
class PlayerProcessor  : public juce::AudioProcessor
{
public:
    PlayerProcessor();
    ~PlayerProcessor() override;

    void prepareToPlay(double newSampleRate, int newSamplesPerBlock) override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;
    void releaseResources() override;
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

private:
    int actualSamplesPerBlockExpected;
    double actualSampleRate;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlayerProcessor)
};
