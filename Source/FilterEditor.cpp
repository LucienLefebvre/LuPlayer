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
    //frequency sliders
    addFilterBand(&lowBandSliders);
    addFilterBand(&middleLowBandSliders);
    addFilterBand(&middleHighBandSliders);
    addFilterBand(&highBandSliders);
   
}

FilterEditor::~FilterEditor()
{

}

void FilterEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
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

}

void FilterEditor::addFilterBand(juce::OwnedArray<juce::Slider>* band)
{
    //Q
    band->add(new juce::Slider);
    addAndMakeVisible(band->getLast());
    band->getLast()->setRange(0.0001f, 5.0f);
    band->getLast()->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    band->getLast()->setTextBoxStyle(juce::Slider::TextBoxBelow, false, knobWidth, 20);
    band->getLast()->setNumDecimalPlacesToDisplay(1);
    band->getLast()->addListener(this);
    //GAIN
    band->add(new juce::Slider);
    addAndMakeVisible(band->getLast());
    band->getLast()->setRange(-12.0f, +12.0f);
    band->getLast()->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    band->getLast()->setTextBoxStyle(juce::Slider::TextBoxBelow, false, knobWidth, 20);
    band->getLast()->setNumDecimalPlacesToDisplay(1);
    band->getLast()->setTextValueSuffix("dB");
    band->getLast()->setDoubleClickReturnValue(true, 0.);
    band->getLast()->addListener(this);
    //FREQUENCY
    band->add(new juce::Slider);
    addAndMakeVisible(band->getLast());
    band->getLast()->setRange(20.0f, 20000.0f);
    band->getLast()->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    band->getLast()->setTextBoxStyle(juce::Slider::TextBoxBelow, false, knobWidth, 20);
    band->getLast()->setSkewFactor(0.3f);
    band->getLast()->setNumDecimalPlacesToDisplay(0);
    band->getLast()->setTextValueSuffix("Hz");
    band->getLast()->addListener(this);
}

void FilterEditor::setEditedFilterProcessor(FilterProcessor& processor)
{
    editedFilterProcessor = &processor;
    DBG(processor.sayHelloWorld());

    lowBandSliders[0]->setValue(editedFilterProcessor->getFilterParameters(0)[2], juce::NotificationType::dontSendNotification);
    lowBandSliders[1]->setValue(juce::Decibels::gainToDecibels(editedFilterProcessor->getFilterParameters(0)[1]), juce::NotificationType::dontSendNotification);
    lowBandSliders[2]->setValue(editedFilterProcessor->getFilterParameters(0)[0], juce::NotificationType::dontSendNotification);

    middleLowBandSliders[0]->setValue(editedFilterProcessor->getFilterParameters(1)[2], juce::NotificationType::dontSendNotification);
    middleLowBandSliders[1]->setValue(juce::Decibels::gainToDecibels(editedFilterProcessor->getFilterParameters(1)[1]), juce::NotificationType::dontSendNotification);
    middleLowBandSliders[2]->setValue(editedFilterProcessor->getFilterParameters(1)[0], juce::NotificationType::dontSendNotification);

    middleHighBandSliders[0]->setValue(editedFilterProcessor->getFilterParameters(2)[2], juce::NotificationType::dontSendNotification);
    middleHighBandSliders[1]->setValue(juce::Decibels::gainToDecibels(editedFilterProcessor->getFilterParameters(2)[1]), juce::NotificationType::dontSendNotification);
    middleHighBandSliders[2]->setValue(editedFilterProcessor->getFilterParameters(2)[0], juce::NotificationType::dontSendNotification);

    highBandSliders[0]->setValue(editedFilterProcessor->getFilterParameters(3)[2], juce::NotificationType::dontSendNotification);
    highBandSliders[1]->setValue(juce::Decibels::gainToDecibels(editedFilterProcessor->getFilterParameters(3)[1]), juce::NotificationType::dontSendNotification);
    highBandSliders[2]->setValue(editedFilterProcessor->getFilterParameters(3)[0], juce::NotificationType::dontSendNotification);



}

void FilterEditor::sliderValueChanged(juce::Slider* slider)
{
    if (editedFilterProcessor != nullptr)
    {
        for (auto i = 0; i < 3; i++)
        {
            if (slider == lowBandSliders[i])
            {
                float filterParams[3] = { lowBandSliders[2]->getValue(), lowBandSliders[0]->getValue(), 
                                        juce::Decibels::decibelsToGain(lowBandSliders[1]->getValue()) };
                editedFilterProcessor->setFilterParameters(0, filterParams);
            }
            else if (slider == middleLowBandSliders[i])
            {
                float filterParams[3] = { middleLowBandSliders[2]->getValue(), middleLowBandSliders[0]->getValue(), 
                                        juce::Decibels::decibelsToGain(middleLowBandSliders[1]->getValue()) };
                editedFilterProcessor->setFilterParameters(1, filterParams);
            }
            else if (slider == middleHighBandSliders[i])
            {
                float filterParams[3] = { middleHighBandSliders[2]->getValue(), middleHighBandSliders[0]->getValue(), 
                                        juce::Decibels::decibelsToGain(middleHighBandSliders[1]->getValue()) };
                editedFilterProcessor->setFilterParameters(2, filterParams);
            }
            else if (slider == highBandSliders[i])
            {
                float filterParams[3] = { highBandSliders[2]->getValue(), highBandSliders[0]->getValue(), 
                                        juce::Decibels::decibelsToGain(highBandSliders[1]->getValue()) };
                editedFilterProcessor->setFilterParameters(3, filterParams);
            }
        }
    }
}