/*
  ==============================================================================

    FilterEditor.cpp
    Created: 16 Apr 2021 1:50:43am
    Author:  Lucien Lefebvre

  ==============================================================================
*/
#pragma once
#include <JuceHeader.h>
#include "FilterEditor.h"
#include <cstdlib>
FilterEditor::FilterEditor()
{
    editedFilterProcessor = &filterProcessor;
    addMouseListener(this, true);
    juce::Timer::startTimer(50);
    channelColour = getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId);
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

    filterEditedBroadcaster = new juce::ChangeBroadcaster();
}

FilterEditor::~FilterEditor()
{
    delete filterEditedBroadcaster;
}

void FilterEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.setColour(channelColour);
    g.setOpacity(0.2);
    g.fillRect(getBounds());
    plotFFT(g);
    if (magnitudeArrayCreated)
        plotFilterGraph(g);
}

void FilterEditor::plotFilterGraph(juce::Graphics& g)
{
    zeroDbY = frequencyPlotBounds.getHeight() / 2;
    zeroHzX = frequencyPlotBounds.getPosition().getX();

    //DRAW 0db Axis
    g.setColour(juce::Colour(134, 141, 145));
    g.drawHorizontalLine(zeroDbY, zeroHzX, frequencyPlotBounds.getRight());

    //Draw frequencies lines
    for (auto i = 0; i < frequencyLines.size(); i++)
    {
        auto xPosition = zeroHzX + getXPositionFromFrequency(frequencyLines[i]);
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
        g.drawHorizontalLine(yPosition, zeroHzX, frequencyPlotBounds.getRight());
        g.drawText(juce::String(dBLines[i]) << "dB", frequencyPlotBounds.getRight() - 30, yPosition - 11,
                        30, 10, juce::Justification::centredRight);
    }



    //Plot filters responses
    if (drawPointInfo == true && pointInfoIndex == 0)    //Low filter curve
    {
        g.setColour(filterPoints[0]->getColour());
        g.setOpacity(0.7);
        plotMagnitudeArray(g, lowMagnitudeArray, 2);
    }
    else if (drawPointInfo == true && pointInfoIndex == 1)    //Low mid filter curve
    {
        g.setColour(filterPoints[1]->getColour());
        g.setOpacity(0.7);
        plotMagnitudeArray(g, lowMidMagnitudeArray, 2);
    }
    else if (drawPointInfo == true && pointInfoIndex == 2)    //high mid filter curve
    {
        g.setColour(filterPoints[2]->getColour());
        g.setOpacity(0.7);
        plotMagnitudeArray(g, highMidMagnitudeArray, 2);
    }
    else if (drawPointInfo == true && pointInfoIndex == 3)    //high filter curve
    {
        g.setColour(filterPoints[3]->getColour());
        g.setOpacity(0.7);
        plotMagnitudeArray(g, highMagnitudeArray, 2);
    }
    //global filter curve
    if (!isFilterBypassed)
    {
        g.setColour(juce::Colour(40, 134, 189));
        g.setOpacity(1.0);
        plotMagnitudeArray(g, magnitudeArray, 3, true);
    }
    else 
    {
        g.setColour(juce::Colours::lightgrey);
        g.setOpacity(0.2);
        plotMagnitudeArray(g, magnitudeArray, 2, false);
    }




    //Draw Point info
    if (drawPointInfo == true)
    {
        //Position of the rectangle
        int xPos = filterPoints[pointInfoIndex]->getRight() + 4;
        int yPos = filterPoints[pointInfoIndex]->getBottom() + 4;
        if (xPos > getWidth() - 52)
            xPos = getWidth() - 52;
        if (yPos > getHeight() - 40)
            yPos = getHeight() - 40;
        g.setColour(filterPoints[pointInfoIndex]->getColour());
        g.setOpacity(0.15);
        g.fillRoundedRectangle(xPos, yPos,
            50, 40, 5);
        g.setColour(juce::Colours::white);


        g.drawText(juce::String(trunc(filterBands[pointInfoIndex]->frequencySlider.getValue())) << "Hz",
            xPos, yPos, 52, 20, juce::Justification::centredLeft);
        auto roundedGain = std::ceil(filterBands[pointInfoIndex]->gainSlider.getValue() * 10.0) / 10.0;
        g.drawText(juce::String(roundedGain) << "dB",
            xPos, yPos + 20, 50, 20, juce::Justification::centredLeft);
    }
    
    //Draw Sliders Label
    g.setColour(juce::Colours::white);
    g.drawText("Q", 0, 10, 10, 80, juce::Justification::centred);
    g.drawText("F", 0, 110, 10, 10, juce::Justification::centred);
    g.drawText("R", 0, 122, 10, 10, juce::Justification::centred);
    g.drawText("E", 0, 134, 10, 10, juce::Justification::centred);
    g.drawText("Q", 0, 146, 10, 10, juce::Justification::centred);
    g.drawText("G", 0, 186, 10, 10, juce::Justification::centred);
    g.drawText("A", 0, 198, 10, 10, juce::Justification::centred);
    g.drawText("I", 0, 210, 10, 10, juce::Justification::centred);
    g.drawText("N", 0, 222, 10, 10, juce::Justification::centred);
}

