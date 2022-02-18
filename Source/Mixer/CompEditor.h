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

        thresholdSlider.reset(new juce::Slider("thresholdSlider"));
        addAndMakeVisible(thresholdSlider.get());
        thresholdSlider->setRange(-60.0f, 0.0f, 0);
        thresholdSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        thresholdSlider->setTextBoxStyle(juce::Slider::NoTextBox, false, 80, 20);
        thresholdSlider->addListener(this);
        thresholdSlider->setDoubleClickReturnValue(true, 0.0f);

        thresholdSlider->setBounds(0, 0, 60, 60);

        ratioSlider.reset(new juce::Slider("ratioSlider"));
        addAndMakeVisible(ratioSlider.get());
        ratioSlider->setRange(1.0f, 10.0f, 0);
        ratioSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        ratioSlider->setTextBoxStyle(juce::Slider::NoTextBox, false, 80, 20);
        ratioSlider->addListener(this);
        ratioSlider->setDoubleClickReturnValue(true, 1.0f);

        ratioSlider->setBounds(76, 0, 60, 60);

        attackSlider.reset(new juce::Slider("attackSlider"));
        addAndMakeVisible(attackSlider.get());
        attackSlider->setRange(1.0f, 500.0, 0);
        attackSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        attackSlider->setTextBoxStyle(juce::Slider::NoTextBox, false, 80, 20);
        attackSlider->addListener(this);
        attackSlider->setSkewFactor(0.3f);
        attackSlider->setDoubleClickReturnValue(true, 20.0f);

        attackSlider->setBounds(0, 80, 60, 60);

        thresholdValueLabel.reset(new juce::Label("thresholdValueLabel", "0dB"));
        addAndMakeVisible(thresholdValueLabel.get());
        thresholdValueLabel->setFont(juce::Font(15.00f, juce::Font::plain).withTypefaceStyle("Regular"));
        thresholdValueLabel->setJustificationType(juce::Justification::centred);
        thresholdValueLabel->setEditable(false, false, false);
        thresholdValueLabel->setColour(juce::TextEditor::textColourId, juce::Colours::black);
        thresholdValueLabel->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x00000000));

        thresholdValueLabel->setBounds(0, 48, 60, 24);

        thresholdLabel.reset(new juce::Label("thresholdLabel",
            TRANS("Threshold")));
        addAndMakeVisible(thresholdLabel.get());
        thresholdLabel->setFont(juce::Font(15.00f, juce::Font::plain).withTypefaceStyle("Regular"));
        thresholdLabel->setJustificationType(juce::Justification::centred);
        thresholdLabel->setEditable(false, false, false);
        thresholdLabel->setColour(juce::TextEditor::textColourId, juce::Colours::black);
        thresholdLabel->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x00000000));

        thresholdLabel->setBounds(0, 64, 60, 24);

        ratioValueLabel.reset(new juce::Label("ratioValueLabel",
            TRANS("1.0\n")));
        addAndMakeVisible(ratioValueLabel.get());
        ratioValueLabel->setFont(juce::Font(15.00f, juce::Font::plain).withTypefaceStyle("Regular"));
        ratioValueLabel->setJustificationType(juce::Justification::centred);
        ratioValueLabel->setEditable(false, false, false);
        ratioValueLabel->setColour(juce::TextEditor::textColourId, juce::Colours::black);
        ratioValueLabel->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x00000000));

        ratioValueLabel->setBounds(74, 48, 64, 24);

        ratioLabel.reset(new juce::Label("ratioLabel",
            TRANS("Ratio\n")));
        addAndMakeVisible(ratioLabel.get());
        ratioLabel->setFont(juce::Font(15.00f, juce::Font::plain).withTypefaceStyle("Regular"));
        ratioLabel->setJustificationType(juce::Justification::centred);
        ratioLabel->setEditable(false, false, false);
        ratioLabel->setColour(juce::TextEditor::textColourId, juce::Colours::black);
        ratioLabel->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x00000000));

        ratioLabel->setBounds(74, 64, 64, 24);

        releaseSlider.reset(new juce::Slider("releaseSlider"));
        addAndMakeVisible(releaseSlider.get());
        releaseSlider->setRange(10.0f, 1000, 0);
        releaseSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        releaseSlider->setTextBoxStyle(juce::Slider::NoTextBox, false, 80, 20);
        releaseSlider->addListener(this);
        releaseSlider->setSkewFactor(0.3f);
        releaseSlider->setDoubleClickReturnValue(true, 200.);

        releaseSlider->setBounds(76, 80, 60, 60);

        attackValueLabel.reset(new juce::Label("attackValueLabel",
            TRANS("0")));
        addAndMakeVisible(attackValueLabel.get());
        attackValueLabel->setFont(juce::Font(15.00f, juce::Font::plain).withTypefaceStyle("Regular"));
        attackValueLabel->setJustificationType(juce::Justification::centred);
        attackValueLabel->setEditable(false, false, false);
        attackValueLabel->setColour(juce::TextEditor::textColourId, juce::Colours::black);
        attackValueLabel->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x00000000));

        attackValueLabel->setBounds(1, 127, 60, 24);

        attackLabel.reset(new juce::Label("attackLabel",
            TRANS("Attack\n")));
        addAndMakeVisible(attackLabel.get());
        attackLabel->setFont(juce::Font(15.00f, juce::Font::plain).withTypefaceStyle("Regular"));
        attackLabel->setJustificationType(juce::Justification::centred);
        attackLabel->setEditable(false, false, false);
        attackLabel->setColour(juce::TextEditor::textColourId, juce::Colours::black);
        attackLabel->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x00000000));

        attackLabel->setBounds(1, 143, 60, 24);

        releaseValueLabel.reset(new juce::Label("releaseValueLabel",
            TRANS("1.0\n")));
        addAndMakeVisible(releaseValueLabel.get());
        releaseValueLabel->setFont(juce::Font(15.00f, juce::Font::plain).withTypefaceStyle("Regular"));
        releaseValueLabel->setJustificationType(juce::Justification::centred);
        releaseValueLabel->setEditable(false, false, false);
        releaseValueLabel->setColour(juce::TextEditor::textColourId, juce::Colours::black);
        releaseValueLabel->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x00000000));

        releaseValueLabel->setBounds(74, 127, 64, 24);

        releaseLabel.reset(new juce::Label("releaseLabel",
            TRANS("Release\n")));
        addAndMakeVisible(releaseLabel.get());
        releaseLabel->setFont(juce::Font(15.00f, juce::Font::plain).withTypefaceStyle("Regular"));
        releaseLabel->setJustificationType(juce::Justification::centred);
        releaseLabel->setEditable(false, false, false);
        releaseLabel->setColour(juce::TextEditor::textColourId, juce::Colours::black);
        releaseLabel->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x00000000));

        releaseLabel->setBounds(74, 143, 64, 24);

        gainSlider.reset(new juce::Slider("gainSlider"));
        addAndMakeVisible(gainSlider.get());
        gainSlider->setRange(-24.0f, 24.0f, 0);
        gainSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        gainSlider->setTextBoxStyle(juce::Slider::NoTextBox, false, 80, 20);
        gainSlider->addListener(this);
        gainSlider->setDoubleClickReturnValue(true, 0.);

        gainSlider->setBounds(37, 150, 60, 60);

        gainValueLabel.reset(new juce::Label("gainValueLabel",
            TRANS("0dB\n")));
        addAndMakeVisible(gainValueLabel.get());
        gainValueLabel->setFont(juce::Font(15.00f, juce::Font::plain).withTypefaceStyle("Regular"));
        gainValueLabel->setJustificationType(juce::Justification::centred);
        gainValueLabel->setEditable(false, false, false);
        gainValueLabel->setColour(juce::TextEditor::textColourId, juce::Colours::black);
        gainValueLabel->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x00000000));

        gainValueLabel->setBounds(34, 197, 64, 24);

        gainLabel.reset(new juce::Label("gainLabel",
            TRANS("Gain\n")));
        addAndMakeVisible(gainLabel.get());
        gainLabel->setFont(juce::Font(15.00f, juce::Font::plain).withTypefaceStyle("Regular"));
        gainLabel->setJustificationType(juce::Justification::centred);
        gainLabel->setEditable(false, false, false);
        gainLabel->setColour(juce::TextEditor::textColourId, juce::Colours::black);
        gainLabel->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x00000000));

        gainLabel->setBounds(34, 213, 64, 24);
    }

    ~CompEditor() override
    {
        thresholdSlider = nullptr;
        ratioSlider = nullptr;
        releaseSlider = nullptr;
        thresholdValueLabel = nullptr;
        thresholdLabel = nullptr;
        ratioValueLabel = nullptr;
        ratioLabel = nullptr;
        attackSlider = nullptr;
        releaseValueLabel = nullptr;
        releaseLabel = nullptr;
        attackLabel = nullptr;
        attackValueLabel = nullptr;
        gainLabel = nullptr;
        gainSlider = nullptr;
        gainValueLabel = nullptr;
    }


    void paint (juce::Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
    }

    void timerCallback()
    {
        repaint();
    }
    void resized() override
    {
    }

    void updateBypassedSliders()
    {
        bool isBypassed = editedCompProcessor->getBypass();
        juce::Colour activeColour(juce::Colour(40, 134, 189));
        thresholdSlider->setEnabled(!isBypassed);
        thresholdSlider->setColour(juce::Slider::ColourIds::thumbColourId, isBypassed ? juce::Colours::grey : activeColour);
        attackSlider->setEnabled(!isBypassed);
        attackSlider->setColour(juce::Slider::ColourIds::thumbColourId, isBypassed ? juce::Colours::grey : activeColour);
        releaseSlider->setEnabled(!isBypassed);
        releaseSlider->setColour(juce::Slider::ColourIds::thumbColourId, isBypassed ? juce::Colours::grey : activeColour);
        gainSlider->setEnabled(!isBypassed);
        gainSlider->setColour(juce::Slider::ColourIds::thumbColourId, isBypassed ? juce::Colours::grey : activeColour);
        ratioSlider->setEnabled(!isBypassed);
        ratioSlider->setColour(juce::Slider::ColourIds::thumbColourId, isBypassed ? juce::Colours::grey : activeColour);
    }

    void setEditedCompProcessor(CompProcessor& processor) 
    {
        editedCompProcessor = &processor;
        auto compParams = editedCompProcessor->getCompParams();
        thresholdSlider->setValue(compParams.threshold);
        gainSlider->setValue(compParams.gain);
        attackSlider->setValue(compParams.attack);
        releaseSlider->setValue(compParams.release);
        ratioSlider->setValue(compParams.ratio);
    }


