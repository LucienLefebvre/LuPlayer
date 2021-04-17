/*
  ==============================================================================

    FilterEditor.cpp
    Created: 16 Apr 2021 1:50:43am
    Author:  DPR

  ==============================================================================
*/
#include <JuceHeader.h>
#include "FilterEditor.h"
FilterEditor::FilterEditor()
{
    //frequency sliders & filter points
    addFilterBand(&lowBandSliders);
    for (auto i = 0; i < 3; i++)
        lowBandSliders[i]->setColour(juce::Slider::ColourIds::thumbColourId, juce::Colours::red);
    filterPoints.add(new filterGraphPoint(0));
    addAndMakeVisible(filterPoints.getLast());
    filterPoints.getLast()->setSize(filterPoints.getLast()->getPointSize(), filterPoints.getLast()->getPointSize());
    filterPoints.getLast()->mouseDraggedBroadcaster.addChangeListener(this);

    addFilterBand(&middleLowBandSliders);
    for (auto i = 0; i < 3; i++)
        middleLowBandSliders[i]->setColour(juce::Slider::ColourIds::thumbColourId, juce::Colours::green);
    filterPoints.add(new filterGraphPoint(1));
    addAndMakeVisible(filterPoints.getLast());
    filterPoints.getLast()->setSize(filterPoints.getLast()->getPointSize(), filterPoints.getLast()->getPointSize());
    filterPoints.getLast()->mouseDraggedBroadcaster.addChangeListener(this);

    addFilterBand(&middleHighBandSliders);
    for (auto i = 0; i < 3; i++)
        middleHighBandSliders[i]->setColour(juce::Slider::ColourIds::thumbColourId, juce::Colours::blue);
    filterPoints.add(new filterGraphPoint(2));
    addAndMakeVisible(filterPoints.getLast());
    filterPoints.getLast()->setSize(filterPoints.getLast()->getPointSize(), filterPoints.getLast()->getPointSize());
    filterPoints.getLast()->mouseDraggedBroadcaster.addChangeListener(this);

    addFilterBand(&highBandSliders);
    for (auto i = 0; i < 3; i++)
        highBandSliders[i]->setColour(juce::Slider::ColourIds::thumbColourId, juce::Colours::yellow);
    filterPoints.add(new filterGraphPoint(3));
    addAndMakeVisible(filterPoints.getLast());
    filterPoints.getLast()->setSize(filterPoints.getLast()->getPointSize(), filterPoints.getLast()->getPointSize());
    filterPoints.getLast()->mouseDraggedBroadcaster.addChangeListener(this);

    DBG("gain de 6 en DB : " << juce::Decibels::gainToDecibels(3.0f));
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
    juce::Path frequencyPath;
    int zeroDbYPosition = frequencyPlotBounds.getHeight() / 2;
    int zeroHzXPosition = frequencyPlotBounds.getPosition().getX();

    int oldX = zeroHzXPosition;
    int oldY = zeroDbYPosition - magnitudeArray[0] * zeroDbYPosition / 12;
    for (auto i = 0; i < frequencyArray.size(); i++)
    {
        int x = zeroHzXPosition + i;
        int y = zeroDbYPosition - magnitudeArray[i] * zeroDbYPosition / 12;
        g.setColour(juce::Colour(40, 134, 189));
        g.drawLine(oldX, oldY, x, y, 2);
        oldX = x;
        oldY = y;
    }

}

void FilterEditor::resized()
{
    for (auto i = 0; i < 3; i++)
    {
        lowBandSliders[i]->setBounds(0, i * knobHeight, knobWidth, knobHeight);
        middleLowBandSliders[i]->setBounds(knobWidth, i * knobHeight, knobWidth, knobHeight);
        middleHighBandSliders[i]->setBounds(2 * knobWidth, i * knobHeight, knobWidth, knobHeight);
        highBandSliders[i]->setBounds(3 * knobWidth, i * knobHeight, knobWidth, knobHeight);
    }
    frequencyPlotBounds.setBounds(highBandSliders[0]->getRight(), 0, getWidth() - highBandSliders[0]->getRight(), getHeight());
    frequencyPlotXStart = frequencyPlotBounds.getX();
    zeroDbY = getHeight() / 2;
    createMagnitudeArray();
    updateFilterGraphPoints();
}

void FilterEditor::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    actualSampleRate = sampleRate;
    actualSamplesPerBlockExpected = samplesPerBlockExpected;
}

