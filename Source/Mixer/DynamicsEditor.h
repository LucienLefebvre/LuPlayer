/*
  ==============================================================================

    DynamicsEditor.h
    Created: 27 Apr 2021 3:35:36pm
    Author:  DPR

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CompEditor.h"
#include "GateEditor.h"
#include "DeesserEditor.h"
#include "LimiterEditor.h"
#include "CompProcessor.h"
#include "Meter.h"
//==============================================================================
/*
*/
class DynamicsEditor  : public juce::Component, public juce::Timer
{
public:
    DynamicsEditor() :
        compReductionMeter(Meter::Mode::ReductionGain),
        gateReductionMeter(Meter::Mode::ReductionGain),
        deesserReductionMeter(Meter::Mode::ReductionGain),
        limiterReductionMeter(Meter::Mode::ReductionGain),
        enableButtonBroadcaster()
    {
        juce::Timer::startTimer(50);

        addAndMakeVisible(&compReductionMeter);
        addAndMakeVisible(&gateReductionMeter);
        addAndMakeVisible(&deesserReductionMeter);
        addAndMakeVisible(&limiterReductionMeter);

        addAndMakeVisible(&dynamicButton);
        dynamicButton.setButtonText("Compressor");
        dynamicButton.onClick = [this] {dynamicsMenuClicked(); };

        addAndMakeVisible(&enableButton);
        enableButton.setButtonText("Enabled");
        enableButton.onClick = [this] {enableButtonClicked(); };

        dynamicsMenu.addItem(1, "Gate");
        dynamicsMenu.addItem(2, "Compressor");
        //dynamicsMenu.addItem(3, "Deesser");
        dynamicsMenu.addItem(4, "Limiter");

        addChildComponent(&compEditor);
        addChildComponent(&gateEditor);
        addChildComponent(&deesserEditor);
        addChildComponent(&limiterEditor);
        setViewedEditor(2);
    }

    ~DynamicsEditor() override
    {
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
        g.setColour(juce::Colours::black);
        g.setOpacity(0.8f);
        g.drawLine(compEditor.getRight(), 0, compEditor.getRight(), getHeight(), 2);
        g.drawLine(gateReductionMeter.getRight(), 0, gateReductionMeter.getRight(), getHeight(), 2);
        g.drawLine(compReductionMeter.getRight(), 0, compReductionMeter.getRight(), getHeight(), 2);
        //g.drawLine(deesserReductionMeter.getRight(), 0, deesserReductionMeter.getRight(), getHeight(), 2);
        g.drawLine(limiterReductionMeter.getRight(), 0, limiterReductionMeter.getRight(), getHeight(), 2);
    }

    void resized() override

    {
        dynamicButton.setBounds(2, getHeight() - buttonsHeight - 2, 76, buttonsHeight);
        enableButton.setBounds(80 + 1, getHeight() - buttonsHeight - 2, 56, buttonsHeight);
        compEditor.setBounds(0, 0, 140, getHeight() - buttonsHeight - 2);
        gateEditor.setBounds(0, 0, 140, getHeight() - buttonsHeight - 2);
        deesserEditor.setBounds(0, 0, 140, getHeight() - buttonsHeight - 2);
        limiterEditor.setBounds(0, 0, 140, getHeight() - buttonsHeight - 2);
        gateReductionMeter.setBounds(compEditor.getRight() + 2, 0, 10, getHeight());
        compReductionMeter.setBounds(gateReductionMeter.getRight() + 2, 0, 10, getHeight());
        //deesserReductionMeter.setBounds(compReductionMeter.getRight() + 2, 0, 10, getHeight());
        limiterReductionMeter.setBounds(compReductionMeter.getRight() + 2, 0, 10, getHeight());
    }

