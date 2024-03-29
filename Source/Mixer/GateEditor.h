/*
  ==============================================================================

    GateEditor.h
    Created: 27 Apr 2021 3:35:49pm
    Author:  Lucien Lefebvre

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CompProcessor.h"
//==============================================================================
class GateEditor  : public juce::Component, juce::Slider::Listener
{
public:
    GateEditor()
    {
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

        thresholdValueLabel.reset(new juce::Label("thresholdValueLabel"));
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

    }

    ~GateEditor() override
    {
        thresholdSlider = nullptr;
        ratioSlider = nullptr;
        attackSlider = nullptr;
        thresholdValueLabel = nullptr;
        thresholdLabel = nullptr;
        ratioValueLabel = nullptr;
        ratioLabel = nullptr;
        releaseSlider = nullptr;
        releaseValueLabel = nullptr;
        releaseLabel = nullptr;
        attackValueLabel = nullptr;
        attackLabel = nullptr;
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    }

    void resized() override
    {

    }

    void setEditedCompProcessor(CompProcessor& processor)
    {
        editedCompProcessor = &processor;
        auto gateParams = editedCompProcessor->getGateParams();
        thresholdSlider->setValue(gateParams.threshold);
        attackSlider->setValue(gateParams.attack);
        releaseSlider->setValue(gateParams.release);
        ratioSlider->setValue(gateParams.ratio);
    }


private:
    void sliderValueChanged(juce::Slider* sliderThatWasMoved)
    {
        if (sliderThatWasMoved == thresholdSlider.get())
        {
            auto value = sliderThatWasMoved->getValue();
            thresholdValueLabel->setText(juce::String(value) + "dB", juce::NotificationType::dontSendNotification);
            editedCompProcessor->setGateThreshold(value);
        }
        else if (sliderThatWasMoved == ratioSlider.get())
        {
            auto value = sliderThatWasMoved->getValue();
            auto roundedValue = std::ceil(value * 10.0) / 10.0;
            ratioValueLabel->setText(juce::String(roundedValue), juce::NotificationType::dontSendNotification);
            editedCompProcessor->setGateRatio(value);
        }
        else if (sliderThatWasMoved == attackSlider.get())
        {
            auto value = sliderThatWasMoved->getValue();
            attackValueLabel->setText(juce::String(trunc(value)) + "ms", juce::NotificationType::dontSendNotification);
            editedCompProcessor->setGateAttack(value);
        }
        else if (sliderThatWasMoved == releaseSlider.get())
        {
            auto value = sliderThatWasMoved->getValue();
            releaseValueLabel->setText(juce::String(trunc(value)) + "ms", juce::NotificationType::dontSendNotification);
            editedCompProcessor->setGateRelease(value);
        }
    }
    CompProcessor* editedCompProcessor = 0;

    std::unique_ptr<juce::Slider> thresholdSlider;
    std::unique_ptr<juce::Slider> ratioSlider;
    std::unique_ptr<juce::Slider> attackSlider;
    std::unique_ptr<juce::Label> thresholdValueLabel;
    std::unique_ptr<juce::Label> thresholdLabel;
    std::unique_ptr<juce::Label> ratioValueLabel;
    std::unique_ptr<juce::Label> ratioLabel;
    std::unique_ptr<juce::Slider> releaseSlider;
    std::unique_ptr<juce::Label> attackValueLabel;
    std::unique_ptr<juce::Label> attackLabel;
    std::unique_ptr<juce::Label> releaseValueLabel;
    std::unique_ptr<juce::Label> releaseLabel;

    int knobWidth = 60;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GateEditor)
};
