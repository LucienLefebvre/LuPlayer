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
class ClipEditor  : public juce::Component
{
public:
    ClipEditor()
    {
        addAndMakeVisible(&enveloppeEditor);
        nameLabel.reset(new juce::Label);
        addAndMakeVisible(nameLabel.get());
        nameLabel->setJustificationType(juce::Justification::centred);
        nameLabel->setFont(juce::Font(25.0f, juce::Font::bold).withTypefaceStyle("Regular"));
        nameLabel->setColour(juce::Label::ColourIds::textColourId, juce::Colour(229, 149, 0));
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
        enveloppeEditor.setBounds(getWidth() / 4, 0, getWidth() * 3 / 4, getHeight());
        nameLabel->setBounds(0, 0, getWidth() / 4 - dividerBarWidth, 25);


    }
    void setPlayer(Player* p)
    {
        editedPlayer = p;
        if (editedPlayer != nullptr)
        {
            enveloppeEditor.setEditedPlayer(editedPlayer);
            nameLabel->setText(editedPlayer->getName(), juce::NotificationType::dontSendNotification);
            DBG(editedPlayer->secon getStop());
        }
    }

    bool keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent)
    {
        if (isVisible())
            enveloppeEditor.keyPressed(key, originatingComponent);
        return true;
    }
private:
    EnveloppeEditor enveloppeEditor;
    Player* editedPlayer = nullptr;
    std::unique_ptr<juce::Label> nameLabel;
    int dividerBarWidth = 4;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ClipEditor)
};
