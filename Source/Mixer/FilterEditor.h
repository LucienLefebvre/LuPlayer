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
#include "FFTAnalyser.h"
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
    void updateBypassed();
private:
    void addFilterBand(int i);

    void timerCallback();
    void sliderValueChanged(juce::Slider* slider) override;
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged);
    void changeListenerCallback(juce::ChangeBroadcaster* source);

    void updateFilterGraphPoints();
    void createMagnitudeArray();
    void plotFilterGraph(juce::Graphics& g);
    void plotMagnitudeArray(juce::Graphics& g, juce::Array<double> array, int lineSize, bool fill = false);
    void averageFFTArray();
    void plotFFT(juce::Graphics& g);

    void mouseDrag(const juce::MouseEvent& event);
    void mouseDown(const juce::MouseEvent& event);
    void mouseUp(const juce::MouseEvent& event);
    void mouseMove(const juce::MouseEvent& event);
    void mouseExit(const juce::MouseEvent& event);
    void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel);

    float getFrequencyFromXPosition(int xPosition);
    int getXPositionFromFrequency(float frequency);
    float getGainFromYPosition(int yPosition);
    int geYPositionFromGain(float gain);

    void sendParameters(int filterBand);

    double actualSampleRate;
    int actualSamplesPerBlockExpected;

    bool isFilterBypassed = false;

    FilterProcessor* editedFilterProcessor = 0;
    int numFilterBands = 4;
    int knobWidth = 66;
    int knobHeight = 80;
    int filterLabelWidth = 10;

    juce::OwnedArray<FilterBandEditor> filterBands;
    juce::OwnedArray<filterGraphPoint> filterPoints;

    juce::Array<double> frequencyArray;
    juce::Array<double> magnitudeArray;
    juce::Array<double> lowMagnitudeArray;
    juce::Array<double> lowMidMagnitudeArray;
    juce::Array<double> highMidMagnitudeArray;
    juce::Array<double> highMagnitudeArray;
    juce::Array<double> fftArray;
    juce::Rectangle<int> frequencyPlotBounds;
    bool magnitudeArrayCreated = false;
    int frequencyPlotXStart = 0;
    float zeroDbY;
    int zeroHzX;
    int filterBandsNumber = 4;
    float frequencyPlotHzStart = 40;
    bool magnitudeChanged = false;

    juce::Array<float> frequencyLines;
    juce::Array<float> dBLines{ -9.0f, -6.0f, -3.0f, 0.0f, 3.0f, 6.0f, 9.0f };

    juce::ComponentDragger dragger;
    int draggedPoint;
    bool pointDragged = false;
    bool drawPointInfo = false;
    bool drawBandInfo = false;
    int pointInfoIndex = 0;
    bool parametersChanged = false;

    int fftAverageIndex = 1;
    bool fftPlotReady = false;
    enum SliderType
    {
        GAIN,
        FREQUENCY,
        Q
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilterEditor)
};
