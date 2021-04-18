/*
  ==============================================================================

    FilterEditor.h
    Created: 16 Apr 2021 1:50:43am
    Author:  DPR

  ==============================================================================
*/
#pragma once
#include <JuceHeader.h>
#include "FilterProcessor.h"
#include "filterGraphPoint.h"
#include "FilterBandEditor.h"

#pragma once
class FilterEditor : public juce::Component,
    public juce::Slider::Listener,
    public juce::MouseListener,
    public juce::ChangeListener,
    public juce::ComboBox::Listener,
    public juce::Timer
{
public:
    FilterEditor();
    ~FilterEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void setEditedFilterProcessor(FilterProcessor& processor);
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);

private:
    void addFilterBand(int i);

    void timerCallback();
    void sliderValueChanged(juce::Slider* slider) override;
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged);
    void changeListenerCallback(juce::ChangeBroadcaster* source);

    void createMagnitudeArray();
    void plotFrequencies(juce::Graphics& g);
    void updateFilterGraphPoints();
    void mouseDrag(const juce::MouseEvent& event);

    float getFrequencyFromXPosition(int xPosition);
    int getXPositionFromFrequency(float frequency);
    float getGainFromYPosition(int yPosition);
    int geYPositionFromGain(float gain);

    void sendParameters(int filterBand);

    double actualSampleRate;
    int actualSamplesPerBlockExpected;

    FilterProcessor* editedFilterProcessor = 0;
    int numFilterBands = 4;
    int knobWidth = 80;
    int knobHeight = 80;

    juce::OwnedArray<FilterBandEditor> filterBands;
    juce::OwnedArray<filterGraphPoint> filterPoints;

    juce::Array<double> frequencyArray;
    juce::Array<double> magnitudeArray;
    juce::Rectangle<int> frequencyPlotBounds;
    bool magnitudeArrayCreated = false;
    int frequencyPlotXStart = 0;
    float zeroDbY;
    int filterBandsNumber = 4;
    float frequencyPlotHzStart = 40;
    bool magnitudeChanged = false;
    juce::Array<float> frequencyLines;
    juce::Array<float> dBLines{ -9.0f, -6.0f, -3.0f, 0.0f, 3.0f, 6.0f, 9.0f };
    enum SliderType
    {
        GAIN,
        FREQUENCY,
        Q
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilterEditor)
};