void FilterEditor::plotMagnitudeArray(juce::Graphics& g, juce::Array<double> array, int lineSize, bool fill)
{
    int oldX = zeroHzX;
    int oldY = zeroDbY;;
    juce::Path p;
    p.clear();
    p.startNewSubPath(oldX, oldY);
    for (auto i = 0; i < frequencyArray.size(); i++)
    {
        int x = zeroHzX + i;
        int y = zeroDbY - array[i] * zeroDbY / 12;
        p.lineTo(x, y);
    }

    p.lineTo(frequencyArray.size() + zeroHzX, 2*zeroDbY);
    p.lineTo(oldX, 2*zeroDbY);
    p.lineTo(oldX, oldY);
    g.strokePath(p, juce::PathStrokeType(3.0));
    if (fill)
    {
        g.setOpacity(0.2);
        g.fillPath(p);
    }
}

void FilterEditor::plotFFT(juce::Graphics& g)
{

}

void FilterEditor::resized()
{
    for (auto i = 0; i < filterBandsNumber; i++)
    {
        filterBands[i]->setBounds(i * knobWidth + filterLabelWidth, 0, knobWidth, getHeight());
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
    dummyFilterProcessor.prepareToPlay(actualSamplesPerBlockExpected, actualSampleRate);
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
    filterBands.getLast()->mouseEnterBandBroacaster.addChangeListener(this);
    filterBands.getLast()->mouseExitBandBroadcaster.addChangeListener(this);

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
    updateBypassed();
    createMagnitudeArray();
    updateFilterGraphPoints();
}

void FilterEditor::createMagnitudeArray()
{
    juce::Array< juce::dsp::IIR::Coefficients<float>> filterCoefficients;
    filterCoefficients.clear();
    lowMagnitudeArray.clear();
    lowMidMagnitudeArray.clear();
    highMidMagnitudeArray.clear();
    highMagnitudeArray.clear();
    for (auto i = 0; i < 4; i++)//create an array of filters coefficients
    {
        if (editedFilterProcessor != nullptr)
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
        lowMagnitudeArray.set(i, juce::Decibels::gainToDecibels(filterCoefficients[0].getMagnitudeForFrequency(frequencyArray[i], actualSampleRate)));
        lowMidMagnitudeArray.set(i, juce::Decibels::gainToDecibels(filterCoefficients[1].getMagnitudeForFrequency(frequencyArray[i], actualSampleRate)));
        highMidMagnitudeArray.set(i, juce::Decibels::gainToDecibels(filterCoefficients[2].getMagnitudeForFrequency(frequencyArray[i], actualSampleRate)));
        highMagnitudeArray.set(i, juce::Decibels::gainToDecibels(filterCoefficients[3].getMagnitudeForFrequency(frequencyArray[i], actualSampleRate)));
        magnitudeArray.set(i, lowMagnitudeArray[i] + lowMidMagnitudeArray[i] + highMidMagnitudeArray[i] + highMagnitudeArray[i]);
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
    if (editedFilterProcessor != nullptr)
    {
        int zeroDbYPosition = frequencyPlotBounds.getHeight() / 2;
        for (auto i = 0; i < filterPoints.size(); i++)
        {
            float pointXposition = juce::mapFromLog10<float>(editedFilterProcessor->getFilterParameters(i).frequency, frequencyPlotHzStart, 20000) * frequencyPlotBounds.getWidth();
            filterPoints[i]->setTopLeftPosition(frequencyPlotBounds.getTopLeft().getX() + pointXposition - 6,
                zeroDbY - 6 - (int)(juce::Decibels::gainToDecibels(editedFilterProcessor->getFilterParameters(i).gain) * ((float)zeroDbY / 12.0f)));
        }
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
    if (pointDragged == true)
    {
        auto x = getMouseXYRelative().getX();
        auto y = getMouseXYRelative().getY();
        filterBands[draggedPoint]->frequencySlider.setValue(getFrequencyFromXPosition(x));
        filterBands[draggedPoint]->gainSlider.setValue(getGainFromYPosition(y));
    }
}

void FilterEditor::mouseMove(const juce::MouseEvent& event)
{
    for (auto i = 0; i < filterPoints.size(); i++)
    {
        auto x = event.getEventRelativeTo(filterPoints[i]).getPosition().getX();
        auto y = event.getEventRelativeTo(filterPoints[i]).getPosition().getY();
        if (filterPoints[i]->contains(juce::Point<int>(x, y)))
        {
            if (drawPointInfo != true)
            {
                drawPointInfo = true;
                pointInfoIndex = i;
                repaint();
            }
            return;
        }
        else
        {
            if (drawPointInfo != false)
            {
                drawPointInfo = false;
                repaint();
            }
        }
        x = event.getEventRelativeTo(filterBands[i]).getPosition().getX();
        y = event.getEventRelativeTo(filterBands[i]).getPosition().getY();
        if (filterBands[i]->contains(juce::Point<int>(x, y)))
        {
            if (drawPointInfo != true)
            {
                drawPointInfo = true;
                pointInfoIndex = i;
                repaint();
            }
            return;
        }
        else
        {
            if (drawPointInfo != false)
            {
                drawPointInfo = false;
                repaint();
            }
        }
    }
}

void FilterEditor::mouseExit(const juce::MouseEvent& event)
{
    drawPointInfo = false;
    repaint();
}
void FilterEditor::mouseUp(const juce::MouseEvent& event)
{
    pointDragged = false;
}

void FilterEditor::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
    for (auto i = 0; i < filterPoints.size(); i++)
    {
        auto x = event.getEventRelativeTo(filterPoints[i]).getPosition().getX();
        auto y = event.getEventRelativeTo(filterPoints[i]).getPosition().getY();
        if (filterPoints[i]->contains(juce::Point<int>(x, y)))
        {
            auto* slider = &filterBands[i]->qSlider;
            if (wheel.deltaY > 0)
                slider->setValue(slider->getValue() * 1.2);
            else if (wheel.deltaY < 0)
                slider->setValue(slider->getValue() * 0.8);
        }
    }
}

void FilterEditor::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    for (auto i = 0; i < filterPoints.size(); i++)
    {
        if (source == &filterPoints[i]->mouseCtrlDraggedBroadcaster)
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
            if (filterBands[i]->getFilterType() == FilterProcessor::FilterTypes::HighShelf
                || filterBands[i]->getFilterType() == FilterProcessor::FilterTypes::LowShelf)
            {
                filterBands[i]->qSlider.setRange(0.1f, 1.0f);
                filterBands[i]->qSlider.setNumDecimalPlacesToDisplay(1);
            }
            else
            {
                filterBands[i]->qSlider.setRange(0.1f, 10.0f);
                filterBands[i]->qSlider.setNumDecimalPlacesToDisplay(1);
            }
        }
    }
}

