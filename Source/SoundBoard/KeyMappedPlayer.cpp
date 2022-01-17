/*
  ==============================================================================

    KeyMappedPlayer.cpp
    Created: 16 Jan 2022 9:32:26pm
    Author:  DPR

  ==============================================================================
*/

#include <JuceHeader.h>
#include "KeyMappedPlayer.h"
#define BLUE juce::Colour(40, 134, 189)
#define ORANGE juce::Colour(229, 149, 0)
//==============================================================================
KeyMappedPlayer::KeyMappedPlayer()
{
    shortcutLabel.reset(new juce::Label());
    addAndMakeVisible(shortcutLabel.get());
    shortcutLabel->setAlpha(0.25);
    shortcutLabel->setColour(juce::Label::ColourIds::textColourId, ORANGE);

    nameLabel.reset(new juce::Label());
    addAndMakeVisible(nameLabel.get());
    nameLabel->setFont(juce::Font(20.00f, juce::Font::plain).withTypefaceStyle("Regular"));

    elapsedTimeLabel.reset(new juce::Label());
    addAndMakeVisible(elapsedTimeLabel.get());
    elapsedTimeLabel->setFont(juce::Font(20.00f, juce::Font::plain).withTypefaceStyle("Regular"));

    volumeSlider.reset(new juce::Slider());
    addAndMakeVisible(volumeSlider.get());
    volumeSlider->setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    volumeSlider->setRange(0., 1., 0.01);
    volumeSlider->setValue(1.0);
    volumeSlider->addListener(this);
    volumeSlider->setSkewFactor(0.5, false);
    volumeSlider->setTextBoxStyle(juce::Slider::NoTextBox, false, 50, 15);
    volumeSlider->setNumDecimalPlacesToDisplay(2);
    volumeSlider->setWantsKeyboardFocus(false);
    volumeSlider->setDoubleClickReturnValue(true, 1.);
    if (!Settings::mouseWheelControlVolume)
        volumeSlider->setScrollWheelEnabled(false);

}

KeyMappedPlayer::~KeyMappedPlayer()
{
}

void KeyMappedPlayer::paint (juce::Graphics& g)
{
    //DRAW SURROUNDING & BACKGROUND
    if (soundPlayer->isPlayerPlaying() && !soundPlayer->isLastSeconds())
        g.setColour(juce::Colours::green);
    else if (soundPlayer->isLastSeconds() && soundPlayer->isPlayerPlaying())
        g.setColour(juce::Colours::red);
    else
        g.setColour(getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 15);

    g.setColour (BLUE);
    if (isDragged)
        g.setColour(juce::Colours::red);
    g.drawRoundedRectangle(getLocalBounds().toFloat(), 15, 2);

    //DRAW THUMBNAIL
    if (soundPlayer->isPlayerPlaying())
        g.setColour(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    thumbnail->drawChannels(g, thumbnailBounds, 0.0, thumbnail->getTotalLength(), juce::Decibels::decibelsToGain(playerInfos.trimVolume) * 2);
}

void KeyMappedPlayer::resized()
{
    nameLabelHeight = getHeight() / 5;
    nameLabel->setBounds(0, 0, getWidth(), nameLabelHeight);

    volumeSliderWidth = getWidth();
    volumeSliderHeight = getHeight() / 5;
    volumeSlider->setBounds(0, getHeight() * 7 / 10, volumeSliderWidth, volumeSliderHeight);
    
    thumbnailHeight = getHeight() * 2 / 5;
    thumbnailBounds.setBounds(0, getHeight() - 2*nameLabelHeight, getWidth(), thumbnailHeight);

    elapsedTimeWidth = getWidth() * 2 / 5;
    elapsedTimeHeight = getHeight() / 5;
    elapsedTimeLabel->setBounds(0, elapsedTimeHeight, elapsedTimeWidth, elapsedTimeHeight);

    shortcutLabelSize = getWidth();
    shortcutLabel->setBounds(0, 0, shortcutLabelSize, shortcutLabelSize);
    shortcutLabel->setFont(juce::Font(getWidth(), juce::Font::plain).withTypefaceStyle("Regular"));
    shortcutLabel->setJustificationType(juce::Justification::centred);
}

void KeyMappedPlayer::setPlayer(Player* p)
{
    soundPlayer = p;
    soundPlayer->playerInfoChangedBroadcaster->addChangeListener(this);
    thumbnail = &soundPlayer->getAudioThumbnail();
    thumbnail->addChangeListener(this);
    juce::Timer::startTimer(50);
    //thumbnail.reset(new juce::AudioThumbnail(521, soundPlayer->getAudioFormatManager(), soundPlayer->getAudioThumbnailCache()));
}

void KeyMappedPlayer::setShortcut(juce::String s)
{
    shortcutKey = s;
    shortcutLabel->setText(shortcutKey, juce::NotificationType::dontSendNotification);
}

void KeyMappedPlayer::isDraggedOver(bool b)
{
    isDragged = b;
    repaint();
}

void KeyMappedPlayer::loadFile(juce::String path, juce::String name)
{
    if (soundPlayer != nullptr)
    {
      soundPlayer->verifyAudioFileFormat(path);
      soundPlayer->setName(name.toStdString());
    }
}

void KeyMappedPlayer::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == soundPlayer->playerInfoChangedBroadcaster)
    {
        updatePlayerInfo();
    }
    else if (source == thumbnail)
    {
        repaint();
    }
}

void KeyMappedPlayer::updatePlayerInfo()
{
    playerInfos = soundPlayer->getPlayerInfo();
    nameLabel->setText(playerInfos.name, juce::NotificationType::dontSendNotification);
    repaint();
    elapsedTimeLabel->setText(soundPlayer->getRemainingTimeAsString(), juce::NotificationType::dontSendNotification);
}

void KeyMappedPlayer::shortcutKeyPressed()
{
    if (!soundPlayer->isPlayerPlaying())
        soundPlayer->play();
    else
        soundPlayer->stop();
}

void KeyMappedPlayer::sliderValueChanged(juce::Slider* slider)
{
    if (slider == volumeSlider.get())
    {

    }
}

void KeyMappedPlayer::timerCallback()
{
    if (soundPlayer != nullptr && soundPlayer->isFileLoaded())
    {
        elapsedTimeLabel->setText(soundPlayer->getRemainingTimeAsString(), juce::NotificationType::dontSendNotification);
    }
    repaint();
}