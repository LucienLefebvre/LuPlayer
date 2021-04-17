/*
  ==============================================================================

    FilterEditor.h
    Created: 16 Apr 2021 1:50:43am
    Author:  DPR

  ==============================================================================
*/
#include <JuceHeader.h>
#include "FilterProcessor.h"
#include "filterGraphPoint.h"
#pragma once
class FilterEditor : public juce::Component,
    public juce::Slider::Listener,
    public juce::MouseListener,
    public juce::ChangeListener
{
public:
    FilterEditor();
    ~FilterEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void setEditedFilterProcessor(FilterProcessor& processor);
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);

private:
    void addFilterBand(juce::OwnedArray<juce::Slider>* band);
    void sliderValueChanged(juce::Slider* slider) override;
    void createMagnitudeArray();
    void plotFrequencies(juce::Graphics& g);
    void updateFilterGraphPoints();
    void mouseDrag(const juce::MouseEvent& event);
    void changeListenerCallback(juce::ChangeBroadcaster* source);
    float getFrequencyFromXPosition(int xPosition);
    float getGainFromYPosition(int yPosition);
    double actualSampleRate;
    int actualSamplesPerBlockExpected;

    FilterProcessor* editedFilterProcessor = 0;
    int numFilterBands = 4;
    int knobWidth = 80;
    int knobHeight = 80;

    juce::OwnedArray<juce::Slider> lowBandSliders;
    juce::OwnedArray<juce::Slider> middleLowBandSliders;
    juce::OwnedArray<juce::Slider> middleHighBandSliders;
    juce::OwnedArray<juce::Slider> highBandSliders;
    juce::OwnedArray<juce::ComboBox> filterTypeSelectors;

    juce::OwnedArray<filterGraphPoint> filterPoints;

    juce::Array<double> frequencyArray;
    juce::Array<double> magnitudeArray;
    juce::Rectangle<int> frequencyPlotBounds;
    bool magnitudeArrayCreated = false;
    int frequencyPlotXStart = 0;
    int zeroDbY;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilterEditor)
};
