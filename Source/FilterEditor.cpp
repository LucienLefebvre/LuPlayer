/*
  ==============================================================================

    FilterEditor.cpp
    Created: 16 Apr 2021 1:50:43am
    Author:  DPR

  ==============================================================================
*/
#pragma once
#include <JuceHeader.h>
#include "FilterEditor.h"
#include <cstdlib>
FilterEditor::FilterEditor()
{
    addMouseListener(this, true);
    juce::Timer::startTimer(50);
    for (auto i = 0; i < filterBandsNumber; i++)
    {
        addFilterBand(i);
    }
    filterBands[0]->setSlidersThumbColours(juce::Colours::red);
    filterBands[1]->setSlidersThumbColours(juce::Colours::green);
    filterBands[2]->setSlidersThumbColours(juce::Colours::blue);
    filterBands[3]->setSlidersThumbColours(juce::Colours::yellow);

    //Create frequency lines to plot
    for (auto i = 1; i < 10; i++)
    {
        frequencyLines.add(i * 10);
        frequencyLines.add(i * 100);
        frequencyLines.add(i * 1000);
        auto j = i * 10000;
        if (j < 20000)
        frequencyLines.add(j);
    }
}

FilterEditor::~FilterEditor()
{

}

void FilterEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    if (magnitudeArrayCreated)
        plotFrequencies(g);
}

void FilterEditor::plotFrequencies(juce::Graphics& g)
{
    int zeroDbYPosition = frequencyPlotBounds.getHeight() / 2;
    int zeroHzXPosition = frequencyPlotBounds.getPosition().getX();

    //DRAW 0db Axis
    g.setColour(juce::Colour(134, 141, 145));
    g.drawHorizontalLine(zeroDbYPosition, zeroHzXPosition, frequencyPlotBounds.getRight());

    //Draw frequencies lines
    for (auto i = 0; i < frequencyLines.size(); i++)
    {
        auto xPosition = zeroHzXPosition + getXPositionFromFrequency(frequencyLines[i]);
        g.drawVerticalLine(xPosition, 0, getHeight());
        if (frequencyLines[i] == 100)
            g.drawText("100Hz", xPosition + 2, frequencyPlotBounds.getBottom() - 10, 
                        50, 10, juce::Justification::centredLeft);
        else if (frequencyLines[i] == 1000)
            g.drawText("1kHz", xPosition + 2, frequencyPlotBounds.getBottom() - 10, 
                        50, 10, juce::Justification::centredLeft);
        if (frequencyLines[i] == 10000)
            g.drawText("10kHz", xPosition + 2, frequencyPlotBounds.getBottom() - 10, 
                        50, 10, juce::Justification::centredLeft);
    }

    //Draw dB lines
    for (auto i = 0; i < dBLines.size(); i++)
    {
        auto yPosition = zeroDbY - geYPositionFromGain(juce::Decibels::decibelsToGain(dBLines[i]));
        g.drawHorizontalLine(yPosition, zeroHzXPosition, frequencyPlotBounds.getRight());
        g.drawText(juce::String(dBLines[i]) << "dB", frequencyPlotBounds.getRight() - 30, yPosition - 11,
                        30, 10, juce::Justification::centredRight);
    }

    //Plot filter response
    int oldX = zeroHzXPosition;
    int oldY = zeroDbY - magnitudeArray[0] * zeroDbY / 12;
    for (auto i = 0; i < frequencyArray.size(); i++)
    {
        int x = zeroHzXPosition + i;
        int y = zeroDbY - magnitudeArray[i] * zeroDbY / 12;
        g.setColour(juce::Colour(40, 134, 189));
        g.drawLine(oldX, oldY, x, y, 3);
        oldX = x;
        oldY = y;
    }

}

void FilterEditor::resized()
{
    for (auto i = 0; i < filterBandsNumber; i++)
    {
        filterBands[i]->setBounds(i * knobWidth, 0, knobWidth, getHeight());
    }
    frequencyPlotXStart = filterBands.getLast()->getRight();
    frequencyPlotBounds.setBounds(frequencyPlotXStart, 0, getWidth() - frequencyPlotXStart, getHeight());
    zeroDbY = (float)getHeight() / 2;
    createMagnitudeArray();
    updateFilterGraphPoints();
    for (auto i = 0; i < filterPoints.size(); i++)
        filterPoints[i]->setPlotBounds(frequencyPlotBounds);
}