void FilterEditor::addFilterBand(juce::OwnedArray<juce::Slider>* band)
{
    //Q sliders
    band->add(new juce::Slider);
    addAndMakeVisible(band->getLast());
    band->getLast()->setRange(0.1f, 10.0f);
    band->getLast()->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    band->getLast()->setTextBoxStyle(juce::Slider::TextBoxBelow, false, knobWidth, 20);
    band->getLast()->setSkewFactor(0.3f);
    band->getLast()->setNumDecimalPlacesToDisplay(1);
    band->getLast()->setDoubleClickReturnValue(true, 1.);
    band->getLast()->addListener(this);
    band->getLast()->setColour(juce::Slider::ColourIds::textBoxOutlineColourId, 
        getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    //GAIN sliders
    band->add(new juce::Slider);
    addAndMakeVisible(band->getLast());
    band->getLast()->setRange(-12.0f, +12.0f);
    band->getLast()->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    band->getLast()->setTextBoxStyle(juce::Slider::TextBoxBelow, false, knobWidth, 20);
    band->getLast()->setNumDecimalPlacesToDisplay(1);
    band->getLast()->setTextValueSuffix("dB");
    band->getLast()->setDoubleClickReturnValue(true, 0.);
    band->getLast()->addListener(this);
    band->getLast()->setColour(juce::Slider::ColourIds::textBoxOutlineColourId,
        getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    //FREQUENCY sliders
    band->add(new juce::Slider);
    addAndMakeVisible(band->getLast());
    band->getLast()->setRange(20.0f, 20000.0f);
    band->getLast()->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    band->getLast()->setTextBoxStyle(juce::Slider::TextBoxBelow, false, knobWidth, 20);
    band->getLast()->setSkewFactor(0.3f);
    band->getLast()->setNumDecimalPlacesToDisplay(0);
    band->getLast()->setTextValueSuffix("Hz");
    band->getLast()->addListener(this);
    band->getLast()->setColour(juce::Slider::ColourIds::textBoxOutlineColourId,
        getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

}

void FilterEditor::setEditedFilterProcessor(FilterProcessor& processor)
{
    editedFilterProcessor = &processor;
    //Set sliders Value
    lowBandSliders[0]->setValue(editedFilterProcessor->getFilterParameters(0)[1], juce::NotificationType::dontSendNotification);
    lowBandSliders[1]->setValue(juce::Decibels::gainToDecibels(editedFilterProcessor->getFilterParameters(0)[2]), juce::NotificationType::dontSendNotification);
    lowBandSliders[2]->setValue(editedFilterProcessor->getFilterParameters(0)[0], juce::NotificationType::dontSendNotification);

    middleLowBandSliders[0]->setValue(editedFilterProcessor->getFilterParameters(1)[1], juce::NotificationType::dontSendNotification);
    middleLowBandSliders[1]->setValue(juce::Decibels::gainToDecibels(editedFilterProcessor->getFilterParameters(1)[2]), juce::NotificationType::dontSendNotification);
    middleLowBandSliders[2]->setValue(editedFilterProcessor->getFilterParameters(1)[0], juce::NotificationType::dontSendNotification);

    middleHighBandSliders[0]->setValue(editedFilterProcessor->getFilterParameters(2)[1], juce::NotificationType::dontSendNotification);
    middleHighBandSliders[1]->setValue(juce::Decibels::gainToDecibels(editedFilterProcessor->getFilterParameters(2)[2]), juce::NotificationType::dontSendNotification);
    middleHighBandSliders[2]->setValue(editedFilterProcessor->getFilterParameters(2)[0], juce::NotificationType::dontSendNotification);

    highBandSliders[0]->setValue(editedFilterProcessor->getFilterParameters(3)[1], juce::NotificationType::dontSendNotification);
    highBandSliders[1]->setValue(juce::Decibels::gainToDecibels(editedFilterProcessor->getFilterParameters(3)[2]), juce::NotificationType::dontSendNotification);
    highBandSliders[2]->setValue(editedFilterProcessor->getFilterParameters(3)[0], juce::NotificationType::dontSendNotification);

    createMagnitudeArray();
    updateFilterGraphPoints();
}

void FilterEditor::createMagnitudeArray()
{
    juce::Array< juce::dsp::IIR::Coefficients<float>> filterCoefficients;

    filterCoefficients.clear();
    for (auto i = 0; i < 4; i++)//create an array of filters coefficients
    {
        float filterFrequency = editedFilterProcessor->getFilterParameters(i)[0];
        float filterGain = editedFilterProcessor->getFilterParameters(i)[2];
        float filterQ = editedFilterProcessor->getFilterParameters(i)[1];
        filterCoefficients.set(i, *juce::dsp::IIR::Coefficients<float>::makePeakFilter(actualSampleRate,
            filterFrequency, filterQ, filterGain));
    }
    double arraySize = frequencyPlotBounds.getWidth();
    frequencyArray.clear();
    magnitudeArray.clear();
    for (auto i = 0; i < arraySize; i++)
    {
        frequencyArray.set(i, juce::mapToLog10<double>((double)1 * (i/arraySize), 20, 20000)); //Create log frequency array to calculate magnitude
        //calculate magnitude array
        magnitudeArray.set(i, juce::Decibels::gainToDecibels(filterCoefficients[0].getMagnitudeForFrequency(frequencyArray[i], actualSampleRate)
                                                            * filterCoefficients[1].getMagnitudeForFrequency(frequencyArray[i], actualSampleRate)
                                                            * filterCoefficients[2].getMagnitudeForFrequency(frequencyArray[i], actualSampleRate)
                                                            * filterCoefficients[3].getMagnitudeForFrequency(frequencyArray[i], actualSampleRate)));
    }
    magnitudeArrayCreated = true;
    repaint();
}

void FilterEditor::sliderValueChanged(juce::Slider* slider)
{
    if (editedFilterProcessor != nullptr)
    {
        for (auto i = 0; i < 3; i++)
        {
            if (slider == lowBandSliders[i])
            {
                std::array<float, 3> filterParams = { lowBandSliders[2]->getValue(), lowBandSliders[0]->getValue(),
                                        juce::Decibels::decibelsToGain(lowBandSliders[1]->getValue()) };
                editedFilterProcessor->setFilterParameters(0, filterParams);
            }
            else if (slider == middleLowBandSliders[i])
            {
                std::array<float, 3> filterParams = { middleLowBandSliders[2]->getValue(), middleLowBandSliders[0]->getValue(),
                                        juce::Decibels::decibelsToGain(middleLowBandSliders[1]->getValue()) };
                editedFilterProcessor->setFilterParameters(1, filterParams);
            }
            else if (slider == middleHighBandSliders[i])
            {
                std::array<float, 3> filterParams = { middleHighBandSliders[2]->getValue(), middleHighBandSliders[0]->getValue(),
                                        juce::Decibels::decibelsToGain(middleHighBandSliders[1]->getValue()) };
                editedFilterProcessor->setFilterParameters(2, filterParams);
            }
            else if (slider == highBandSliders[i])
            {
                std::array<float, 3> filterParams = { highBandSliders[2]->getValue(), highBandSliders[0]->getValue(),
                                        juce::Decibels::decibelsToGain(highBandSliders[1]->getValue()) };
                editedFilterProcessor->setFilterParameters(3, filterParams);
            }
        }
    }
    createMagnitudeArray();
    updateFilterGraphPoints();
}

void FilterEditor::updateFilterGraphPoints()
{
    int zeroDbYPosition = frequencyPlotBounds.getHeight() / 2;
    for (auto i = 0; i < filterPoints.size(); i++)
    {
        float pointXposition = juce::mapFromLog10<float>(editedFilterProcessor->getFilterParameters(i)[0], 20, 20000) * frequencyPlotBounds.getWidth();
        filterPoints[i]->setCentrePosition(frequencyPlotBounds.getTopLeft().getX() + pointXposition,
                        zeroDbYPosition - (juce::Decibels::gainToDecibels(editedFilterProcessor->getFilterParameters(i)[2]) * zeroDbYPosition / 12));
    }
}

void FilterEditor::mouseDrag(const juce::MouseEvent& event)
{

}

void FilterEditor::changeListenerCallback(juce::ChangeBroadcaster* source)
{


    if (source == &filterPoints[0]->mouseDraggedBroadcaster)
    {
        //set the value, (in arguments we get the centre of the point)
        lowBandSliders[2]->setValue(getFrequencyFromXPosition(filterPoints[0]->getX() + filterPoints[0]->getWidth() / 2));
        lowBandSliders[1]->setValue(getGainFromYPosition(filterPoints[0]->getY() + filterPoints[0]->getHeight() / 2));
    }
    else if (source == &filterPoints[1]->mouseDraggedBroadcaster)
    {
        //set the value, (in arguments we get the centre of the point)
        middleLowBandSliders[2]->setValue(getFrequencyFromXPosition(filterPoints[1]->getX() + filterPoints[1]->getWidth() / 2));
        middleLowBandSliders[1]->setValue(getGainFromYPosition(filterPoints[1]->getY() + filterPoints[1]->getHeight() / 2));
    }
    else if (source == &filterPoints[2]->mouseDraggedBroadcaster)
    {
        //set the value, (in arguments we get the centre of the point)
        middleHighBandSliders[2]->setValue(getFrequencyFromXPosition(filterPoints[2]->getX() + filterPoints[2]->getWidth() / 2));
        middleHighBandSliders[1]->setValue(getGainFromYPosition(filterPoints[2]->getY() + filterPoints[2]->getHeight() / 2));
    }
    else if (source == &filterPoints[3]->mouseDraggedBroadcaster)
    {
        //set the value, (in arguments we get the centre of the point)
        highBandSliders[2]->setValue(getFrequencyFromXPosition(filterPoints[3]->getX() + filterPoints[3]->getWidth() / 2));
        highBandSliders[1]->setValue(getGainFromYPosition(filterPoints[3]->getY() + filterPoints[3]->getHeight() / 2));
    }

}

float FilterEditor::getFrequencyFromXPosition(int xPosition)
{
    //used to get the frequency corresponding to the x position in the eq plot
    // it takes the relative position in the filterEditor component and map it
    float frequency = juce::mapToLog10<float>((float)(xPosition - frequencyPlotXStart) / frequencyPlotBounds.getWidth(), 20.0f, 20000.0f);
    return frequency;
}

float FilterEditor::getGainFromYPosition(int yPosition)
{
    //same as above but for the gain
    float gain = (float)((zeroDbY - yPosition) / 12.0f);
    return gain;
}