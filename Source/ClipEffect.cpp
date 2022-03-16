/*
  ==============================================================================

    ClipEffect.cpp
    Created: 12 Jan 2022 4:22:43pm
    Author:  DPR

  ==============================================================================
*/

#include <JuceHeader.h>
#include "ClipEffect.h"

//==============================================================================
ClipEffect::ClipEffect()
{
    inputMeter = &initializationMeter;
    outputMeter = &initializationMeter;
    compMeter = &initializationMeter;
    addAndMakeVisible(&filterEditor);
    addAndMakeVisible(&displayInputMeter);
    addAndMakeVisible(&displayOutputMeter);
    addAndMakeVisible(&displayCompMeter);

    addAndMakeVisible(&bypassButton);
    bypassButton.setButtonText("Enable");
    bypassButton.onClick = [this] { bypassButtonClicked(); };

    addAndMakeVisible(&nameLabel);
    nameLabel.setJustificationType(juce::Justification::centred);
    nameLabel.setFont(juce::Font(25.0f, juce::Font::bold).withTypefaceStyle("Regular"));
    nameLabel.setColour(juce::Label::ColourIds::textColourId, juce::Colour(229, 149, 0));
    nameLabel.setInterceptsMouseClicks(false, true);

    displayCompMeter.shouldDrawScale(false);
    addAndMakeVisible(&compEditor);
    juce::Timer::startTimer(50);

    filterEditor.filterEditedBroadcaster->addChangeListener(this);
}

ClipEffect::~ClipEffect()
{
    setNullPlayer();
}

void ClipEffect::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    filterEditor.prepareToPlay(samplesPerBlockExpected, sampleRate);
    displayInputMeter.prepareToPlay(samplesPerBlockExpected, sampleRate);
    displayOutputMeter.prepareToPlay(samplesPerBlockExpected, sampleRate);
    displayCompMeter.prepareToPlay(samplesPerBlockExpected, sampleRate);
    dummyFilterProcessor.prepareToPlay(samplesPerBlockExpected, sampleRate);
    dummyPlayer.playerPrepareToPlay(samplesPerBlockExpected, sampleRate);
}

void ClipEffect::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
}

void ClipEffect::resized()
{
    if (editedPlayer != nullptr)
    {
        bypassButton.setBounds(0, 0, bypassButtonWidth, byPassButtonHeight);
        displayInputMeter.setBounds(bypassButton.getRight() + spaceBetweenComponents / 2, 0, meterSize, getHeight());
        displayOutputMeter.setBounds(getWidth() - meterSize, 0, meterSize, getHeight());
        compEditor.setBounds(displayOutputMeter.getX() - spaceBetweenComponents - compWidth - 10, 0, compWidth, getHeight());
        displayCompMeter.setReductionGainWidth(10);
        displayCompMeter.setBounds(displayOutputMeter.getX() - 20, 0, 20, getHeight());
        filterEditor.setBounds(displayInputMeter.getRight() + spaceBetweenComponents / 2, 0, getWidth() - ((2 * meterSize) + (4 * spaceBetweenComponents) + compWidth + bypassButtonWidth), getHeight());
        nameLabel.setBounds(filterEditor.getFilterGraphXStart() + 100, 2, filterEditor.getRight() - filterEditor.getFilterGraphXStart() - 100, 30);
    }
}

void ClipEffect::setEditedFilterProcessor(FilterProcessor& fp)
{
    filterEditor.setEditedFilterProcessor(fp);
}

void ClipEffect::setEditedCompProcessor(CompProcessor& cp)
{
    compEditor.setEditedCompProcessor(cp);
}
void ClipEffect::setEditedBuffer(std::unique_ptr<juce::AudioBuffer<float>>& buffer)
{
    editedBuffer = &buffer;
}

void ClipEffect::setMeters(Meter& inputM, Meter& outputM, Meter& compM)
{
    inputMeter = &inputM;
    outputMeter = &outputM;
    compMeter = &compM;
}

void ClipEffect::setName(std::string n)
{
    name = juce::String(n);
    nameLabel.setText(name, juce::NotificationType::dontSendNotification);
}

