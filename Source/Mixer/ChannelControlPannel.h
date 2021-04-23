/*
  ==============================================================================

    ChannelControlPannel.h
    Created: 21 Apr 2021 11:10:16pm
    Author:  DPR

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "MixerInput.h"
#include "FilterProcessor.h"
#include "FilterEditor.h"
//#include "InputsControl.h"
//==============================================================================
/*
*/
class ChannelControlPannel  : public juce::Component, public juce::ComboBox::Listener, public juce::Label::Listener,
    public juce::Slider::Listener, public juce::ChangeListener
{
public:
    ChannelControlPannel()
    {
        channelColour = juce::Colour(juce::uint8(50), 62, 68, 1.0f);
        inputSelector.addListener(this);

        addAndMakeVisible(trimSlider);
        trimSlider.setRange(-24, 24, 0.5);
        trimSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        trimSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 50, 10);
        trimSlider.addListener(this);
        trimSlider.setDoubleClickReturnValue(true, 0.);
        trimSlider.setPopupDisplayEnabled(true, true, this, 2000);
        trimSlider.setScrollWheelEnabled(false);
        trimSlider.setWantsKeyboardFocus(false);
        trimSlider.setTextValueSuffix("dB");

        addAndMakeVisible(&inputNameLabel);
        inputNameLabel.setFont(juce::Font(15.00f, juce::Font::plain).withTypefaceStyle("Regular"));
        inputNameLabel.setText("INPUT", juce::NotificationType::dontSendNotification);
        inputNameLabel.setJustificationType(juce::Justification::centred);
        inputNameLabel.setEditable(true);
        inputNameLabel.addListener(this);

        addAndMakeVisible(&eqButton);
        eqButton.setButtonText("EQ");
        eqButton.setSize(80, 18);
        eqButton.onClick = [this] {eqButtonClicked(); };

        addAndMakeVisible(&gateButton);
        gateButton.setButtonText("Gate");
        gateButton.setSize(80, 18);

        addAndMakeVisible(&compButton);
        compButton.setButtonText("Compressor");
        compButton.setSize(80, 18);

        addAndMakeVisible(&deesserButton);
        deesserButton.setButtonText("Deeser");
        deesserButton.setSize(80, 18);

        addAndMakeVisible(&limiterButton);
        limiterButton.setButtonText("Limiter");
        limiterButton.setSize(80, 18);
        
        addAndMakeVisible(&optionButton);
        optionButton.onClick = [this] {optionButtonClicked(); };

        addAndMakeVisible(inputSelector);
        inputSelector.addListener(this);
    }

    ~ChannelControlPannel() override
    {

    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (channelColour);   // clear the background

    }

    void resized() override
    {
        inputNameLabel.setBounds(0, 0, 100, 20);
        inputSelector.setBounds(0, inputNameLabel.getBottom(), 100, 20);
        trimSlider.setBounds(10, inputSelector.getBottom(), 80, 80);
        eqButton.setCentrePosition(getWidth() / 2, trimSlider.getBottom() + 10);
        gateButton.setCentrePosition(getWidth() / 2, eqButton.getBottom() + 11);
        compButton.setCentrePosition(getWidth() / 2, gateButton.getBottom() + 11);
        deesserButton.setCentrePosition(getWidth() / 2, compButton.getBottom() + 11);
        limiterButton.setCentrePosition(getWidth() / 2, deesserButton.getBottom() + 11);
        optionButton.setBounds(0, getHeight() - 20, getWidth(), 20);
    }

    void setEditedProcessors(MixerInput& editedInput)
    {
        editedMixerInput = &editedInput;
        updateInputInfo();


    }
    void updateInputInfo()
    {
        //Colour
        channelColour = editedMixerInput->getInputColour();
        //name
        inputNameLabel.setText(editedMixerInput->getName(), juce::NotificationType::dontSendNotification);
        //selector
        inputSelector.setSelectedId(editedMixerInput->getSelectedInput() + 2, juce::NotificationType::dontSendNotification);
        //trim knob
        trimSlider.setValue(juce::Decibels::gainToDecibels(editedMixerInput->getTrimLevel()));
        //eq button
        if (!editedMixerInput->filterProcessor.isBypassed())
            eqButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colour(40, 134, 189));
        else
            eqButton.setColour(juce::TextButton::ColourIds::buttonColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        
        repaint();
    }
    void setEditedEditors(FilterEditor& f)
    {
        filterEditor = &f;
    }


    void setDeviceManagerInfos(juce::AudioDeviceManager& devicemanager)
    {
        deviceManager = &devicemanager;
        numInputsChannels = deviceManager->getCurrentAudioDevice()->getActiveInputChannels().getHighestBit() + 1;
        inputsChannelsName = deviceManager->getCurrentAudioDevice()->getInputChannelNames();
        
        updateInputSelectors();
    }

    void updateInputSelectors()
    {
        inputSelector.clear();
        inputSelector.addItem("None", 1);
        for (auto g = 0; g < numInputsChannels; g++)
        {
            inputSelector.addItem(inputsChannelsName[g], g + 2);
        }
    }

    void labelTextChanged(juce::Label* labelThatHasChanged)
    {
        if (labelThatHasChanged == &inputNameLabel)
        editedMixerInput->setName(labelThatHasChanged->getText());
    }

    void sliderValueChanged(juce::Slider* slider)
    {
        if (slider == &trimSlider)
            editedMixerInput->setTrimLevel(juce::Decibels::decibelsToGain(slider->getValue()));
    }

    void eqButtonClicked()
    {
        if (!editedMixerInput->filterProcessor.isBypassed())
        {
            editedMixerInput->filterProcessor.setBypassed(true);
            eqButton.setColour(juce::TextButton::ColourIds::buttonColourId, 
                                getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
            filterEditor->updateBypassed();
        }
        else
        {
            editedMixerInput->filterProcessor.setBypassed(false);
            eqButton.setColour(juce::TextButton::ColourIds::buttonColourId, 
                                juce::Colour(40, 134, 189));
            filterEditor->updateBypassed();
        }
    }

    void updateInputSelectorsState()
    {
        //feed an array of the inputs states (selected or not)
        //selectedInputs.clear();
        //selectedInputs.resize(numInputsChannels);
        //for (auto i = 0; i < selectedInputs.size(); i++)
        //    selectedInputs.set(i, false);
        //for (auto i = 0; i < inputsControl->inputs.size(); i++)
        //{
        //    if (inputsControl->inputs[i]->getSelectedInput() != -1)
        //        selectedInputs.set(inputsControl->inputs[i]->getSelectedInput(), true);
        //}

        ////update input selector
        //for (auto i = 0; i < inputsControl->inputs.size(); i++)
        //{
        //    for (auto g = 0; g < numInputsChannels; g++)
        //    {
        //        if (selectedInputs[g] && inputSelector.getSelectedId() != g)
        //            inputSelector.setItemEnabled(g + 2, false);
        //        else
        //            inputSelector.setItemEnabled(g + 2, true);
        //    }
        //}

    }
    void optionButtonClicked()
    {
        auto cs = std::make_unique<juce::ColourSelector>();
        cs->setName("colour");
        cs->addChangeListener(this);
        cs->setColour(juce::ColourSelector::backgroundColourId, juce::Colours::transparentBlack);
        cs->setCurrentColour(channelColour, juce::NotificationType::dontSendNotification);
        cs->setSize(300, 400);
        juce::CallOutBox::launchAsynchronously(std::move(cs), optionButton.getScreenBounds(), nullptr);

    }

    void changeListenerCallback(juce::ChangeBroadcaster* source)
    {
        if (juce::ColourSelector* cs = dynamic_cast <juce::ColourSelector*> (source))
        {
            channelColour = cs->getCurrentColour();
            editedMixerInput->setInputColour(channelColour);
            repaint();
        }

    }
private:
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
    {

        auto selectedInput = comboBoxThatHasChanged->getSelectedId() - 2;
        if (selectedInput >= -1)
            editedMixerInput->setSelectedInput(selectedInput);
    }

    //InputsControl* inputsControl = 0;
    MixerInput* editedMixerInput;
    FilterProcessor* editedFilterProcessor;
    CompProcessor* editedCompProcessor;
    FilterEditor* filterEditor;

    juce::TextButton optionButton;
    std::unique_ptr<juce::ColourSelector> colourSelector;

    juce::ComboBox inputSelector;
    juce::Slider trimSlider;
    juce::Label inputNameLabel;

    juce::Colour channelColour;

    juce::TextButton eqButton;
    juce::TextButton compButton;
    juce::TextButton gateButton;
    juce::TextButton limiterButton;
    juce::TextButton deesserButton;

    int numInputsChannels;
    juce::StringArray inputsChannelsName;
    juce::AudioDeviceManager* deviceManager;
    juce::Array<bool> selectedInputs;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChannelControlPannel)
};
