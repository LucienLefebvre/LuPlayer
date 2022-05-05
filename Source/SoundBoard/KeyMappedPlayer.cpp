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
    editButton->setAlpha(0.8);

    playHead.reset(new PlayHead());
    playHead->setMouseClickGrabsKeyboardFocus(false);

    dBLabel.reset(new juce::Label());
    addChildComponent(dBLabel.get());

    busyBar.reset(new juce::ProgressBar(busyBarValue));
    addChildComponent(busyBar.get());
    busyBar->setAlpha(0.7);
}

KeyMappedPlayer::~KeyMappedPlayer()
{
    juce::MultiTimer::stopTimer(0);
    juce::MultiTimer::stopTimer(1);
    soundPlayer = nullptr;
    playerDraggedBroadcaster->removeAllChangeListeners();
}

void KeyMappedPlayer::paint (juce::Graphics& g)
{
    if (soundPlayer != nullptr && soundPlayer->isPlayerPlaying())
    {
        g.setColour(defaultColour);
        g.setOpacity(1.0f);
    }

    //DRAW THUMBNAIL
    double transportPosition = soundPlayer->transport.getCurrentPosition();
    int playHeadPosition = (transportPosition / soundPlayer->getLenght()) * getWidth();

    thumbnailBounds.setBounds(0, 2 * nameLabelHeight, playHeadPosition, thumbnailHeight);
    juce::Array<float> gainValue;
    for (int i = 0; i < thumbnailBounds.getX() + thumbnailBounds.getWidth(); i++)
    {
        if (soundPlayer->isEnveloppeEnabled())
        {
            float time = ((i - thumbnailBounds.getX()) * soundPlayer->getLenght() / thumbnailBounds.getWidth()) / soundPlayer->getLenght();
            float value = juce::Decibels::decibelsToGain(soundPlayer->getEnveloppeValue(time, *soundPlayer->getEnveloppePath()) * 24);
            gainValue.set(i, 1.0f);
        }
        else
        {
            gainValue.set(i, 1.0f);
        }
    }
    thumbnail->setGainValues(gainValue);

    g.setColour(defaultColour.brighter(0.5f));
    g.setOpacity(1.0);
    
    double thumbnailMiddleTime = (playHeadPosition / (double)getWidth()) * soundPlayer->getLenght();

    if (thumbnail != nullptr)
        thumbnail->drawChannels(g, thumbnailBounds, 0.0, thumbnailMiddleTime, juce::Decibels::decibelsToGain(playerInfos.trimVolume) * 1.5f);

    playThumbnailBounds.setBounds(playHeadPosition, 2 * nameLabelHeight, getWidth() - playHeadPosition, thumbnailHeight);
    juce::Array<float> playGainValue;
    for (int i = 0; i < playThumbnailBounds.getX() + playThumbnailBounds.getWidth(); i++)
    {
        if (soundPlayer->isEnveloppeEnabled())
        {
            float time = ((i - playThumbnailBounds.getX()) * soundPlayer->getLenght() / playThumbnailBounds.getWidth()) / soundPlayer->getLenght();
            float value = juce::Decibels::decibelsToGain(soundPlayer->getEnveloppeValue(time, *soundPlayer->getEnveloppePath()) * 24);
            playGainValue.set(i, 1.0f);
        }
        else
        {
            playGainValue.set(i, 1.0f);
        }
    }

    playThumbnail->setGainValues(playGainValue);

    g.setColour(defaultColour);
    g.setOpacity(1.0);
    if (playThumbnail != nullptr)
        playThumbnail->drawChannels(g, playThumbnailBounds, thumbnailMiddleTime, soundPlayer->getLenght(), juce::Decibels::decibelsToGain(playerInfos.trimVolume) * 1.5f);

    g.setColour(currentColour);
    if (isDragged)
        g.setColour(juce::Colours::red);
    g.drawRoundedRectangle(getLocalBounds().toFloat(), 15, 2);
}

