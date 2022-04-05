/*
  ==============================================================================

    KeyMappedPlayer.cpp
    Created: 16 Jan 2022 9:32:26pm
    Author:  Lucien Lefebvre

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

    addMouseListener(this, true);
    
    defaultColour = BLUE;

    shortcutLabel.reset(new juce::Label());
    addAndMakeVisible(shortcutLabel.get());
    shortcutLabel->setAlpha(0.32);
    shortcutLabel->setColour(juce::Label::ColourIds::textColourId, ORANGE);
    shortcutLabel->setMouseClickGrabsKeyboardFocus(false);

    nameLabel.reset(new juce::Label());
    addAndMakeVisible(nameLabel.get());
    nameLabel->setFont(juce::Font(20.00f, juce::Font::plain).withTypefaceStyle("Regular"));
    nameLabel->setMinimumHorizontalScale(0.5);
    nameLabel->setMouseClickGrabsKeyboardFocus(false);

    elapsedTimeLabel.reset(new juce::Label());
    addChildComponent(elapsedTimeLabel.get());
    elapsedTimeLabel->setFont(juce::Font(20.00f, juce::Font::plain).withTypefaceStyle("Regular"));
    elapsedTimeLabel->setMouseClickGrabsKeyboardFocus(false);

    volumeSlider.reset(new juce::Slider());
    addChildComponent(volumeSlider.get());
    volumeSlider->setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    juce::NormalisableRange<double> range(-80, +12, 0.5, 2);
    volumeSlider->setNormalisableRange(range);
    //volumeSlider->setRange(0., 2., 0.01);
    volumeSlider->setValue(0.0);
    volumeSlider->setTextValueSuffix("dB");
    volumeSlider->addListener(this);
    volumeSlider->setTextBoxStyle(juce::Slider::NoTextBox, false, 50, 15);
    volumeSlider->setNumDecimalPlacesToDisplay(2);
    volumeSlider->setWantsKeyboardFocus(false);
    volumeSlider->setDoubleClickReturnValue(true, 0.0);
    volumeSlider->setPopupDisplayEnabled(true, true, nullptr);
    volumeSlider->setScrollWheelEnabled(true);
    volumeSlider->setMouseClickGrabsKeyboardFocus(false);

    editButton.reset(new juce::TextButton());
    addChildComponent(editButton.get());
    editButton->setButtonText("Edit");
    editButton->onClick = [this] {editButtonClicked(); };
    editButton->setMouseClickGrabsKeyboardFocus(false);

    playHead.reset(new PlayHead());
    addAndMakeVisible(playHead.get());
    playHead->setMouseClickGrabsKeyboardFocus(false);

    dBLabel.reset(new juce::Label());
    addChildComponent(dBLabel.get());
}

KeyMappedPlayer::~KeyMappedPlayer()
{
    juce::MultiTimer::stopTimer(0);
    juce::MultiTimer::stopTimer(1);
    soundPlayer = nullptr;
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

    if (soundPlayer != nullptr && soundPlayer->isPlayerPlaying())
    {
        g.setColour(defaultColour);
        g.setOpacity(0.3f);
        g.fillRoundedRectangle(getLocalBounds().reduced(1).toFloat(), 15);
    }

    //DRAW THUMBNAIL
    juce::Array<float> gainValue;
    for (int i = 0; i < thumbnailBounds.getX() + thumbnailBounds.getWidth(); i++)
    {
        if (soundPlayer->isEnveloppeEnabled())
        {
            float time = ((i - thumbnailBounds.getX()) * thumbnail->getTotalLength() / thumbnailBounds.getWidth()) / thumbnail->getTotalLength();
            float value = juce::Decibels::decibelsToGain(soundPlayer->getEnveloppeValue(time, *soundPlayer->getEnveloppePath()) * 24);
            gainValue.set(i, value);
        }
        else
        {
            gainValue.set(i, 1.0f);
        }
    }
    thumbnail->setGainValues(gainValue);

    g.setColour(currentColour);
    if (thumbnail != nullptr)
        thumbnail->drawChannels(g, thumbnailBounds, 0.0, thumbnail->getTotalLength(), juce::Decibels::decibelsToGain(playerInfos.trimVolume) * 1.5f);
}

void KeyMappedPlayer::resized()
{
    nameLabelHeight = getHeight() / 5;
    nameLabel->setBounds(- nameLabelScrollX, 0, getWidth(), nameLabelHeight);
    nameLabel->setFont(juce::Font(nameLabelHeight * 0.75f, juce::Font::plain).withTypefaceStyle("Regular"));
    nameLabelTextTotalWidth = nameLabel->getFont().getStringWidth(nameLabel->getText());
    nameLabel->setSize(nameLabelTextTotalWidth, nameLabelHeight);

    /*volumeSliderWidth = getWidth();
    volumeSliderHeight = getHeight() / 5;
    volumeSlider->setBounds(0, getHeight() * 3 / 5, volumeSliderWidth, volumeSliderHeight);
    */
    thumbnailHeight = getHeight() * 3 / 5;
    thumbnailBounds.setBounds(0, 2*nameLabelHeight, getWidth(), thumbnailHeight);

    playHead->setSize(1, thumbnailHeight);

    elapsedTimeWidth = getWidth() * 3 / 5;
    elapsedTimeHeight = getHeight() / 5;
    elapsedTimeLabel->setBounds(0, elapsedTimeHeight, elapsedTimeWidth, elapsedTimeHeight);
    elapsedTimeLabel->setFont(juce::Font(nameLabelHeight, juce::Font::plain).withTypefaceStyle("Regular"));

    shortcutLabelWidth = getWidth();
    shortcutLabelHeight = getHeight();
    shortcutLabel->setSize(shortcutLabelWidth, shortcutLabelHeight);
    shortcutLabel->setCentrePosition(getWidth() / 2, getHeight() / 2);
    shortcutLabel->setFont(juce::Font(getWidth(), juce::Font::plain).withTypefaceStyle("Regular"));
    shortcutLabel->setJustificationType(juce::Justification::centred);

    editButtonWidth = 3 * getWidth() / 10;
    editButton->setBounds(elapsedTimeWidth, elapsedTimeHeight, editButtonWidth, elapsedTimeHeight);

    dBLabel->setCentrePosition(thumbnailBounds.getCentre());
    int dBLabelHeight = nameLabelHeight;
    dBLabel->setSize(getWidth(), dBLabelHeight);
    dBLabel->setFont(juce::Font(dBLabelHeight, juce::Font::plain).withTypefaceStyle("Regular"));
    dBLabel->setJustificationType(juce::Justification::centred);
    dBLabel->setColour(juce::Label::ColourIds::textColourId, ORANGE);
}

