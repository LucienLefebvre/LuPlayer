/*
  ==============================================================================

    CompEditor.h
    Created: 19 Apr 2021 11:23:34pm
    Author:  DPR

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CompProcessor.h"
//==============================================================================
/*
*/
class CompEditor : public juce::Component,
    public juce::Slider::Listener,
    public juce::Timer
{
public:
    CompEditor()
    {
        juce::Timer::startTimer(50);

        addAndMakeVisible(&thresholdSlider);
        thresholdSlider.setRange(-40.0f, 0.0f);
        thresholdSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        thresholdSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, knobWidth, 20);
        thresholdSlider.setNumDecimalPlacesToDisplay(1);
        thresholdSlider.setTextValueSuffix("dB");
        thresholdSlider.setDoubleClickReturnValue(true, 0.);
        thresholdSlider.setColour(juce::Slider::ColourIds::textBoxOutlineColourId,
            getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        thresholdSlider.setComponentID("THRESHOLD");
        thresholdSlider.addListener(this);

        addAndMakeVisible(&ratioSlider);
        ratioSlider.setRange(1.0f, 10.0f);
        ratioSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        ratioSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, knobWidth, 20);
        ratioSlider.setNumDecimalPlacesToDisplay(1);
        ratioSlider.setTextValueSuffix("");
        ratioSlider.setDoubleClickReturnValue(true, 1.);
        ratioSlider.setColour(juce::Slider::ColourIds::textBoxOutlineColourId,
            getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        ratioSlider.setComponentID("RATIO");
        ratioSlider.addListener(this);


        addAndMakeVisible(&attackSlider);
        attackSlider.setRange(1.0f, 500.0f);
        attackSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        attackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, knobWidth, 20);
        attackSlider.setNumDecimalPlacesToDisplay(1);
        attackSlider.setTextValueSuffix("ms");
        attackSlider.setDoubleClickReturnValue(true, 10.);
        attackSlider.setSkewFactor(0.3f);
        attackSlider.setColour(juce::Slider::ColourIds::textBoxOutlineColourId,
            getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        attackSlider.setComponentID("ATTACK");
        attackSlider.addListener(this);


        addAndMakeVisible(&releaseSlider);
        releaseSlider.setRange(1.0f, 1000.0f);
        releaseSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        releaseSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, knobWidth, 20);
        releaseSlider.setNumDecimalPlacesToDisplay(1);
        releaseSlider.setSkewFactor(0.3f);
        releaseSlider.setTextValueSuffix("ms");
        releaseSlider.setDoubleClickReturnValue(true, 100.);
        releaseSlider.setColour(juce::Slider::ColourIds::textBoxOutlineColourId,
            getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        releaseSlider.setComponentID("RELEASE");
        releaseSlider.addListener(this);


        addAndMakeVisible(&gainSlider);
        gainSlider.setRange(-12.0f, 12.0f);
        gainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, knobWidth, 20);
        gainSlider.setNumDecimalPlacesToDisplay(1);
        gainSlider.setTextValueSuffix("dB");
        gainSlider.setDoubleClickReturnValue(true, 0.);
        gainSlider.setColour(juce::Slider::ColourIds::textBoxOutlineColourId,
            getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        gainSlider.setComponentID("GAIN");
        gainSlider.addListener(this);

        addAndMakeVisible(&cButton);
        cButton.setButtonText("C");

        addAndMakeVisible(&dButton);
        dButton.setButtonText("D");

        addAndMakeVisible(&gButton);
        gButton.setButtonText("G");

        addAndMakeVisible(&lButton);
        lButton.setButtonText("L");
    }

    ~CompEditor() override
    {
    }


    void paint (juce::Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background


        if (editedCompProcessor != nullptr)
            reductionDB = editedCompProcessor->getReductionDB();
        int reductionRectHeight = reductionDB * getHeight() / 24;
        if (reductionRectHeight < 0)
            reductionRectHeight = 0;
        g.setColour(juce::Colour(40, 134, 189));
        g.fillRoundedRectangle(knobWidth * 2 + 5, topButtonsHeight, 20, reductionRectHeight, 4);

    }

    void timerCallback()
    {
        repaint();
    }
    void resized() override
    {
        gButton.setBounds(5, 0, topButtonsWIdth, topButtonsHeight);
        cButton.setBounds(55, 0, topButtonsWIdth, topButtonsHeight);
        dButton.setBounds(105, 0, topButtonsWIdth, topButtonsHeight);
        lButton.setBounds(155, 0, topButtonsWIdth, topButtonsHeight);
        thresholdSlider.setBounds(0, topButtonsHeight, knobWidth, knobWidth);
        ratioSlider.setBounds(knobWidth, topButtonsHeight, knobWidth, knobWidth);
        attackSlider.setBounds(0, knobWidth + topButtonsHeight, knobWidth, knobWidth);
        releaseSlider.setBounds(knobWidth, knobWidth + topButtonsHeight, knobWidth, knobWidth);
        gainSlider.setBounds(knobWidth / 2, 2 * knobWidth + topButtonsHeight, knobWidth, knobWidth);
    }

    void setEditedCompProcessor(CompProcessor& processor) 
    {
        editedCompProcessor = &processor;
        auto compParams = editedCompProcessor->getCompParams();
        thresholdSlider.setValue(compParams.threshold);
        gainSlider.setValue(compParams.gain);
        attackSlider.setValue(compParams.attack);
        releaseSlider.setValue(compParams.release);
        ratioSlider.setValue(compParams.ratio);
    }


private:
    void sliderValueChanged(juce::Slider* slider)
    {
        if (slider == &thresholdSlider)
            editedCompProcessor->setThreshold(thresholdSlider.getValue());
        if (slider == &gainSlider)
            editedCompProcessor->setGain(gainSlider.getValue());
        if (slider == &ratioSlider)
            editedCompProcessor->setRatio(ratioSlider.getValue());
        if (slider == &attackSlider)
            editedCompProcessor->setAttack(attackSlider.getValue());
        if (slider == &releaseSlider)
            editedCompProcessor->setRelease(releaseSlider.getValue());
    }
    CompProcessor* editedCompProcessor = 0;
    CompProcessor::CompParameters compParams;

    int topButtonsHeight = 20;
    int topButtonsWIdth = 40;
    juce::TextButton cButton;
    juce::TextButton gButton;
    juce::TextButton dButton;
    juce::TextButton lButton;

    juce::Slider thresholdSlider;
    juce::Slider ratioSlider;
    juce::Slider attackSlider;
    juce::Slider releaseSlider;
    juce::Slider gainSlider;
    int knobWidth = 80;
    float reductionDB = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CompEditor)
};
