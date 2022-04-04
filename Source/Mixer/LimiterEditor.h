/*
  ==============================================================================

    LimiterEditor.h
    Created: 27 Apr 2021 3:36:10pm
    Author:  Lucien Lefebvre

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

        thresholdSlider->setBounds(0, 96, 60, 60);

        thresholdValueLabel.reset(new juce::Label("thresholdValueLabel",
            TRANS("0")));
        addAndMakeVisible(thresholdValueLabel.get());
        thresholdValueLabel->setFont(juce::Font(15.00f, juce::Font::plain).withTypefaceStyle("Regular"));
        thresholdValueLabel->setJustificationType(juce::Justification::centred);
        thresholdValueLabel->setEditable(false, false, false);
        thresholdValueLabel->setColour(juce::TextEditor::textColourId, juce::Colours::black);
        thresholdValueLabel->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x00000000));

        thresholdValueLabel->setBounds(0, 144, 60, 24);

        thresholdLabel.reset(new juce::Label("thresholdLabel",
            TRANS("Threshold")));
        addAndMakeVisible(thresholdLabel.get());
        thresholdLabel->setFont(juce::Font(15.00f, juce::Font::plain).withTypefaceStyle("Regular"));
        thresholdLabel->setJustificationType(juce::Justification::centred);
        thresholdLabel->setEditable(false, false, false);
        thresholdLabel->setColour(juce::TextEditor::textColourId, juce::Colours::black);
        thresholdLabel->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x00000000));

        thresholdLabel->setBounds(0, 160, 60, 24);

        releaseSlider.reset(new juce::Slider("releaseSlider"));
        addAndMakeVisible(releaseSlider.get());
        releaseSlider->setRange(10.0f, 2000.0f, 0);
        releaseSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        releaseSlider->setTextBoxStyle(juce::Slider::NoTextBox, false, 80, 20);
        releaseSlider->addListener(this);
        releaseSlider->setSkewFactor(0.3f);
        releaseSlider->setBounds(72, 96, 60, 60);

        releaseValueLabel.reset(new juce::Label("releaseValueLabel",
            TRANS("0")));
        addAndMakeVisible(releaseValueLabel.get());
        releaseValueLabel->setFont(juce::Font(15.00f, juce::Font::plain).withTypefaceStyle("Regular"));
        releaseValueLabel->setJustificationType(juce::Justification::centred);
        releaseValueLabel->setEditable(false, false, false);
        releaseValueLabel->setColour(juce::TextEditor::textColourId, juce::Colours::black);
        releaseValueLabel->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x00000000));

        releaseValueLabel->setBounds(73, 143, 60, 24);

        releaseLabel.reset(new juce::Label("releaseLabel",
            TRANS("Release\n")));
        addAndMakeVisible(releaseLabel.get());
        releaseLabel->setFont(juce::Font(15.00f, juce::Font::plain).withTypefaceStyle("Regular"));
        releaseLabel->setJustificationType(juce::Justification::centred);
        releaseLabel->setEditable(false, false, false);
        releaseLabel->setColour(juce::TextEditor::textColourId, juce::Colours::black);
        releaseLabel->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x00000000));

        releaseLabel->setBounds(73, 159, 60, 24);

        gainSlider.reset(new juce::Slider("gainSlider"));
        addAndMakeVisible(gainSlider.get());
        gainSlider->setRange(0.0f, +24.0f, 0.1f);
        gainSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        gainSlider->setTextBoxStyle(juce::Slider::NoTextBox, false, 80, 20);
        gainSlider->addListener(this);

        gainSlider->setBounds(37, 4, 60, 60);

        gainValueLabel.reset(new juce::Label("gainValueLabel",
            TRANS("0")));
        addAndMakeVisible(gainValueLabel.get());
        gainValueLabel->setFont(juce::Font(15.00f, juce::Font::plain).withTypefaceStyle("Regular"));
        gainValueLabel->setJustificationType(juce::Justification::centred);
        gainValueLabel->setEditable(false, false, false);
        gainValueLabel->setColour(juce::TextEditor::textColourId, juce::Colours::black);
        gainValueLabel->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x00000000));

        gainValueLabel->setBounds(37, 52, 60, 24);

        gainLabel.reset(new juce::Label("gainLabel",
            TRANS("Input Gain")));
        addAndMakeVisible(gainLabel.get());
        gainLabel->setFont(juce::Font(15.00f, juce::Font::plain).withTypefaceStyle("Regular"));
        gainLabel->setJustificationType(juce::Justification::centred);
        gainLabel->setEditable(false, false, false);
        gainLabel->setColour(juce::TextEditor::textColourId, juce::Colours::black);
        gainLabel->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x00000000));

        gainLabel->setBounds(37, 68, 60, 24);
    }

    ~LimiterEditor() override
    {
        thresholdSlider = nullptr;
        releaseSlider = nullptr;
        thresholdValueLabel = nullptr;
        thresholdLabel = nullptr;
        releaseValueLabel = nullptr;
        releaseLabel = nullptr;
        gainSlider = nullptr;
        gainValueLabel = nullptr;
        gainLabel = nullptr;

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
        else if (sliderThatWasMoved == gainSlider.get())
        {
            auto value = sliderThatWasMoved->getValue();
            gainValueLabel->setText(juce::String(value) + "dB", juce::NotificationType::dontSendNotification);
            editedCompProcessor->setLimitGain(value);
        }
    }
    void setEditedCompProcessor(CompProcessor& processor)
    {
        editedCompProcessor = &processor;
        auto limitParams = editedCompProcessor->getLimitParams();
        thresholdSlider->setValue(limitParams.threshold);
        releaseSlider->setValue(limitParams.release);
        gainSlider->setValue(limitParams.gain);
    }

private:
    CompProcessor* editedCompProcessor = 0;

    std::unique_ptr<juce::Slider> thresholdSlider;
    std::unique_ptr<juce::Label> thresholdValueLabel;
    std::unique_ptr<juce::Label> thresholdLabel;
    std::unique_ptr<juce::Slider> releaseSlider;
    std::unique_ptr<juce::Label> releaseValueLabel;
    std::unique_ptr<juce::Label> releaseLabel;
    std::unique_ptr<juce::Slider> gainSlider;
    std::unique_ptr<juce::Label> gainValueLabel;
    std::unique_ptr<juce::Label> gainLabel;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LimiterEditor)
};