    void setEditedCompProcessor(CompProcessor& processor)
    {
        editedCompProcessor = &processor;
        compEditor.setEditedCompProcessor(processor);
        gateEditor.setEditedCompProcessor(processor);
        deesserEditor.setEditedCompProcessor(processor);
        limiterEditor.setEditedCompProcessor(processor);
        updateEnableButton();

    }

    void dynamicsMenuClicked()
    {
        int result = dynamicsMenu.show();
        setViewedEditor(result);
    }

    void enableButtonClicked()
    {
        if (displayedEditor > 0)
        {
            switch (displayedEditor)
            {
            case 1 :
                if (editedCompProcessor->getGateParams().bypassed)
                    editedCompProcessor->setGateBypass(false);
                else
                    editedCompProcessor->setGateBypass(true);
                break;
            case 2 :
                if (editedCompProcessor->getCompParams().bypassed)
                    editedCompProcessor->setBypass(false);
                else
                    editedCompProcessor->setBypass(true);
                break;
            case 4:
                if (editedCompProcessor->getLimitParams().bypassed)
                    editedCompProcessor->setLimitBypass(false);
                else
                    editedCompProcessor->setLimitBypass(true);
                break;
            }
            updateEnableButton();
        }
        enableButtonBroadcaster.sendChangeMessage();
    }

    void updateEnableButton()
    {
        if (editedCompProcessor != nullptr)
        {
            auto result = false;
            switch (displayedEditor)
            {
            case 1:
                result = editedCompProcessor->getGateParams().bypassed;
                break;
            case 2:
                result = editedCompProcessor->getCompParams().bypassed;
                break;
            case 4:
                result = editedCompProcessor->getLimitParams().bypassed;
                break;
            }
            if (result == true)
            {
                enableButton.setColour(juce::TextButton::ColourIds::buttonColourId,
                    getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
                enableButton.setButtonText("Bypassed");
            }
            else
            {
                enableButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colour(40, 134, 189));
                enableButton.setButtonText("Enabled");
            }
        }
    }

    
    void setViewedEditor(int editorIndex)
    {
        displayedEditor = editorIndex;
        switch (editorIndex)
        {
        case 1 :
            compEditor.setVisible(false);
            gateEditor.setVisible(true);
            deesserEditor.setVisible(false);
            limiterEditor.setVisible(false);
            dynamicButton.setButtonText("Gate");
            break;
        case 2 :
            compEditor.setVisible(true);
            gateEditor.setVisible(false);
            deesserEditor.setVisible(false);
            limiterEditor.setVisible(false);
            dynamicButton.setButtonText("Compressor");
            break;
        case 3 :
            compEditor.setVisible(false);
            gateEditor.setVisible(false);
            deesserEditor.setVisible(true);
            limiterEditor.setVisible(false);
            dynamicButton.setButtonText("De-esser");
            break;
        case 4 :
            compEditor.setVisible(false);
            gateEditor.setVisible(false);
            deesserEditor.setVisible(false);
            limiterEditor.setVisible(true);
            dynamicButton.setButtonText("Limiter");
            break;
        }
        updateEnableButton();
    }

    void timerCallback()
    {
        compReductionMeter.setReductionGain(editedCompProcessor->getCompReductionDB());
        gateReductionMeter.setReductionGain(editedCompProcessor->getGateReductionDB());
        limiterReductionMeter.setReductionGain(editedCompProcessor->getGateReductionDB());
        repaint();
    }

    juce::ChangeBroadcaster enableButtonBroadcaster;
private:
    int buttonsHeight = 20;
    juce::TextButton dynamicButton;
    juce::PopupMenu dynamicsMenu;
    juce::TextButton enableButton;

    int displayedEditor = 0;

    CompProcessor* editedCompProcessor = 0;

    CompEditor compEditor;
    GateEditor gateEditor;
    DeesserEditor deesserEditor;
    LimiterEditor limiterEditor;

    Meter compReductionMeter;
    Meter gateReductionMeter;
    Meter deesserReductionMeter;
    Meter limiterReductionMeter;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DynamicsEditor)
};
