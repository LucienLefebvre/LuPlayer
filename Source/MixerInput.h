/*
  ==============================================================================

    MixerInput.h
    Created: 9 Apr 2021 9:18:19am
    Author:  DPR

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "FilterProcessor.h"
#include "CompProcessor.h"
//==============================================================================
/*
*/
class MixerInput : public juce::Component,
    public juce::ComboBox::Listener,
    public juce::ChangeBroadcaster,
    public juce::Slider::Listener
{
public:
    enum Mode
    {
        Mono,
        Stereo
    };

    MixerInput(Mode mode);
    ~MixerInput() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void MixerInput::getNextAudioBlock(juce::AudioBuffer<float>* inputBuffer, juce::AudioBuffer<float>* outputBuffer);
    void MixerInput::prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void MixerInput::feedInputSelector(int channel, juce::String name, bool isSelectable);
    void MixerInput::clearInputSelector();
    void MixerInput::selectDefaultInput(int defaultInput);
    int MixerInput::getSelectedInput();
    void MixerInput::updateComboboxItemsState(int itemId, bool isEnabled);
    void MixerInput::setInputIndex(int index);
    int MixerInput::getInputIndex();

    std::unique_ptr<juce::ChangeBroadcaster> comboboxChanged;
    juce::TextButton selectButton;

    FilterProcessor filterProcessor;
    CompProcessor compProcessor;
private:
    void MixerInput::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged);
    void MixerInput::sliderValueChanged(juce::Slider* slider);
    std::unique_ptr<juce::AudioBuffer<float>> channelBuffer;
    Mode inputMode;
    int inputIndex;

    juce::Slider volumeSlider;
    juce::Slider panKnob;
    juce::ComboBox inputSelector;


    double actualSampleRate;
    int actualSamplesPerBlockExpected;

    int selectedInput = -1;

    juce::LinearSmoothedValue<float> level;
    juce::LinearSmoothedValue<float> pan;
    float panL;
    float panR;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MixerInput)
};