private:
    void sliderValueChanged(juce::Slider* sliderThatWasMoved)
    {
        if (sliderThatWasMoved == thresholdSlider.get())
        {
            auto value = sliderThatWasMoved->getValue();
            thresholdValueLabel->setText(juce::String(value) + "dB", juce::NotificationType::dontSendNotification);
            editedCompProcessor->setThreshold(value);
        }
        else if (sliderThatWasMoved == ratioSlider.get())
        {
            auto value = sliderThatWasMoved->getValue();
            auto roundedValue = std::ceil(value * 10.0) / 10.0;
            ratioValueLabel->setText(juce::String(roundedValue), juce::NotificationType::dontSendNotification);
            editedCompProcessor->setRatio(value);
        }
        else if (sliderThatWasMoved == attackSlider.get())
        {
            auto value = sliderThatWasMoved->getValue();
            attackValueLabel->setText(juce::String(trunc(value)) + "ms", juce::NotificationType::dontSendNotification);
            editedCompProcessor->setAttack(value);
        }
        else if (sliderThatWasMoved == releaseSlider.get())
        {
            auto value = sliderThatWasMoved->getValue();
            releaseValueLabel->setText(juce::String(trunc(value)) + "ms", juce::NotificationType::dontSendNotification);
            editedCompProcessor->setRelease(value);
        }
        else if (sliderThatWasMoved == gainSlider.get())
        {
            auto value = sliderThatWasMoved->getValue();
            auto roundedValue = std::ceil(value * 10.0) / 10.0;
            gainValueLabel->setText(juce::String(roundedValue) + "dB", juce::NotificationType::dontSendNotification);
            editedCompProcessor->setGain(value);
        }
    }
    CompProcessor* editedCompProcessor = 0;
    //CompProcessor::CompParameters compParams;

    std::unique_ptr<juce::Slider> thresholdSlider;
    std::unique_ptr<juce::Slider> ratioSlider;
    std::unique_ptr<juce::Slider> releaseSlider;
    std::unique_ptr<juce::Label> thresholdValueLabel;
    std::unique_ptr<juce::Label> thresholdLabel;
    std::unique_ptr<juce::Label> ratioValueLabel;
    std::unique_ptr<juce::Label> ratioLabel;
    std::unique_ptr<juce::Slider> attackSlider;
    std::unique_ptr<juce::Label> releaseValueLabel;
    std::unique_ptr<juce::Label> releaseLabel;
    std::unique_ptr<juce::Label> attackValueLabel;
    std::unique_ptr<juce::Label> attackLabel;
    std::unique_ptr<juce::Slider> gainSlider;
    std::unique_ptr<juce::Label> gainValueLabel;
    std::unique_ptr<juce::Label> gainLabel;

    int knobWidth = 60;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CompEditor)
};
