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
    currentColour = BLUE;

    shortcutLabel.reset(new juce::Label());
    addAndMakeVisible(shortcutLabel.get());
    shortcutLabel->setAlpha(0.25);
    shortcutLabel->setColour(juce::Label::ColourIds::textColourId, ORANGE);

    nameLabel.reset(new juce::Label());
    addAndMakeVisible(nameLabel.get());
    nameLabel->setFont(juce::Font(20.00f, juce::Font::plain).withTypefaceStyle("Regular"));
    nameLabel->setMinimumHorizontalScale(0.5);

    elapsedTimeLabel.reset(new juce::Label());
    addChildComponent(elapsedTimeLabel.get());
    elapsedTimeLabel->setFont(juce::Font(20.00f, juce::Font::plain).withTypefaceStyle("Regular"));

    volumeSlider.reset(new juce::Slider());
    addChildComponent(volumeSlider.get());
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
    //if (soundPlayer->isPlayerPlaying() && !soundPlayer->isLastSeconds())
    //    g.setColour(juce::Colours::green);
    //else if (soundPlayer->isLastSeconds() && soundPlayer->isPlayerPlaying())
    //    g.setColour(juce::Colours::red);
    //else
    //    g.setColour(getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
    //g.fillRoundedRectangle(getLocalBounds().toFloat(), 15);


    g.setColour(currentColour);
    if (isDragged)
        g.setColour(juce::Colours::red);
    g.drawRoundedRectangle(getLocalBounds().toFloat(), 15, 2);

    //DRAW THUMBNAIL

    g.setColour(currentColour);
    thumbnail->drawChannels(g, thumbnailBounds, 0.0, thumbnail->getTotalLength(), juce::Decibels::decibelsToGain(playerInfos.trimVolume) * 2.0f);
}

void KeyMappedPlayer::resized()
{
    nameLabelHeight = getHeight() / 5;
    nameLabel->setBounds(- nameLabelScrollX, 0, getWidth(), nameLabelHeight);
    nameLabel->setFont(juce::Font(nameLabelHeight, juce::Font::plain).withTypefaceStyle("Regular"));
    nameLabelTextTotalWidth = nameLabel->getFont().getStringWidth(nameLabel->getText());
    nameLabel->setSize(nameLabelTextTotalWidth, nameLabelHeight);

    volumeSliderWidth = getWidth();
    volumeSliderHeight = getHeight() / 5;
    volumeSlider->setBounds(0, getHeight() * 3 / 5, volumeSliderWidth, volumeSliderHeight);
    
    thumbnailHeight = getHeight() * 3 / 5;
    thumbnailBounds.setBounds(0, 2*nameLabelHeight, getWidth(), thumbnailHeight);

    elapsedTimeWidth = getWidth() * 3 / 5;
    elapsedTimeHeight = getHeight() / 5;
    elapsedTimeLabel->setBounds(0, elapsedTimeHeight, elapsedTimeWidth, elapsedTimeHeight);
    elapsedTimeLabel->setFont(juce::Font(nameLabelHeight, juce::Font::plain).withTypefaceStyle("Regular"));

    shortcutLabelSize = getWidth();
    shortcutLabel->setBounds(0, 0, shortcutLabelSize, shortcutLabelSize);
    shortcutLabel->setFont(juce::Font(getWidth(), juce::Font::plain).withTypefaceStyle("Regular"));
    shortcutLabel->setJustificationType(juce::Justification::centred);
}

void KeyMappedPlayer::setPlayer(Player* p)
{
    soundPlayer = p;
    soundPlayer->playerInfoChangedBroadcaster->addChangeListener(this);
    soundPlayer->remainingTimeBroadcaster->addChangeListener(this);
    thumbnail = &soundPlayer->getAudioThumbnail();
    thumbnail->addChangeListener(this);
    juce::MultiTimer::startTimer(0, 50);
    juce::MultiTimer::startTimer(1, 50);
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
        if (soundPlayer->isFileLoaded())
        {
            volumeSlider->setVisible(true);
            elapsedTimeLabel->setVisible(true);
        }
    }
    else if (source == thumbnail)
    {
        repaint();
    }
    else if (source == soundPlayer->remainingTimeBroadcaster)
    {
        setPlayerColours(juce::Colours::red);
    }
}

void KeyMappedPlayer::updatePlayerInfo()
{
    playerInfos = soundPlayer->getPlayerInfo();
    nameLabel->setText(playerInfos.name, juce::NotificationType::dontSendNotification);
    elapsedTimeLabel->setText(soundPlayer->getRemainingTimeAsString(), juce::NotificationType::dontSendNotification);
    if (soundPlayer->isPlayerPlaying())
        setPlayerColours(juce::Colours::green);
    else
        setPlayerColours(BLUE);

    resized();
    repaint();
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

void KeyMappedPlayer::timerCallback(int timerID)
{
    switch (timerID)
    {
    case 0:
        if (soundPlayer != nullptr && soundPlayer->isFileLoaded())
        {
            elapsedTimeLabel->setText(soundPlayer->getRemainingTimeAsString(), juce::NotificationType::dontSendNotification);
        }
        //repaint();
        break;
    case 1:
        scrollNameLabel();
        break;
    }
}

void KeyMappedPlayer::setPlayerColours(juce::Colour c)
{
    currentColour = c;
    elapsedTimeLabel->setColour(juce::Label::ColourIds::textColourId, currentColour);
    nameLabel->setColour(juce::Label::ColourIds::textColourId, currentColour);
    repaint();
}

void KeyMappedPlayer::scrollNameLabel()
{
    if (soundPlayer->isFileLoaded())
    {
        if (scrollLabel == false)
        {
            startTickScrollTimer++;
            if (startTickScrollTimer == 20)
            {
                scrollLabel = true;
                startTickScrollTimer = 0;
            }
        }
        else if (scrollLabel == true)
        {
            nameLabelScrollX += 2;
            if (nameLabelScrollX > (nameLabelTextTotalWidth - getWidth()))
            {
                endTickScrollTimer++;
                if (endTickScrollTimer == 20)
                {
                    scrollLabel = false;
                    nameLabelScrollX = 0;
                    nameLabel->setTopLeftPosition(nameLabelScrollX, 0);
                    endTickScrollTimer = 0;
                }
            }
            else
            {
                nameLabel->setTopLeftPosition(-nameLabelScrollX, 0);
            }

        }
    }
}