void KeyMappedPlayer::resized()
{
    nameLabelHeight = getHeight() / 5;
    nameLabel->setBounds(- nameLabelScrollX, 0, getWidth(), nameLabelHeight);
    nameLabel->setFont(juce::Font(nameLabelHeight * 0.75f, juce::Font::plain).withTypefaceStyle("Regular"));
    nameLabelTextTotalWidth = nameLabel->getFont().getStringWidth(nameLabel->getText());
    nameLabel->setSize(nameLabelTextTotalWidth, nameLabelHeight);

    thumbnailHeight = getHeight() * 3 / 5;

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

    dBLabel->setCentrePosition(getLocalBounds().getCentre());
    int dBLabelHeight = nameLabelHeight;
    dBLabel->setSize(getWidth(), dBLabelHeight);
    dBLabel->setFont(juce::Font(dBLabelHeight, juce::Font::plain).withTypefaceStyle("Regular"));
    dBLabel->setJustificationType(juce::Justification::centred);
    dBLabel->setColour(juce::Label::ColourIds::textColourId, ORANGE);


    busyBarBounds.setBounds(0, getHeight() / 10 * 6, getWidth(), getHeight() / 5);
    busyBar->setBounds(busyBarBounds.reduced(30, 5));
}

void KeyMappedPlayer::setPlayer(Player* p)
{
    soundPlayer = p;
    soundPlayer->playerInfoChangedBroadcaster->addChangeListener(this);
    soundPlayer->remainingTimeBroadcaster->addChangeListener(this);
    soundPlayer->soundEditedBroadcaster->addChangeListener(this);
    soundPlayer->playerDeletedBroadcaster->addChangeListener(this);
    soundPlayer->enveloppePathChangedBroadcaster->addChangeListener(this);
    soundPlayer->normalizationLaunchedBroadcaster->addChangeListener(this);
    soundPlayer->normalizationFinishedBroadcaster->addChangeListener(this);

    thumbnail = &soundPlayer->getAudioThumbnail();
    playThumbnail = &soundPlayer->getPlayThumbnail();
    thumbnail->addChangeListener(this);    
    playThumbnail->addChangeListener(this);
    juce::MultiTimer::startTimer(0, 50);
    juce::MultiTimer::startTimer(1, 50);

}

Player* KeyMappedPlayer::getPlayer()
{
    if (soundPlayer != nullptr)
        return soundPlayer;
}

void KeyMappedPlayer::setShortcut(juce::String s)
{
    shortcutKey = s;
    shortcutKeyPress = juce::KeyPress::createFromDescription(s);
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
        else if (source == playThumbnail)
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
        else if (source == soundPlayer->normalizationLaunchedBroadcaster.get())
        {
            busyBar->setTextToDisplay("Normalizing...");
            busyBar->setVisible(true);
        }
        else if (source == soundPlayer->normalizationFinishedBroadcaster.get())
        {
            busyBar->setVisible(false);
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

void KeyMappedPlayer::shortcutKeyPressed(bool commandDown)
{
    if (soundPlayer != nullptr)
    {
        if (commandDown)
        {
            soundPlayer->stop();
        }
        else if (soundPlayer->getPlayMode() == 1)
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
        if (event.mods.isCommandDown())
        {
            soundPlayer->stop();
        }
        else if (event.mods.isAltDown())
        {
            playerDraggedBroadcaster->sendChangeMessage();
        }
        else
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
}

void KeyMappedPlayer::mouseDrag(const juce::MouseEvent& event)
{
    if (soundPlayer != nullptr)
    {
        if (!event.mods.isAltDown())
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
}

void KeyMappedPlayer::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel)
{
    if (soundPlayer != nullptr)
    {
        if (e.mods.isCommandDown() && soundPlayer->isFileLoaded())
        {
            if (wheel.deltaY > 0)
                soundPlayer->setTrimVolume(soundPlayer->getTrimVolume() + 1);
            else if (wheel.deltaY < 0)
                soundPlayer->setTrimVolume(soundPlayer->getTrimVolume() - 1);
        }
        else if (soundPlayer->isFileLoaded())
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
                repaint();
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

        break;
    case 1:
        scrollNameLabel();
        break;
    }
}

void KeyMappedPlayer::setPlayerColours(juce::Colour c)
{
    if (soundPlayer != nullptr)
    {
        if (soundPlayer->isPlayerPlaying())
            currentColour = c;
        else
            currentColour = defaultColour;
        elapsedTimeLabel->setColour(juce::Label::ColourIds::textColourId, currentColour);
        nameLabel->setColour(juce::Label::ColourIds::textColourId, currentColour);
        repaint();
    }
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

juce::KeyPress KeyMappedPlayer::getShortcut()
{
    return shortcutKeyPress;
}
