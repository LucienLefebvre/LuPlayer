/*
  ==============================================================================

    ChannelControlPannel.h
    Created: 21 Apr 2021 11:10:16pm
    Author:  Lucien Lefebvre

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "MixerInput.h"
#include "FilterProcessor.h"
#include "FilterEditor.h"
#include "DynamicsEditor.h"
//#include "InputsControl.h"
//==============================================================================
/*
*/
class ChannelControlPannel  : public juce::Component,
    public juce::ComboBox::Listener, 
    public juce::Label::Listener,
    public juce::Slider::Listener, 
    public juce::ChangeListener
{
public:
    enum ToolBarItemIDs
    {
        Open_Sound = 1,
        Delete_Sound,
        Select_Colour
    };
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
        eqButton.setButtonText("EQ");
        eqButton.setSize(80, 18);
        eqButton.onClick = [this] {eqButtonClicked(); };

        addAndMakeVisible(&gateButton);
        gateButton.setButtonText("Gate");
        gateButton.setSize(80, 18);
        gateButton.onClick = [this] {gateButtonCLicked(); };

        addAndMakeVisible(&compButton);
        compButton.setButtonText("Comp");
        compButton.setSize(80, 18);
        compButton.onClick = [this] {compButtonCLicked(); };

        /*addAndMakeVisible(&deesserButton);
        deesserButton.setButtonText("D");
        deesserButton.setSize(80, 18);*/

        addAndMakeVisible(&limiterButton);
        limiterButton.setButtonText("Limit");
        limiterButton.setSize(80, 18);
        limiterButton.onClick = [this] {limitButtonCLicked(); };

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
        trimSlider.setBounds(10, inputSelector.getBottom(), 80, 80);

        eqButton.setBounds(1, trimSlider.getBottom() + 15, bypassButtonsWidth, bypassButtonsHeight);
        gateButton.setBounds(51, trimSlider.getBottom() + 15, bypassButtonsWidth, bypassButtonsHeight);
        compButton.setBounds(1, eqButton.getBottom() + 5, bypassButtonsWidth, bypassButtonsHeight);
        //deesserButton.setBounds(61, trimSlider.getBottom() + 5, bypassButtonsWidth, bypassButtonsHeight);
        limiterButton.setBounds(51, eqButton.getBottom() + 5, bypassButtonsWidth, bypassButtonsHeight);

        vcaButton.setBounds(1, compButton.getBottom() + 20 , 98, 20);

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
        //gate Button
        if (!editedMixerInput->compProcessor.getGateParams().bypassed)
            gateButton.setColour(juce::TextButton::ColourIds::buttonColourId,
                juce::Colour(40, 134, 189));
        else
            gateButton.setColour(juce::TextButton::ColourIds::buttonColourId,
                getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        //comp Button
        if (!editedMixerInput->compProcessor.getCompParams().bypassed)
            compButton.setColour(juce::TextButton::ColourIds::buttonColourId,
                juce::Colour(40, 134, 189));
        else
            compButton.setColour(juce::TextButton::ColourIds::buttonColourId,
                getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        //limit Button
        if (!editedMixerInput->compProcessor.getLimitParams().bypassed)
            limiterButton.setColour(juce::TextButton::ColourIds::buttonColourId,
                juce::Colour(40, 134, 189));
        else
            limiterButton.setColour(juce::TextButton::ColourIds::buttonColourId,
                getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

        //VCA
        if (!editedMixerInput->isVCAAssigned())
            vcaButton.setColour(juce::TextButton::ColourIds::buttonColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        else
            vcaButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::red);

        repaint();
    }

    void setEditedEditors(FilterEditor& f, DynamicsEditor& d)
    {
        filterEditor = &f;
        dynmamicsEditor = &d;
        dynmamicsEditor->enableButtonBroadcaster.addChangeListener(this);
    }

    void setDeviceManagerInfos(juce::AudioDeviceManager& devicemanager)
    {
        deviceManager = &devicemanager;
        if (deviceManager != nullptr)
        {
            numActiveInputsChannels = deviceManager->getCurrentAudioDevice()->getActiveInputChannels().countNumberOfSetBits();
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

    void gateButtonCLicked()
    {
        if (!editedMixerInput->compProcessor.getGateParams().bypassed)
            editedMixerInput->compProcessor.setGateBypass(true);
        else
        {
            editedMixerInput->compProcessor.setGateBypass(false);
            dynmamicsEditor->setViewedEditor(1);
        }
        updateInputInfo();
        dynmamicsEditor->updateEnableButton();
    }

    void compButtonCLicked()
    {
        if (!editedMixerInput->compProcessor.getCompParams().bypassed)
            editedMixerInput->compProcessor.setBypass(true);
        else
        {
            editedMixerInput->compProcessor.setBypass(false);
            dynmamicsEditor->setViewedEditor(2);
        }
        updateInputInfo();
        dynmamicsEditor->updateEnableButton();
    }

    void limitButtonCLicked()
    {
        if (!editedMixerInput->compProcessor.getLimitParams().bypassed)
            editedMixerInput->compProcessor.setLimitBypass(true);
        else
        {
            editedMixerInput->compProcessor.setLimitBypass(false);
            dynmamicsEditor->setViewedEditor(4);
        }
        updateInputInfo();
        dynmamicsEditor->updateEnableButton();
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
        if (source == &dynmamicsEditor->enableButtonBroadcaster)
            updateInputInfo();
        else if (juce::ColourSelector* cs = dynamic_cast <juce::ColourSelector*> (source))
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
    DynamicsEditor* dynmamicsEditor;

    juce::TextButton optionButton;
    std::unique_ptr<juce::ColourSelector> colourSelector;
    juce::PopupMenu optionMenu;

    juce::ComboBox inputSelector;
    juce::Slider trimSlider;
    juce::Label inputNameLabel;

    juce::Colour channelColour;

    int bypassButtonsWidth = 48;
    int bypassButtonsHeight = 20;
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
