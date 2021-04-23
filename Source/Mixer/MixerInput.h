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
    public juce::Slider::Listener,
    public juce::Label::Listener
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

    void getNextAudioBlock(juce::AudioBuffer<float>* inputBuffer, juce::AudioBuffer<float>* outputBuffer);
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);

    void feedInputSelector(int channel, juce::String name, bool isSelectable);
    void clearInputSelector();
    void selectDefaultInput(int defaultInput);
    void updateComboboxItemsState(int itemId, bool isEnabled);

    void setSelectedInput(int input);
    int getSelectedInput();

    void setInputIndex(int index);
    int getInputIndex();

    void setName(juce::String s);
    juce::String getName();

    void setTrimLevel(float l);
    float getTrimLevel();

    void setInputColour(juce::Colour c);
    juce::Colour getInputColour();

    std::unique_ptr<juce::ChangeBroadcaster> inputEdited;
    juce::TextButton selectButton;

    FilterProcessor filterProcessor;
    CompProcessor compProcessor;
private:
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged);
    void sliderValueChanged(juce::Slider* slider);
    void labelTextChanged(juce::Label* labelThatHasChanged);
    std::unique_ptr<juce::AudioBuffer<float>> channelBuffer;
    Mode inputMode;
    int inputIndex;


    juce::Label inputLabel;
    juce::String name;
    juce::Slider volumeSlider;
    juce::Slider panKnob;



    double actualSampleRate;
    int actualSamplesPerBlockExpected;

    int selectedInput = -1;

    juce::LinearSmoothedValue<float> level;
    juce::LinearSmoothedValue<float> trimLevel;
    juce::LinearSmoothedValue<float> pan;
    float panL;
    float panR;

    juce::Colour inputColour;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MixerInput)
};
