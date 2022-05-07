/*
  ==============================================================================

    updateDialog.h
    Created: 7 May 2022 11:38:52am
    Author:  DPR

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class updateDialog  : public juce::Component
{
public:
    updateDialog(juce::String& releaseNotes)
    {
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
    }

    ~updateDialog() override
    {
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
    }

    void resized() override
    {
        newVersionLabel->setBounds(0, 0, getWidth(), 30);
        downloadButton->setBounds(0, newVersionLabel->getBottom() + 10, getWidth(), 30);
        int textY = downloadButton->getBottom() + 10;
        releaseNotesText->setBounds(0, textY, getWidth(), getHeight() - textY);
    }

private:
    std::unique_ptr<juce::Label> newVersionLabel;
    std::unique_ptr<juce::HyperlinkButton> downloadButton;
    std::unique_ptr<juce::TextEditor> releaseNotesText;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (updateDialog)
};
