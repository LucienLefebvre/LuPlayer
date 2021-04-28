/*
  ==============================================================================

    LimiterEditor.h
    Created: 27 Apr 2021 3:36:10pm
    Author:  DPR

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CompProcessor.h"
//==============================================================================
/*
*/
class LimiterEditor  : public juce::Component,
    public juce::Slider::Listener
{
public:
    LimiterEditor()
    {
        thresholdSlider.reset(new juce::Slider("thresholdSlider"));
        addAndMakeVisible(thresholdSlider.get());
        thresholdSlider->setRange(-60, 0, 0);
        thresholdSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        thresholdSlider->setTextBoxStyle(juce::Slider::NoTextBox, false, 80, 20);
        thresholdSlider->addListener(this);

        thresholdSlider->setBounds(40, 0, 60, 60);

        thresholdValueLabel.reset(new juce::Label("thresholdValueLabel",
            TRANS("0")));
        addAndMakeVisible(thresholdValueLabel.get());
        thresholdValueLabel->setFont(juce::Font(15.00f, juce::Font::plain).withTypefaceStyle("Regular"));
        thresholdValueLabel->setJustificationType(juce::Justification::centred);
        thresholdValueLabel->setEditable(false, false, false);
        thresholdValueLabel->setColour(juce::TextEditor::textColourId, juce::Colours::black);
        thresholdValueLabel->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x00000000));

        thresholdValueLabel->setBounds(40, 48, 60, 24);

        thresholdLabel.reset(new juce::Label("thresholdLabel",
            TRANS("Threshold")));
        addAndMakeVisible(thresholdLabel.get());
        thresholdLabel->setFont(juce::Font(15.00f, juce::Font::plain).withTypefaceStyle("Regular"));
        thresholdLabel->setJustificationType(juce::Justification::centred);
        thresholdLabel->setEditable(false, false, false);
        thresholdLabel->setColour(juce::TextEditor::textColourId, juce::Colours::black);
        thresholdLabel->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x00000000));

        thresholdLabel->setBounds(40, 64, 60, 24);

        releaseSlider.reset(new juce::Slider("releaseSlider"));
        addAndMakeVisible(releaseSlider.get());
        releaseSlider->setRange(10.0f, 2000.0f, 0);
        releaseSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        releaseSlider->setTextBoxStyle(juce::Slider::NoTextBox, false, 80, 20);
        releaseSlider->addListener(this);
        releaseSlider->setSkewFactor(0.3f);
        releaseSlider->setBounds(40, 96, 60, 60);

        releaseValueLabel.reset(new juce::Label("releaseValueLabel",
            TRANS("0")));
        addAndMakeVisible(releaseValueLabel.get());
        releaseValueLabel->setFont(juce::Font(15.00f, juce::Font::plain).withTypefaceStyle("Regular"));
        releaseValueLabel->setJustificationType(juce::Justification::centred);
        releaseValueLabel->setEditable(false, false, false);
        releaseValueLabel->setColour(juce::TextEditor::textColourId, juce::Colours::black);
        releaseValueLabel->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x00000000));

        releaseValueLabel->setBounds(41, 143, 60, 24);

        releaseLabel.reset(new juce::Label("releaseLabel",
            TRANS("Release\n")));
        addAndMakeVisible(releaseLabel.get());
        releaseLabel->setFont(juce::Font(15.00f, juce::Font::plain).withTypefaceStyle("Regular"));
        releaseLabel->setJustificationType(juce::Justification::centred);
        releaseLabel->setEditable(false, false, false);
        releaseLabel->setColour(juce::TextEditor::textColourId, juce::Colours::black);
        releaseLabel->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x00000000));

        releaseLabel->setBounds(41, 159, 60, 24);

    }

    ~LimiterEditor() override
    {
        thresholdSlider = nullptr;
        releaseSlider = nullptr;
        thresholdValueLabel = nullptr;
        thresholdLabel = nullptr;
        releaseValueLabel = nullptr;
        releaseLabel = nullptr;
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
    }

    void resized() override
    {
    }

    void sliderValueChanged(juce::Slider* sliderThatWasMoved)
    {
        if (sliderThatWasMoved == thresholdSlider.get())
        {
            auto value = sliderThatWasMoved->getValue();
            thresholdValueLabel->setText(juce::String(value) + "dB", juce::NotificationType::dontSendNotification);
            editedCompProcessor->setLimitThreshold(value);
        }
        else if (sliderThatWasMoved == releaseSlider.get())
        {
            auto value = sliderThatWasMoved->getValue();
            releaseValueLabel->setText(juce::String(value) + "ms", juce::NotificationType::dontSendNotification);
            editedCompProcessor->setLimitRelease(value);
        }
    }
    void setEditedCompProcessor(CompProcessor& processor)
    {
        editedCompProcessor = &processor;
        auto limitParams = editedCompProcessor->getLimitParams();
        thresholdSlider->setValue(limitParams.threshold);
        releaseSlider->setValue(limitParams.release);
    }

private:
    CompProcessor* editedCompProcessor = 0;

    std::unique_ptr<juce::Slider> thresholdSlider;
    std::unique_ptr<juce::Label> thresholdValueLabel;
    std::unique_ptr<juce::Label> thresholdLabel;
    std::unique_ptr<juce::Slider> releaseSlider;
    std::unique_ptr<juce::Label> releaseValueLabel;
    std::unique_ptr<juce::Label> releaseLabel;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LimiterEditor)
};
