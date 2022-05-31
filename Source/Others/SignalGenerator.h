/*
  ==============================================================================

    SignalGenerator.h
    Created: 31 May 2022 2:34:45pm
    Author:  DPR

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class SignalGenerator : public juce::Component,
    public juce::Slider::Listener
{
public:
    SignalGenerator()
    {
        //CONTROL
        enableButton.reset(new juce::ToggleButton());
        addAndMakeVisible(enableButton.get());
        enableButton->setButtonText("Enable");

        leftButton.reset(new juce::ToggleButton());
        addAndMakeVisible(leftButton.get());
        leftButton->setButtonText("Left");
        leftButton->setToggleState(true, juce::dontSendNotification);

        rightButton.reset(new juce::ToggleButton());
        addAndMakeVisible(rightButton.get());
        rightButton->setButtonText("Right");
        rightButton->setToggleState(true, juce::dontSendNotification);

        //FREQUENCY
        frequencySlider.reset(new juce::Slider());
        addAndMakeVisible(frequencySlider.get());
        frequencySlider->setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
        frequencySlider->setRange(20.0, 20000.0, 1);
        frequencySlider->setTextValueSuffix("Hz");
        frequencySlider->setValue(1000);
        frequencySlider->setSkewFactor(0.25, false);
        frequencySlider->addListener(this);

        hundredHzButton.reset(new juce::TextButton());
        addAndMakeVisible(hundredHzButton.get());
        hundredHzButton->setButtonText("100 Hz");
        hundredHzButton->onClick = [this] {frequencySlider->setValue(100.0f); };

        oneKHzButton.reset(new juce::TextButton());
        addAndMakeVisible(oneKHzButton.get());
        oneKHzButton->setButtonText("1 kHz");
        oneKHzButton->onClick = [this] {frequencySlider->setValue(1000.0f); };

        tenKHzButton.reset(new juce::TextButton());
        addAndMakeVisible(tenKHzButton.get());
        tenKHzButton->setButtonText("10 kHz");
        tenKHzButton->onClick = [this] {frequencySlider->setValue(10000.0f); };

        //LEVEL
        levelSlider.reset(new juce::Slider());
        addAndMakeVisible(levelSlider.get());
        levelSlider->setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
        levelSlider->setRange(-60.0, 0.0, 1);
        levelSlider->setTextValueSuffix("dB");
        levelSlider->setValue(-18.0f);
        levelSlider->setSkewFactor(2, false);
        levelSlider->addListener(this);

        minusTwentyDBButton.reset(new juce::TextButton());
        addAndMakeVisible(minusTwentyDBButton.get());
        minusTwentyDBButton->setButtonText("-20dBFS");
        minusTwentyDBButton->onClick = [this] {levelSlider->setValue(-20.0f); };

        minusEighteenDBButton.reset(new juce::TextButton());
        addAndMakeVisible(minusEighteenDBButton.get());
        minusEighteenDBButton->setButtonText("-18dBFS");
        minusEighteenDBButton->onClick = [this] {levelSlider->setValue(-18.0f); };

        zeroLUButton.reset(new juce::TextButton());
        addAndMakeVisible(zeroLUButton.get());
        zeroLUButton->setButtonText("0LU");
        zeroLUButton->onClick = [this] {levelSlider->setValue(-23.0f);
                                        frequencySlider->setValue(1000.0f);
                                        rightButton->setToggleState(true, juce::dontSendNotification); 
                                        leftButton->setToggleState(true, juce::dontSendNotification); };
    }

    ~SignalGenerator() override
    {
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
        g.setColour(BLUE);
        g.drawRect(0, 0, getWidth(), getHeight());
    }

    void resized() override
    {
        enableButton->setBounds(0, 0, buttonWidth, buttonHeight);
        leftButton->setBounds(0, enableButton->getBottom() + spacer, buttonWidth, buttonHeight);
        rightButton->setBounds(leftButton->getRight(), enableButton->getBottom() + spacer, buttonWidth, buttonHeight);

        frequencySlider->setBounds(0, leftButton->getBottom() + 2 * spacer, 2 * buttonWidth, buttonHeight);
        hundredHzButton->setBounds(0, frequencySlider->getBottom() + spacer, 200 / 3 - 3, buttonHeight);
        oneKHzButton->setBounds(hundredHzButton->getRight() + 5, frequencySlider->getBottom() + spacer, 200 / 3 - 3, buttonHeight);
        tenKHzButton->setBounds(oneKHzButton->getRight() + 5, frequencySlider->getBottom() + spacer, 200 / 3 - 3, buttonHeight);

        levelSlider->setBounds(0, tenKHzButton->getBottom() + 2 * spacer, 2 * buttonWidth, buttonHeight);
        minusTwentyDBButton->setBounds(0, levelSlider->getBottom() + spacer, 200 / 3 - 3, buttonHeight);
        minusEighteenDBButton->setBounds(minusTwentyDBButton->getRight() + 5, levelSlider->getBottom() + spacer, 200 / 3 - 3, buttonHeight);
        zeroLUButton->setBounds(minusEighteenDBButton->getRight() + 5, levelSlider->getBottom() + spacer, 200 / 3 - 3, buttonHeight);
    }

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate)
    {
        actualSamplesPerBlockExpected = samplesPerBlockExpected;
        actualSampleRate = sampleRate;

        updateAngleDelta();
    }

    void updateAngleDelta()
    {
        auto cyclesPerSample = frequencySlider->getValue() / actualSampleRate;
        angleDelta = cyclesPerSample * 2.0 * juce::MathConstants<double>::pi;
    }

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
    {
        bufferToFill.clearActiveBufferRegion();

        auto* leftBuffer = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
        auto* rightBuffer = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);

        for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
        {
            auto currentSample = (float)std::sin(currentAngle);
            currentAngle += angleDelta;
            if (leftButton->getToggleState())
                leftBuffer[sample] = currentSample * level;
            if (rightButton->getToggleState())
                rightBuffer[sample] = currentSample * level;
        }
    }
    bool isEnabled()
    {
        return enableButton->getToggleState();
    }

    void setEnabled(bool b)
    {
        enableButton->setToggleState(b, juce::dontSendNotification);
    }

    void sliderValueChanged(juce::Slider* slider)
    {
        if (slider == frequencySlider.get())
            updateAngleDelta();
        else if (slider == levelSlider.get())
            level = juce::Decibels::decibelsToGain(levelSlider->getValue());
    }

    void visibilityChanged() override
    {
        if (!isVisible())
            enableButton->setToggleState(false, juce::dontSendNotification);
    }

private:
    std::unique_ptr<juce::ToggleButton> enableButton;
    std::unique_ptr<juce::ToggleButton> rightButton;
    std::unique_ptr<juce::ToggleButton> leftButton;

    std::unique_ptr<juce::Slider> frequencySlider;
    std::unique_ptr<juce::TextButton> hundredHzButton;
    std::unique_ptr<juce::TextButton> oneKHzButton;
    std::unique_ptr<juce::TextButton> tenKHzButton;

    std::unique_ptr<juce::Slider> levelSlider;
    std::unique_ptr<juce::TextButton> minusTwentyDBButton;
    std::unique_ptr<juce::TextButton> minusEighteenDBButton;
    std::unique_ptr<juce::TextButton> zeroLUButton;

    int buttonHeight = 25;
    int buttonWidth = 100;
    int spacer = 5;

    double frequency = 1000;
    double currentAngle = 0.0f, angleDelta = 0.0f;
    double level = 1.0f;

    int actualSamplesPerBlockExpected;
    double actualSampleRate;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SignalGenerator)
};
