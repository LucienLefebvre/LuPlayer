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
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    inputMeter = &initializationMeter;
    outputMeter = &initializationMeter;
    compMeter = &initializationMeter;
    addAndMakeVisible(&filterEditor);
    addAndMakeVisible(&displayInputMeter);
    addAndMakeVisible(&displayOutputMeter);
    addAndMakeVisible(&displayCompMeter);

    addAndMakeVisible(&nameLabel);
    nameLabel.setJustificationType(juce::Justification::centred);
    nameLabel.setFont(juce::Font(25.0f, juce::Font::bold).withTypefaceStyle("Regular"));
    nameLabel.setColour(juce::Label::ColourIds::textColourId, juce::Colour(229, 149, 0));
    nameLabel.setInterceptsMouseClicks(false, true);
    //nameLabel.setColour(juce::Label::ColourIds::backgroundColourId, juce::Colour(40, 134, 189));
    //nameLabel.setAlpha(0.7);

    displayCompMeter.shouldDrawScale(false);
    addAndMakeVisible(&compEditor);
    juce::Timer::startTimer(50);

    filterEditor.filterEditedBroadcaster->addChangeListener(this);
}

ClipEffect::~ClipEffect()
{
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
        displayInputMeter.setBounds(0, 0, meterSize, getHeight());
        displayOutputMeter.setBounds(getWidth() - meterSize, 0, meterSize, getHeight());
        compEditor.setBounds(displayOutputMeter.getX() - spaceBetweenComponents - compWidth - 10, 0, compWidth, getHeight());
        displayCompMeter.setReductionGainWidth(10);
        displayCompMeter.setBounds(displayOutputMeter.getX() - 20, 0, 20, getHeight());
        filterEditor.setBounds(displayInputMeter.getRight() + spaceBetweenComponents, 0, getWidth() - ((2 * meterSize) + (3 * spaceBetweenComponents) + compWidth), getHeight());
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
    displayInputMeter.setMeterData(inputMeter->getMeterData());
    displayOutputMeter.setMeterData(outputMeter->getMeterData());
    displayCompMeter.setReductionGain(compMeter->getReductionGain());
}

void ClipEffect::setPlayer(Player* p)
{
    if (editedPlayer != nullptr)
        editedPlayer->setFxEditedPlayer(false);

    editedPlayer = p;
    if (editedPlayer != nullptr)
    {
        setEditedFilterProcessor(editedPlayer->filterProcessor);
        setEditedCompProcessor(editedPlayer->compProcessor);
        setEditedBuffer(editedPlayer->getBuffer());
        setMeters(editedPlayer->getInputMeter(), editedPlayer->getOutputMeter(), editedPlayer->getCompMeter());
        editedPlayer->setFxEditedPlayer(true);
        setName(editedPlayer->getName());
        editedPlayer->fxButtonBroadcaster->addChangeListener(this);
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
            filterEditor.updateBypassed();
            compEditor.updateBypassedSliders();
            if (editedPlayer->getFilterProcessor().isBypassed())
                nameLabel.setColour(juce::Label::ColourIds::textColourId, juce::Colours::grey);
            else
                nameLabel.setColour(juce::Label::ColourIds::textColourId, juce::Colour(229, 149, 0));
        }
        else if (source == filterEditor.filterEditedBroadcaster)
        {
            editedPlayer->setFilterParameters(editedPlayer->getFilterParameters());
        }
    }
}

void ClipEffect::setFxEditedPlayer()
{
    if (editedPlayer != nullptr)
        editedPlayer->setFxEditedPlayer(true);
}