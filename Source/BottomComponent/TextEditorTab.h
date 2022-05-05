/*
  ==============================================================================

    TextEditorTab.h
    Created: 9 Apr 2022 2:32:00pm
    Author:  DPR

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "FocusAwareTextEditor.h"
//==============================================================================
/*
*/
class TextEditorTab  :  public juce::Component,
                        public juce::Slider::Listener,
                        public juce::ChangeListener
{
public:
    TextEditorTab()
    {
        addAndMakeVisible(&textEditor);
        textEditor.setMultiLine(true);
        textEditor.setReturnKeyStartsNewLine(true);
        textEditor.setFont(juce::Font(defaultFontHeight));
        textEditor.setColour(juce::TextEditor::ColourIds::focusedOutlineColourId, juce::Colours::red);
        textEditor.setPopupMenuEnabled(false);

        fontSizeSlider.reset(new juce::Slider());
        addAndMakeVisible(fontSizeSlider.get());
        fontSizeSlider->setSliderStyle(juce::Slider::IncDecButtons);
        fontSizeSlider->setRange(1, 50, 1);
        fontSizeSlider->setValue(defaultFontHeight);
        fontSizeSlider->addListener(this);
        fontSizeSlider->setIncDecButtonsMode(juce::Slider::incDecButtonsDraggable_Vertical);
        fontSizeSlider->setTextBoxStyle(juce::Slider::TextBoxLeft, true, 50, topControlsHeight);

        focusLabel.reset(new juce::Label);
        addChildComponent(focusLabel.get());
        focusLabel->setText("Warning ! Keyboard focus is on text editor. Click outside the editor to activate keyboards shortcuts", juce::dontSendNotification);
        focusLabel->setColour(juce::Label::ColourIds::backgroundColourId, juce::Colours::red);

        textEditor.textFocusGainedBroadcaster->addChangeListener(this);
        textEditor.textFocusLostBroadcaster->addChangeListener(this);
    }

    ~TextEditorTab() override
    {
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

    }

    void resized() override
    {
        textEditor.setBounds(0, topControlsHeight, getWidth(), getHeight() - topControlsHeight);
        fontSizeSlider->setBounds(0, 0, 125, topControlsHeight);
        focusLabel->setBounds(fontSizeSlider->getRight(), 0, 600, topControlsHeight);
    }

    void sliderValueChanged(juce::Slider* slider)
    {
        if (slider == fontSizeSlider.get())
        {
            textEditor.applyFontToAllText(juce::Font(slider->getValue()));
        }
    }

    void changeListenerCallback(juce::ChangeBroadcaster* source)
    {
        if (source == textEditor.textFocusLostBroadcaster.get())
        {
            focusLabel->setVisible(false);
        }
        else if (source == textEditor.textFocusGainedBroadcaster.get())
        {
            focusLabel->setVisible(true);
        }
    }
    FocusAwareTextEditor textEditor;
private:
    int topControlsHeight = 25;
    int defaultFontHeight = 16;

    std::unique_ptr<juce::Slider> fontSizeSlider;
    std::unique_ptr<juce::Label> focusLabel;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TextEditorTab)
};
