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
    ChannelControlPannel() : deleteBroadcaster()
    {
        channelColour = juce::Colour(juce::uint8(50), 62, 68, 1.0f);
        inputSelector.addListener(this);

        addAndMakeVisible(trimSlider);
        trimSlider.setRange(-24, 24, 0.5);
        trimSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        trimSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 15);
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
        eqButton.setButtonText("E");
        eqButton.setSize(80, 18);
        eqButton.onClick = [this] {eqButtonClicked(); };

        addAndMakeVisible(&gateButton);
        gateButton.setButtonText("G");
        gateButton.setSize(80, 18);

        addAndMakeVisible(&compButton);
        compButton.setButtonText("C");
        compButton.setSize(80, 18);

        addAndMakeVisible(&deesserButton);
        deesserButton.setButtonText("D");
        deesserButton.setSize(80, 18);

        addAndMakeVisible(&limiterButton);
        limiterButton.setButtonText("L");
        limiterButton.setSize(80, 18);
        
        addAndMakeVisible(&optionButton);
        optionButton.setButtonText("Options");
        optionButton.onClick = [this] {optionButtonClicked(); };

        optionMenu.addItem(1, "Save preset", true, false);
        optionMenu.addItem(2, "Load preset", true, false);
        optionMenu.addItem(3, "Colour", true, false);
        optionMenu.addItem(4, "Delete Input", true, false);
        addAndMakeVisible(inputSelector);
        inputSelector.addListener(this);

        addAndMakeVisible(&vcaButton);
        vcaButton.setButtonText("VCA");
        vcaButton.onClick = [this] {vcaButtonClicked(); };
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
        trimSlider.setBounds(-10, inputSelector.getBottom(), 80, 80);
        vcaButton.setBounds(trimSlider.getRight() - 10, trimSlider.getY() + 5, getWidth() - trimSlider.getRight() + 10, 20);
        eqButton.setBounds(1, trimSlider.getBottom() + 5, bypassButtonsSize, bypassButtonsSize);
        gateButton.setBounds(21, trimSlider.getBottom() + 5, bypassButtonsSize, bypassButtonsSize);
        compButton.setBounds(41, trimSlider.getBottom() + 5, bypassButtonsSize, bypassButtonsSize);
        deesserButton.setBounds(61, trimSlider.getBottom() + 5, bypassButtonsSize, bypassButtonsSize);
        limiterButton.setBounds(81, trimSlider.getBottom() + 5, bypassButtonsSize, bypassButtonsSize);

        optionButton.setBounds(0, getHeight() - 20, getWidth(), 20);

    }

    void setEditedProcessors(MixerInput& editedInput)
    {
        editedMixerInput = &editedInput;
        updateInputInfo();


    }

    void prepareToPlay()
    {
        //updateInputSelectors();
    }
    void updateInputInfo()
    {
        //Colour
        setChannelColour(editedMixerInput->getInputColour());
        //name
        inputNameLabel.setText(editedMixerInput->getName(), juce::NotificationType::dontSendNotification);
        //selector
        
        updateInputSelectors();

        //trim knob
        trimSlider.setValue(juce::Decibels::gainToDecibels(editedMixerInput->getTrimLevel()));
        //eq button
        if (!editedMixerInput->filterProcessor.isBypassed())
            eqButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colour(40, 134, 189));
        else
            eqButton.setColour(juce::TextButton::ColourIds::buttonColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        //VCA
        if (!editedMixerInput->isVCAAssigned())
            vcaButton.setColour(juce::TextButton::ColourIds::buttonColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        else
            vcaButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::red);

        repaint();
    }
    void setEditedEditors(FilterEditor& f)
    {
        filterEditor = &f;
    }


    void setDeviceManagerInfos(juce::AudioDeviceManager& devicemanager)
    {
        deviceManager = &devicemanager;
        if (deviceManager != nullptr)
        {
            numActiveInputsChannels = deviceManager->getCurrentAudioDevice()->getActiveInputChannels().countNumberOfSetBits();
            DBG("num inputs channel " << numActiveInputsChannels);
            inputsChannelsName = deviceManager->getCurrentAudioDevice()->getInputChannelNames();

            updateInputSelectors();
        }
    }

    void updateInputSelectors()
    {
        inputSelector.clear();
        inputSelector.addItem("None", 1);
        if (deviceManager != nullptr)
        {
            numInputsChannels = deviceManager->getCurrentAudioDevice()->getActiveInputChannels().getHighestBit() + 1;

            for (auto g = 0; g < numInputsChannels; g++)
            {
                if (deviceManager->getCurrentAudioDevice()->getActiveInputChannels()[g] == 1)
                {
                    if (editedMixerInput->getInputParams().mode == MixerInput::Mode::Mono)
                    {
                        inputSelector.addItem(inputsChannelsName[g], inputSelector.getNumItems() + 1);
                    }
                    else if (editedMixerInput->getInputParams().mode == MixerInput::Mode::Stereo)
                    {
                        juce::String nameToAdd = juce::String(inputsChannelsName[g] + " + " + inputsChannelsName[g + 1]);
                        inputSelector.addItem(nameToAdd, inputSelector.getNumItems() + 1);
                        g++;
                    }
                }
            }
        }
        inputSelector.setSelectedId(editedMixerInput->getSelectedInput() + 2, juce::NotificationType::dontSendNotification);
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

    void vcaButtonClicked()
    {
        if (editedMixerInput->isVCAAssigned())
        {
            editedMixerInput->setVCAAssigned(false);
            vcaButton.setColour(juce::TextButton::ColourIds::buttonColourId,
                getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        }
        else
        {
            editedMixerInput->setVCAAssigned(true);
            vcaButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::red);
        }
    }
    void updateInputSelectorsState()
    {
        //feed an array of the inputs states (selected or not)
        //selectedInputs.clear();
        //selectedInputs.resize(numActiveInputsChannels);
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
        //    for (auto g = 0; g < numActiveInputsChannels; g++)
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
        int result = optionMenu.show();
        switch (result)
        {
        case 1 :
            break;
        case 2 :
            break;
        case 3 :
            colourButtonClicked();
            break;
        case 4 :
            deleteInput();
            break;
        }
    }

    void colourButtonClicked()
    {
        auto cs = std::make_unique<juce::ColourSelector>();
        cs->setName("colour");
        cs->addChangeListener(this);
        cs->setColour(juce::ColourSelector::backgroundColourId, juce::Colours::transparentBlack);
        cs->setCurrentColour(channelColour, juce::NotificationType::dontSendNotification);
        cs->setSize(300, 400);
        juce::CallOutBox::launchAsynchronously(std::move(cs), inputNameLabel.getScreenBounds(), nullptr);
    }

    void deleteInput()
    {
        deleteBroadcaster.sendChangeMessage();

    }
    void changeListenerCallback(juce::ChangeBroadcaster* source)
    {
        if (juce::ColourSelector* cs = dynamic_cast <juce::ColourSelector*> (source))
        {
            setChannelColour(cs->getCurrentColour());
        }

    }
    void setChannelColour(juce::Colour c)
    {
        channelColour = c;
        editedMixerInput->setInputColour(c);
        repaint();
    }

    juce::ChangeBroadcaster deleteBroadcaster;
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
    juce::PopupMenu optionMenu;

    juce::ComboBox inputSelector;
    juce::Slider trimSlider;
    juce::Label inputNameLabel;

    juce::Colour channelColour;

    int bypassButtonsSize = 18;
    juce::TextButton eqButton;
    juce::TextButton compButton;
    juce::TextButton gateButton;
    juce::TextButton limiterButton;
    juce::TextButton deesserButton;

    juce::TextButton vcaButton;

    int numInputsChannels = 0;
    int numActiveInputsChannels = 0;
    juce::StringArray inputsChannelsName;
    juce::AudioDeviceManager* deviceManager = 0;
    juce::Array<bool> selectedInputs;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChannelControlPannel)
};
