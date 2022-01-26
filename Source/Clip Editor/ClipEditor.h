/*
  ==============================================================================

    ClipEditor.h
    Created: 24 Jan 2022 2:47:44pm
    Author:  DPR

  ==============================================================================
*/

#pragma once
#define BLUE juce::Colour(40, 134, 189)
#define ORANGE juce::Colour(229, 149, 0)
#include <JuceHeader.h>
#include "EnveloppeEditor.h"
//==============================================================================
/*
*/
class ClipEditor  : public juce::Component,
    public juce::ChangeListener,
    public juce::Slider::Listener,
    public juce::Timer
{
public:
    ClipEditor()
    {
        juce::Timer::startTimerHz(30);

        addAndMakeVisible(&enveloppeEditor);

        //Name Label
        nameLabel.reset(new juce::Label);
        addAndMakeVisible(nameLabel.get());
        nameLabel->setJustificationType(juce::Justification::centred);
        nameLabel->setFont(juce::Font(25.0f, juce::Font::bold).withTypefaceStyle("Regular"));
        nameLabel->setColour(juce::Label::ColourIds::textColourId, juce::Colour(229, 149, 0));
        //Trim Volume Slider
        trimVolumeSlider.reset(new juce::Slider);
        addAndMakeVisible(trimVolumeSlider.get());
        trimVolumeSlider->setRange(-24, 24, 0.5);
        trimVolumeSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        trimVolumeSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 15);
        trimVolumeSlider->setColour(juce::Slider::ColourIds::textBoxOutlineColourId,
            getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        trimVolumeSlider->addListener(this);
        trimVolumeSlider->setDoubleClickReturnValue(true, 0.);
        trimVolumeSlider->setPopupDisplayEnabled(true, true, this, 2000);
        trimVolumeSlider->setScrollWheelEnabled(true);
        trimVolumeSlider->setWantsKeyboardFocus(false);
        trimVolumeSlider->setTextValueSuffix("dB");
        //Cue Button
        cueButton.reset(new juce::TextButton);
        addAndMakeVisible(cueButton.get());
        cueButton->setButtonText("Cue");
        cueButton->onClick = [this] {cueButtonClicked(); };
        //Cue Time Label
        cueTimeLabel.reset(new juce::Label);
        addAndMakeVisible(cueTimeLabel.get());
        cueTimeLabel->setJustificationType(juce::Justification::centred);
        cueTimeLabel->setFont(juce::Font(35.0f, juce::Font::bold).withTypefaceStyle("Regular"));
        //In Button
        inButton.reset(new juce::TextButton());
        addAndMakeVisible(inButton.get());
        inButton->setButtonText("IN");
        inButton->onClick = [this] {inButtonClicked(); };
        //In Label
        inLabel.reset(new juce::Label());
        addAndMakeVisible(inLabel.get());
        inLabel->setFont(juce::Font(25.0f, juce::Font::bold).withTypefaceStyle("Regular"));
        //Out Button
        outButton.reset(new juce::TextButton());
        addAndMakeVisible(outButton.get());
        outButton->setButtonText("OUT");
        outButton->onClick = [this] {outButtonClicked(); };
        //Out Label
        outLabel.reset(new juce::Label());
        addAndMakeVisible(outLabel.get());
        outLabel->setFont(juce::Font(25.0f, juce::Font::bold).withTypefaceStyle("Regular"));
        //Loop Button
        loopButton.reset(new juce::ToggleButton());
        addAndMakeVisible(loopButton.get());
        loopButton->setButtonText("Loop");
        loopButton->onClick = [this] {if (editedPlayer != nullptr) editedPlayer->updateLoopButton(loopButton.get(), "loop"); };
        //Enveloppe button
        enveloppeButton.reset(new juce::TextButton());
        addAndMakeVisible(enveloppeButton.get());
        enveloppeButton->setButtonText("Enveloppe");
        enveloppeButton->onClick = [this] {if (editedPlayer != nullptr) editedPlayer->envButtonClicked(); };
        //FX button
        fxButton.reset(new juce::TextButton());
        addAndMakeVisible(fxButton.get());
        fxButton->setButtonText("Effects");
        fxButton->onClick = [this] {if (editedPlayer != nullptr) editedPlayer->fxButtonClicked(); };
        //Normalise button
        normButton.reset(new juce::TextButton());
        addAndMakeVisible(normButton.get());
        normButton->setButtonText("Normalise");
        normButton->onClick = [this] {if (editedPlayer != nullptr) editedPlayer->normButtonClicked(); };
        //Denoise button
        denoiseButton.reset(new juce::TextButton());
        addAndMakeVisible(denoiseButton.get());
        denoiseButton->setButtonText("Denoiser");
        denoiseButton->onClick = [this] {if (editedPlayer != nullptr) editedPlayer->denoiseButtonClicked(); };
    }

    ~ClipEditor() override
    {
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
        g.setColour(BLUE);
        g.fillRect(getWidth() / 4 - dividerBarWidth, 0, dividerBarWidth, getHeight());
    }

    void resized() override
    {
        inOutButtonWidth = getWidth() / 24;
        enveloppeEditor.setBounds(getWidth() / 4, 0, getWidth() * 3 / 4, getHeight());
        nameLabel->setBounds(0, 0, getWidth() / 4 - dividerBarWidth, 25);
        trimVolumeSlider->setBounds(getWidth() / 4 - dividerBarWidth - volumeSliderWidth, nameLabel->getBottom(), volumeSliderWidth, volumeSliderWidth);
        cueButton->setBounds(2, nameLabel->getBottom() + 10, volumeSliderWidth, volumeSliderWidth - 10);
        cueTimeLabel->setBounds(cueButton->getRight() + 2, nameLabel->getBottom() + 5,
            trimVolumeSlider->getPosition().getX() - cueButton->getRight() - 4, 80);
        inButton->setBounds(0, cueButton->getBottom() + 10, inOutButtonWidth, 25);
        inLabel->setBounds(inButton->getRight(), inButton->getY(), inOutButtonWidth, 25);
        outButton->setBounds(inOutButtonWidth * 4, cueButton->getBottom() + 10, inOutButtonWidth, 25);
        outLabel->setBounds(outButton->getRight(), inButton->getY(), inOutButtonWidth, 25);
        loopButton->setBounds(getWidth() / 8 - inOutButtonWidth / 2, inButton->getY(), inOutButtonWidth, 25);
        int buttonsYPosition = loopButton->getBottom() + ((getHeight() - 25 - loopButton->getBottom()) / 4);
        int buttonsBisYPosition = loopButton->getBottom() + 3 * ((getHeight() - 25 - loopButton->getBottom()) / 4);
        int buttonsHeight = (getHeight() - 25 - loopButton->getBottom()) / 3;
        int buttonsWidth = getWidth() / 4 / 3;
        enveloppeButton->setCentrePosition(getWidth() / 16, buttonsYPosition);
        fxButton->setCentrePosition(3 * getWidth() / 16, buttonsYPosition);
        normButton->setCentrePosition(getWidth() / 16, buttonsBisYPosition);
        denoiseButton->setCentrePosition(3 * getWidth() / 16, buttonsBisYPosition);
        enveloppeButton->setSize(buttonsWidth, buttonsHeight);
        fxButton->setSize(buttonsWidth, buttonsHeight);
        normButton->setSize(buttonsWidth, buttonsHeight);
        denoiseButton->setSize(buttonsWidth, buttonsHeight);
    }

    void setPlayer(Player* p)
    {
        if (editedPlayer != nullptr)
        {
            editedPlayer->soundEditedBroadcaster->removeChangeListener(this);
        }

        editedPlayer = p;
        if (editedPlayer != nullptr)
        {
            enveloppeEditor.setEditedPlayer(editedPlayer);
            editedPlayer->soundEditedBroadcaster->addChangeListener(this);
            updateInfos();
        }
    }

    bool keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent)
    {
        if (isVisible())
            enveloppeEditor.keyPressed(key, originatingComponent);
        return true;
    }

    void changeListenerCallback(juce::ChangeBroadcaster* source)
    {
        if (source == editedPlayer->soundEditedBroadcaster)
        {
            updateInfos();
        }
    }

    void updateInfos()
    {
        if (editedPlayer != nullptr)
        {
            //Name
            nameLabel->setText(editedPlayer->getName(), juce::NotificationType::dontSendNotification);

            //Tril
            trimVolumeSlider->setValue(editedPlayer->getTrimVolume());

            //Cue Button
            if (editedPlayer->cueTransport.isPlaying())
                cueButton->setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::green);
            else
                cueButton->setColour(juce::TextButton::ColourIds::buttonColourId, 
                    getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

            //In button
            if (editedPlayer->isStartTimeSet())
                inButton->setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colour(0, 196, 255));
            else
                inButton->setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colour(0, 115, 150));

            //In label
            inLabel->setText(editedPlayer->secondsToMMSS(editedPlayer->getStart()), juce::NotificationType::dontSendNotification);

            //Out Button
            if (editedPlayer->isStopTimeSet())
            {
                outButton->setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colour(238, 255, 0));
                outButton->setColour(juce::TextButton::ColourIds::textColourOffId, juce::Colours::black);
            }
            else
            {
                outButton->setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colour(141, 150, 0));
                outButton->setColour(juce::TextButton::ColourIds::textColourOffId, juce::Colours::white);
            }
            //Out label
            outLabel->setText(editedPlayer->secondsToMMSS(editedPlayer->getStop()), juce::NotificationType::dontSendNotification);

            //Loop Button
            loopButton->setToggleState(editedPlayer->getIsLooping(), juce::NotificationType::dontSendNotification);
        }
    }

    void sliderValueChanged(juce::Slider* slider)
    {
        if (editedPlayer != nullptr)
        {
            if (slider == trimVolumeSlider.get())
            {
                editedPlayer->setTrimVolume(trimVolumeSlider->getValue());
            }
        }
    }

    void cueButtonClicked()
    {
        if (editedPlayer != nullptr)
        {
            editedPlayer->cueButtonClicked();
        }
    }

    void timerCallback()
    {
        if (editedPlayer != nullptr)
        {
            cueTimeLabel->setText(editedPlayer->getCueTimeAsString(), juce::NotificationType::dontSendNotification);
        }
    }

    void inButtonClicked()
    {
        if (editedPlayer != nullptr)
        {
            editedPlayer->setTimeClicked();
        }
    }

    void outButtonClicked()
    {
        if (editedPlayer != nullptr)
        {
            editedPlayer->stopTimeClicked();
        }
    }
private:
    EnveloppeEditor enveloppeEditor;
    Player* editedPlayer = nullptr;

    int dividerBarWidth = 4;
    int volumeSliderWidth = 80;
    int inOutButtonWidth = 50;

    std::unique_ptr<juce::Label> nameLabel;
    std::unique_ptr<juce::Slider> trimVolumeSlider;
    std::unique_ptr<juce::Label> cueTimeLabel;
    std::unique_ptr<juce::TextButton> cueButton;
    std::unique_ptr<juce::TextButton> inButton;
    std::unique_ptr<juce::TextButton> outButton;
    std::unique_ptr<juce::Label> inLabel;
    std::unique_ptr<juce::Label> outLabel;
    std::unique_ptr<juce::ToggleButton> loopButton;
    std::unique_ptr<juce::TextButton> enveloppeButton;
    std::unique_ptr<juce::TextButton> fxButton;
    std::unique_ptr<juce::TextButton> normButton;
    std::unique_ptr<juce::TextButton> denoiseButton;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ClipEditor)
};
