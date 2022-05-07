/*
  ==============================================================================

    updateDialog.h
    Created: 7 May 2022 11:38:52am
    Author:  Lucien Lefebvre

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../Settings/Settings.h"
//==============================================================================
/*
*/
class updateDialog : public juce::Component,
                     public juce::Button::Listener
{
public:
    updateDialog(juce::String& releaseNotes, Settings* s)
    {
        settings = s;
        newVersionLabel.reset(new juce::Label());
        addAndMakeVisible(newVersionLabel.get());
        newVersionLabel->setText("A new version of LuPlayer is available !", juce::dontSendNotification);
        newVersionLabel->setFont(juce::Font(30));
        newVersionLabel->setJustificationType(juce::Justification::centred);

        downloadButton.reset(new juce::HyperlinkButton("Go to download page", juce::URL("https://github.com/LucienLefebvre/LuPlayer/releases")));
        addAndMakeVisible(downloadButton.get());
        downloadButton->setFont(juce::Font(30), false);

        releaseNotesText.reset(new juce::TextEditor());
        addAndMakeVisible(releaseNotesText.get());
        releaseNotesText->setMultiLine(true, true);
        releaseNotesText->setReadOnly(true);
        releaseNotesText->setFont(juce::Font(17));
        releaseNotesText->setText(releaseNotes);

        autoCheckButton.reset(new juce::ToggleButton());
        addAndMakeVisible(autoCheckButton.get());
        autoCheckButton->setButtonText("Automatically check for new update on startup");
        autoCheckButton->addListener(this);
        autoCheckButton->setToggleState(Settings::autoCheckNewUpdate, juce::dontSendNotification);
    }

    ~updateDialog() override
    {
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    }

    void resized() override
    {
        newVersionLabel->setBounds(0, 0, getWidth(), 30);
        downloadButton->setBounds(0, newVersionLabel->getBottom() + 10, getWidth(), 30);
        int textY = downloadButton->getBottom() + 10;
        autoCheckButton->setBounds(0, getHeight() - 20, getWidth(), 20);
        releaseNotesText->setBounds(0, textY, getWidth(), autoCheckButton->getY() - textY);
    }

private:
    void buttonClicked(juce::Button* button)
    {
        if (button == autoCheckButton.get())
        {
            settings->setAutoCheckUpdate(button->getToggleState());
        }
    }
    std::unique_ptr<juce::Label> newVersionLabel;
    std::unique_ptr<juce::HyperlinkButton> downloadButton;
    std::unique_ptr<juce::TextEditor> releaseNotesText;
    std::unique_ptr<juce::ToggleButton> autoCheckButton;

    Settings* settings;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (updateDialog)
};