void ClipEffect::timerCallback()
{
    if (editedPlayer != nullptr)
    {
        displayInputMeter.setRMSMeterData(editedPlayer->meterSource.getRMSLevel(0), editedPlayer->meterSource.getRMSLevel(1));
        displayInputMeter.setPeakMeterDate(editedPlayer->meterSource.getMaxLevel(0), editedPlayer->meterSource.getMaxLevel(1));
    }
    if (editedPlayer != nullptr)
    {
        displayOutputMeter.setRMSMeterData(editedPlayer->outMeterSource.getRMSLevel(0), editedPlayer->outMeterSource.getRMSLevel(1));
        displayOutputMeter.setPeakMeterDate(editedPlayer->outMeterSource.getMaxLevel(0), editedPlayer->outMeterSource.getMaxLevel(1));
    }
    if (editedPlayer != nullptr)
        displayCompMeter.setReductionGain(compMeter->getReductionGain());
}

void ClipEffect::setPlayer(Player* p)
{
    if (editedPlayer != nullptr)
    {
        editedPlayer->setEditedPlayer(false);
        editedPlayer->fxButtonBroadcaster->removeChangeListener(this);
        editedPlayer->playerDeletedBroadcaster->removeChangeListener(this);
        editedPlayer->soundEditedBroadcaster->removeChangeListener(this);
    }

    editedPlayer = p;
    if (editedPlayer != nullptr)
    {
        setEditedFilterProcessor(editedPlayer->filterProcessor);
        setEditedCompProcessor(editedPlayer->compProcessor);
        setEditedBuffer(editedPlayer->getBuffer());
        setMeters(editedPlayer->getInputMeter(), editedPlayer->getOutputMeter(), editedPlayer->getCompMeter());
        editedPlayer->setEditedPlayer(true);
        setName(editedPlayer->getName());
        editedPlayer->fxButtonBroadcaster->addChangeListener(this);
        editedPlayer->playerDeletedBroadcaster->addChangeListener(this);
        editedPlayer->soundEditedBroadcaster->addChangeListener(this);
        updateBypassed();
    }
    else
        setEditedFilterProcessor(dummyFilterProcessor);

    resized();
}

void ClipEffect::setDummyPlayer()
{
    setPlayer(&dummyPlayer);
}

void ClipEffect::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (editedPlayer != nullptr)
    {
        if (source == editedPlayer->fxButtonBroadcaster)
        {
            updateBypassed();
        }
        else if (source == filterEditor.filterEditedBroadcaster)
        {
            editedPlayer->setFilterParameters(editedPlayer->getFilterParameters());
        }
        else if (source == editedPlayer->playerDeletedBroadcaster)
        {
            setNullPlayer();
        }
        else if (source == editedPlayer->soundEditedBroadcaster)
            setName(editedPlayer->getName());
    }
}

void ClipEffect::setFxEditedPlayer()
{
    if (editedPlayer != nullptr)
        editedPlayer->setEditedPlayer(true);
}

void ClipEffect::setNullPlayer()
{
    editedPlayer = nullptr;
    filterEditor.setNullProcessor();
    nameLabel.setText("", juce::NotificationType::dontSendNotification);
}

void ClipEffect::updateBypassed()
{
    if (editedPlayer != nullptr)
    {
        filterEditor.updateBypassed();
        compEditor.updateBypassedSliders();
        if (filterEditor.getEditedFilterProcessor()->isBypassed())
        {
            nameLabel.setColour(juce::Label::ColourIds::textColourId, juce::Colours::grey);
            bypassButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::red);
            bypassButton.setButtonText("Enable");
        }
        else
        {
            nameLabel.setColour(juce::Label::ColourIds::textColourId, juce::Colour(229, 149, 0));
            bypassButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::green);
            bypassButton.setButtonText("Bypass");
        }
    }
}

void ClipEffect::bypassButtonClicked()
{
    juce::FileLogger::getCurrentLogger()->writeToLog("clip editor bypass button pressed");
    if (editedPlayer != nullptr)
    {
        editedPlayer->bypassFX(editedPlayer->isFxEnabled(), true);
    }
}

void ClipEffect::visibilityChanged()
{
    updateBypassed();
}