void FilterEditor::updateBypassed()
{
    isFilterBypassed = editedFilterProcessor->isBypassed();
    for (auto i = 0; i < filterBands.size(); i++)
    {
        filterBands[i]->enableControl(!isFilterBypassed);
        setBypassedSliderColours(isFilterBypassed);
        if (isFilterBypassed)
            filterPoints[i]->setColour(4);
        else
            filterPoints[i]->setColour(i);
    }
    repaint();
}

void FilterEditor::sendParameters(int i)
{
    FilterProcessor::FilterParameters params;
    params.frequency = filterBands[i]->frequencySlider.getValue();
    params.gain = juce::Decibels::decibelsToGain(filterBands[i]->gainSlider.getValue());
    params.Q = filterBands[i]->qSlider.getValue();
    params.type = filterBands[i]->getFilterType();
    editedFilterProcessor->setFilterParameters(i, params);
    filterEditedBroadcaster->sendChangeMessage();
    magnitudeChanged = true;
    parametersChanged = true;
    
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
    zeroDbY = (float)getHeight() / 2.0f;
    float gain = ((zeroDbY - yPosition) * 12) / zeroDbY;
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
    if (parametersChanged)
        updateFilterGraphPoints();
}

void FilterEditor::setColour(juce::Colour colour)
{
    channelColour = colour;
    repaint();
}

int FilterEditor::getFilterGraphXStart()
{
    return frequencyPlotXStart;
}

void FilterEditor::setBypassedSliderColours(bool isBypassed)
{
    if (!isBypassed)
    {
        filterBands[0]->setSlidersThumbColours(juce::Colours::red);
        filterBands[1]->setSlidersThumbColours(juce::Colours::green);
        filterBands[2]->setSlidersThumbColours(juce::Colours::blue);
        filterBands[3]->setSlidersThumbColours(juce::Colours::yellow);
    }
    else
    {
        for (auto* band : filterBands)
        {
            band->setSlidersThumbColours(juce::Colours::grey);
        }
    }
}

void FilterEditor::setNullProcessor()
{
    FilterProcessor::GlobalParameters defaultParams = FilterProcessor::makeDefaultFilter();
    editedFilterProcessor = &dummyFilterProcessor;
    dummyFilterProcessor.setFilterParameters(0, defaultParams.lowBand);
    dummyFilterProcessor.setFilterParameters(1, defaultParams.lowMidBand);
    dummyFilterProcessor.setFilterParameters(2, defaultParams.highMidBand);
    dummyFilterProcessor.setFilterParameters(3, defaultParams.highBand);
    createMagnitudeArray();
    updateFilterGraphPoints();
    for (auto i = 0; i < filterBands.size(); i++)
    {
        filterBands[i]->enableControl(false);
        setBypassedSliderColours(true);
        filterPoints[i]->setColour(4);
    }
    repaint();
    editedFilterProcessor = nullptr;
}

FilterProcessor* FilterEditor::getEditedFilterProcessor()
{
    if (editedFilterProcessor != nullptr)
        return editedFilterProcessor;
}