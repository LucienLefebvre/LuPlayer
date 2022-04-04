/*
  ==============================================================================

    MixerInput.h
    Created: 9 Apr 2021 9:18:19am
    Author:  Lucien Lefebvre

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "FilterProcessor.h"
#include "CompProcessor.h"
#include "Meter.h"
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

    struct InputParams
    {
        Mode mode;
        juce::String name;
        int selectedInput;
        int inputIndex;
        float level;
        float trimLevel;
        bool vcaAssigned;
        juce::Colour colour;
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

    InputParams getInputParams();

    Mode getInputMode();

    void setSelectedInput(int input);
    int getSelectedInput();

    void setInputIndex(int index);
    int getInputIndex();

    void setName(juce::String s);
    juce::String getName();

    void setTrimLevel(float l);
    float getTrimLevel();

    void setVCAAssigned(bool isAssigned);
    bool isVCAAssigned();
    void setVCALevel(float l);
    void setInputColour(juce::Colour c);
    juce::Colour getInputColour();

    std::unique_ptr<juce::ChangeBroadcaster> inputEdited;
    juce::TextButton selectButton;

    FilterProcessor filterProcessor;
    CompProcessor compProcessor;
    std::unique_ptr<Meter> inputMeter;
    std::unique_ptr<Meter> outputMeter;
private:
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged);
    void sliderValueChanged(juce::Slider* slider);
    void labelTextChanged(juce::Label* labelThatHasChanged);

    double actualSampleRate;
    int actualSamplesPerBlockExpected;

    std::unique_ptr<juce::AudioBuffer<float>> channelBuffer;
    Mode inputMode;
    int inputIndex;
    int selectedInput = -1;
    bool vcaAssigned = false;
    juce::Colour inputColour;

    juce::Label inputLabel;
    juce::String name;
    juce::Slider volumeSlider;
    juce::Slider panKnob;

    juce::LinearSmoothedValue<float> level;
    juce::LinearSmoothedValue<float> trimLevel;
    juce::LinearSmoothedValue<float> pan;
    float vcaLevel = 1.0f;
    float panL;
    float panR;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MixerInput)
};