void KeyMappedPlayer::setPlayer(Player* p)
{
    soundPlayer = p;
    soundPlayer->playerInfoChangedBroadcaster->addChangeListener(this);
    soundPlayer->remainingTimeBroadcaster->addChangeListener(this);
    soundPlayer->soundEditedBroadcaster->addChangeListener(this);
    soundPlayer->playerDeletedBroadcaster->addChangeListener(this);
    soundPlayer->enveloppePathChangedBroadcaster->addChangeListener(this);
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
    if (soundPlayer != nullptr)
    {
        if (source == soundPlayer->playerInfoChangedBroadcaster)
        {
            updatePlayerInfo();
            if (soundPlayer->isFileLoaded())
            {
                volumeSlider->setVisible(true);
                elapsedTimeLabel->setVisible(true);
                editButton->setVisible(true);
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
        else if (source == soundPlayer->soundEditedBroadcaster)
        {
            updatePlayerInfo();
        }
        else if (source == soundPlayer->playerDeletedBroadcaster)
        {
            /*soundPlayer = nullptr;
            thumbnail = nullptr;*/
            volumeSlider->setVisible(false);
            elapsedTimeLabel->setVisible(false);
            editButton->setVisible(false);
            currentColour = BLUE;
            repaint();
        }
        else if (source == soundPlayer->enveloppePathChangedBroadcaster.get())
        {
            repaint();
        }
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
        setPlayerColours(defaultColour);

    if (soundPlayer->isEditedPlayer())
        editButton->setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::red);
    else
        editButton->setColour(juce::TextButton::ColourIds::buttonColourId,
            getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    if (soundPlayer->getColourHasChanged())
        defaultColour = soundPlayer->getPlayerColour();
    else
        defaultColour = BLUE;

    resized();
    repaint();
}

void KeyMappedPlayer::shortcutKeyPressed()
{
    if (soundPlayer != nullptr)
    {
        if (soundPlayer->getPlayMode() == 1)
        {
            soundPlayer->stop();
            soundPlayer->launch();
        }
        else if (soundPlayer->getPlayMode() == 2)
        {
            startOrStop();
        }
    }
}

void KeyMappedPlayer::startOrStop()
{
    if (soundPlayer != nullptr)
    {
        if (!soundPlayer->isPlayerPlaying())
            soundPlayer->launch();
        else
            soundPlayer->stop();
    }
}

void KeyMappedPlayer::mouseDown(const juce::MouseEvent& event)
{
    dBLabel->setVisible(true);

    if (soundPlayer != nullptr)
    {
        if (event.getNumberOfClicks() == 2)
        {
            soundPlayer->setGain(juce::Decibels::decibelsToGain(0));
        }
        if (soundPlayer->isFileLoaded())
            setDBText();
        gainAtDragStart = juce::Decibels::gainToDecibels(soundPlayer->getVolume());
        gainTimeStartDisplay = juce::Time::getMillisecondCounter();
    }
}

void KeyMappedPlayer::mouseDrag(const juce::MouseEvent& event)
{
    if (soundPlayer != nullptr)
    {
        if (soundPlayer->isFileLoaded())
        {
            float gain = gainAtDragStart - event.getDistanceFromDragStartY() / 10 + event.getDistanceFromDragStartX() / 10;
            soundPlayer->setGain(juce::Decibels::decibelsToGain(gain));
            setDBText();
            gainTimeStartDisplay = juce::Time::getMillisecondCounter();
        }
    }
}

void KeyMappedPlayer::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel)
{
    if (soundPlayer != nullptr)
    {
        if (soundPlayer->isFileLoaded())
        {
            float playerGain = juce::Decibels::gainToDecibels(soundPlayer->getVolume());
            float gain = 0.0f;
            if (wheel.deltaY > 0)
                gain = 0.5;
            else if (wheel.deltaY < 0)
                gain = -0.5f;
            soundPlayer->setGain(juce::Decibels::decibelsToGain(playerGain + gain));
            dBLabel->setVisible(true);
            setDBText();
            gainTimeStartDisplay = juce::Time::getMillisecondCounter();
        }
    }
}


void KeyMappedPlayer::setDBText()
{
    dBLabel->setText(juce::String(round(juce::Decibels::gainToDecibels(soundPlayer->getVolume()))) + "dB", juce::dontSendNotification);
}
void KeyMappedPlayer::mouseUp(const juce::MouseEvent& event)
{
    //dBLabel->setVisible(false);
}

void KeyMappedPlayer::sliderValueChanged(juce::Slider* slider)
{
    if (slider == volumeSlider.get())
    {
        if (soundPlayer != nullptr)
        {
            soundPlayer->setGain(juce::Decibels::decibelsToGain(volumeSlider->getValue()));
        }
    }
}

void KeyMappedPlayer::timerCallback(int timerID)
{
    switch (timerID)
    {
    case 0:
        if (soundPlayer != nullptr)
        {
            if (soundPlayer->isFileLoaded())
            {
                elapsedTimeLabel->setText(soundPlayer->getRemainingTimeAsString(), juce::NotificationType::dontSendNotification);
            }
            if (soundPlayer->isPlayerPlaying())
            {
                playHead->setVisible(true);
                int playHeadPosition = (soundPlayer->transport.getCurrentPosition() / soundPlayer->getLenght()) * getWidth();
                playHead->setTopLeftPosition(playHeadPosition, thumbnailBounds.getY());
                nameLabel->setColour(juce::Label::ColourIds::textColourId, currentColour); 
                elapsedTimeLabel->setColour(juce::Label::ColourIds::textColourId, currentColour); 
            }
            else
            {
                playHead->setVisible(false);
            }

            if (dBLabel->isVisible())
            {
                if (juce::Time::getMillisecondCounter() - gainTimeStartDisplay > gainDisplayTimeMs)
                    dBLabel->setVisible(false);
            }
        }
        else
            playHead->setVisible(false);

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
    elapsedTimeLabel->setColour(juce::Label::ColourIds::textColourId, juce::Colours::white);
    nameLabel->setColour(juce::Label::ColourIds::textColourId, currentColour);
    repaint();
}

void KeyMappedPlayer::scrollNameLabel()
{
    if (soundPlayer != nullptr)
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
}

void KeyMappedPlayer::editButtonClicked()
{
    if (soundPlayer != nullptr)
        soundPlayer->envButtonClicked();
}

void KeyMappedPlayer::setPlayerDefaultColour(juce::Colour c)
{
    defaultColour = c;
}

juce::Colour KeyMappedPlayer::getPlayerDefaultColour()
{
    return defaultColour;
}