void FilterEditor::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    actualSampleRate = sampleRate;
    actualSamplesPerBlockExpected = samplesPerBlockExpected;
}

void FilterEditor::addFilterBand(int i)
{
    filterBands.add(new FilterBandEditor(i));
    addAndMakeVisible(filterBands.getLast());
    filterBands.getLast()->frequencySlider.addListener(this);
    filterBands.getLast()->qSlider.addListener(this);
    filterBands.getLast()->gainSlider.addListener(this);
    filterBands.getLast()->filterTypeSelector.addListener(this);
    filterBands.getLast()->comboBoxBroadcaster.addChangeListener(this);

    filterPoints.add(new filterGraphPoint(i));
    addAndMakeVisible(filterPoints.getLast());
    filterPoints.getLast()->setSize(filterPoints.getLast()->getPointSize(), filterPoints.getLast()->getPointSize());
    filterPoints.getLast()->mouseDraggedBroadcaster.addChangeListener(this);
    filterPoints.getLast()->mouseCtrlDraggedBroadcaster.addChangeListener(this);
    filterPoints.getLast()->mouseClickResetBroadcaster.addChangeListener(this);
    filterPoints.getLast()->setColour(i);
}

void FilterEditor::setEditedFilterProcessor(FilterProcessor& processor)
{
    editedFilterProcessor = &processor;

    for (auto i = 0; i < filterBands.size(); i++)
    {
        auto bandParams = editedFilterProcessor->getFilterParameters(i);
        filterBands[i]->frequencySlider.setValue(bandParams.frequency);
        filterBands[i]->gainSlider.setValue(juce::Decibels::gainToDecibels(bandParams.gain));
        filterBands[i]->qSlider.setValue(bandParams.Q);
        filterBands[i]->setFilterType(bandParams.type);
    }
    createMagnitudeArray();
    updateFilterGraphPoints();
}

void FilterEditor::createMagnitudeArray()
{
    juce::Array< juce::dsp::IIR::Coefficients<float>> filterCoefficients;
    filterCoefficients.clear();
    for (auto i = 0; i < 4; i++)//create an array of filters coefficients
    {
        filterCoefficients.set(i, *editedFilterProcessor->getFilterCoefs(i).state);
    }
    double arraySize = frequencyPlotBounds.getWidth();
    frequencyArray.clear();
    magnitudeArray.clear();

    for (auto i = 0; i < arraySize; i++)
    {
        //Create log frequency array to calculate magnitude
        frequencyArray.set(i, juce::mapToLog10<double>((double)1 * (i/arraySize), frequencyPlotHzStart, 20000)); 
        //calculate magnitude array
        magnitudeArray.set(i, juce::Decibels::gainToDecibels(filterCoefficients[0].getMagnitudeForFrequency(frequencyArray[i], actualSampleRate)
                                                            * filterCoefficients[1].getMagnitudeForFrequency(frequencyArray[i], actualSampleRate)
                                                            * filterCoefficients[2].getMagnitudeForFrequency(frequencyArray[i], actualSampleRate)
                                                            * filterCoefficients[3].getMagnitudeForFrequency(frequencyArray[i], actualSampleRate)));
    }
    magnitudeArrayCreated = true;
    magnitudeChanged = false;
    repaint();
}

void FilterEditor::sliderValueChanged(juce::Slider* slider)
{
    if (editedFilterProcessor != nullptr)
    {
        for (auto i = 0; i < filterBands.size(); i++)
        {
            if (slider->getParentComponent() == filterBands[i])
            {
                sendParameters(i);
            }
        }
    }
}

void FilterEditor::updateFilterGraphPoints()
{
    int zeroDbYPosition = frequencyPlotBounds.getHeight() / 2;
    for (auto i = 0; i < filterPoints.size(); i++)
    {
        float pointXposition = juce::mapFromLog10<float>(editedFilterProcessor->getFilterParameters(i).frequency, frequencyPlotHzStart, 20000) * frequencyPlotBounds.getWidth();
        filterPoints[i]->setTopLeftPosition(frequencyPlotBounds.getTopLeft().getX() + pointXposition - 6,
            zeroDbY - 6 - (int)(juce::Decibels::gainToDecibels(editedFilterProcessor->getFilterParameters(i).gain) * ((float)zeroDbY / 12.0f)));
    }
}

void FilterEditor::mouseDown(const juce::MouseEvent& event)
{
    for (auto i = 0; i < filterPoints.size(); i++)
    {
        if (event.eventComponent == filterPoints[i])
        {
            draggedPoint = i;
            pointDragged = true;
        }
    }
}

void FilterEditor::mouseDrag(const juce::MouseEvent& event)
{
    if (pointDragged == true && frequencyPlotBounds.contains(getMouseXYRelative()))
    {
        auto x = getMouseXYRelative().getX();
        auto y = getMouseXYRelative().getY();
        DBG(x);
        filterBands[draggedPoint]->frequencySlider.setValue(getFrequencyFromXPosition(x));
        filterBands[draggedPoint]->gainSlider.setValue(getGainFromYPosition(y));
    }
}

void FilterEditor::mouseUp(const juce::MouseEvent& event)
{
    pointDragged = false;
}

void FilterEditor::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    for (auto i = 0; i < filterPoints.size(); i++)
    {
        if (source == &filterPoints[i]->mouseDraggedBroadcaster)
        {
            //set the value, (in arguments we get the centre of the point)
            //filterBands[i]->frequencySlider.setValue(getFrequencyFromXPosition(filterPoints[i]->getX() + filterPoints[i]->getWidth() / 2));
            //filterBands[i]->gainSlider.setValue(getGainFromYPosition((float)filterPoints[i]->getY()));

        }
        else if (source == &filterPoints[i]->mouseCtrlDraggedBroadcaster)
        {
            filterBands[i]->qSlider.setValue(filterBands[i]->qSlider.getValue() * (1 - (float)filterPoints[0]->getDragYDistance() / 100.0f));
        }
        else if (source == &filterPoints[i]->mouseClickResetBroadcaster)
        {
            filterBands[i]->gainSlider.setValue(0);
            switch (i)
            {
            case 0 :
                filterBands[i]->frequencySlider.setValue(100);
                break;
            case 1:
                filterBands[i]->frequencySlider.setValue(400);
                break;
            case 2:
                filterBands[i]->frequencySlider.setValue(2000);
                break;
            case 3:
                filterBands[i]->frequencySlider.setValue(8000);
                break;
            }
        }
        else if (source == &filterBands[i]->comboBoxBroadcaster)
        {

            sendParameters(i);
        }
    }
}

void FilterEditor::sendParameters(int i)
{
    FilterProcessor::FilterParameters params;
    params.frequency = filterBands[i]->frequencySlider.getValue();
    params.gain = juce::Decibels::decibelsToGain(filterBands[i]->gainSlider.getValue());
    params.Q = filterBands[i]->qSlider.getValue();
    params.type = filterBands[i]->getFilterType();
    editedFilterProcessor->setFilterParameters(i, params);
    magnitudeChanged = true;
    updateFilterGraphPoints();
}

void FilterEditor::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{

}

float FilterEditor::getFrequencyFromXPosition(int xPosition)
{
    //used to get the frequency corresponding to the x position in the eq plot
    // it takes the relative position in the filterEditor component and map it
    float frequency = juce::mapToLog10<float>((float)(xPosition - frequencyPlotXStart) / frequencyPlotBounds.getWidth(), frequencyPlotHzStart, 20000.0f);
    return frequency;
}

int FilterEditor::getXPositionFromFrequency(float frequency)
{
   return juce::mapFromLog10<float>(frequency, frequencyPlotHzStart, 20000) * frequencyPlotBounds.getWidth();
}

float FilterEditor::getGainFromYPosition(int yPosition)
{
    //same as above but for the gain
    float gain = (zeroDbY - yPosition) / 12.0f;
    DBG(gain);
    return gain;
}

int FilterEditor::geYPositionFromGain(float gain)
{
    return (int)(juce::Decibels::gainToDecibels(gain) * ((float)zeroDbY / 12.0f));
}

void FilterEditor::timerCallback()
{
    if (magnitudeChanged)
    createMagnitudeArray();
}

