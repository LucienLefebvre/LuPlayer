/*
  ==============================================================================

    Player.cpp
    Created: 26 Jan 2021 7:36:59pm
    Author:  Lucien

  ==============================================================================
*/

#include <JuceHeader.h>
#include "Player.h"
#include "Windows.h"

#include "Settings.h"
//#include <string>

//==============================================================================
Player::Player(int index): openButton("Open"), playButton("Play"), stopButton("Stop"), cueButton("6s"), deleteButton("Delete"), thumbnailCache(5), thumbnail(521, formatManager, thumbnailCache), resampledSource(&transport, false, 2)
{
   //std::unique_ptr<Settings> settings = std::make_unique<Settings>();

    playerIndex = index;
    juce::Timer::startTimer(100);
    state = Stopped;

    int playerPosition = playerIndex + 1;
    addAndMakeVisible(playerPositionLabel);
    playerPositionLabel.setButtonText(juce::String(playerPosition));
    playerPositionLabel.setBounds(borderRectangleWidth + 5, 5, 35, 35);
    playerPositionLabel.onClick = [this]{ assignNextPlayer(); };


    addAndMakeVisible(&openButton);
    openButton.onClick = [this] { openButtonClicked(); };
    openButton.setBounds(rightControlsStart, 80, openDeleteButtonWidth, 20);
    openButton.setWantsKeyboardFocus(false);


    addAndMakeVisible(&deleteButton);
    deleteButton.onClick = [this] { deleteFile(); };
    deleteButton.setBounds(rightControlsStart + openDeleteButtonWidth, 80, openDeleteButtonWidth, 20);
    deleteButton.setWantsKeyboardFocus(false);

    addAndMakeVisible(&playButton);
    playButton.onClick = [this] { playButtonClicked(); };
    playButton.setBounds(rightControlsStart, 0, playStopButtonWidth, 39);
    playButton.setEnabled(false);
    playButton.setWantsKeyboardFocus(false);


    addAndMakeVisible(&stopButton);
    stopButton.onClick = [this] { stopButtonClicked(); };
    stopButton.setBounds(rightControlsStart + playStopButtonWidth, 0, playStopButtonWidth, 39);
    stopButton.setEnabled(false);
    stopButton.setWantsKeyboardFocus(false);


    addAndMakeVisible(&cueButton);
    cueButton.onClick = [this] { cueButtonClicked(); };
    cueButton.setBounds(rightControlsStart + 2*playStopButtonWidth, 0, playStopButtonWidth, 39);
    cueButton.setEnabled(false);
    cueButton.setWantsKeyboardFocus(false);


    addAndMakeVisible(&loopButton);
    loopButton.setBounds(380, 80, 70, 20);
    loopButton.setButtonText("loop");
    loopButton.setEnabled(true);
    loopButton.onClick = [this]{ updateLoopButton(&loopButton, "loop");; };
    loopButton.setToggleState(false, true);
    loopButton.setWantsKeyboardFocus(false);

    addAndMakeVisible(volumeSlider);
    volumeSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);    
    volumeSlider.setBounds(rightControlsStart + rightControlsWidth, -5, volumeSliderWidth, 100);
    volumeSlider.setRange(0., 1., 0.01);
    volumeSlider.addListener(this);
    volumeSlider.setValue(0.);
    formatManager.registerBasicFormats();
    volumeSlider.setSkewFactor(0.5, false);
    volumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 50, 15);
    volumeSlider.setNumDecimalPlacesToDisplay(2);
    volumeSlider.setWantsKeyboardFocus(false);
    volumeSlider.setDoubleClickReturnValue(true, 1.);


    addAndMakeVisible(volumeLabel);
    volumeLabel.setFont(juce::Font(10.00f, juce::Font::plain).withTypefaceStyle("Regular"));
    volumeLabel.setJustificationType(juce::Justification::centred);
    volumeLabel.setEditable(false, false, false);
    volumeLabel.setBounds(rightControlsStart + rightControlsWidth, 90, volumeSliderWidth, 10);
    volumeLabel.setText(juce::String(juce::Decibels::gainToDecibels(volumeSlider.getValue())), juce::NotificationType::dontSendNotification);
    volumeLabel.setWantsKeyboardFocus(false);


    addAndMakeVisible(trimVolumeSlider);
    trimVolumeSlider.setRange(-24, 24, 0.5);
    trimVolumeSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    trimVolumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 50, 10);
    trimVolumeSlider.addListener(this);
    if (isCart)
    trimVolumeSlider.setBounds(0 -8, 40, 64, 56);
    else
        trimVolumeSlider.setBounds(borderRectangleWidth - 8, 40, 64, 56);
    trimVolumeSlider.setDoubleClickReturnValue(true, 0.);
    trimVolumeSlider.setPopupDisplayEnabled(true, true, this, 2000);
    trimVolumeSlider.setWantsKeyboardFocus(false);

 
    addAndMakeVisible(trimLabel);
    trimLabel.setFont(juce::Font(15.00f, juce::Font::plain).withTypefaceStyle("Regular"));
    trimLabel.setJustificationType(juce::Justification::centred);
    trimLabel.setEditable(false, false, false);
    trimLabel.setText(TRANS("Trim"), juce::NotificationType::dontSendNotification);
    trimLabel.setBounds(borderRectangleWidth, 81, 50, 24);
    trimLabel.setWantsKeyboardFocus(false);



    addAndMakeVisible(remainingTimeLabel);
    remainingTimeLabel.setBounds(450, 40, 150, 39);
    remainingTimeLabel.setText(TRANS("0"), juce::NotificationType::dontSendNotification);
    remainingTimeLabel.setJustificationType(juce::Justification::centred);
    remainingTimeLabel.setFont(juce::Font(35.00f, juce::Font::plain).withTypefaceStyle("Regular"));
    remainingTimeLabel.setWantsKeyboardFocus(false);


    addAndMakeVisible(soundName);
    soundName.setBounds(leftControlsWidth + borderRectangleWidth, 80, 330, 20);
    soundName.setJustificationType(juce::Justification::centred);
    soundName.setFont(juce::Font(19.0f, juce::Font::bold).withTypefaceStyle("Regular"));
    soundName.setText(juce::String(""), juce::NotificationType::dontSendNotification);
    soundName.setWantsKeyboardFocus(false);
    soundName.setEditable(false, true, false);

    //TODO régler bug accents dans string

    addAndMakeVisible(playerIDLabel);
    //playerIDLabel.setBounds(400, 80, 50, 20);
    playerIDLabel.setText(juce::String(playerIndex), juce::NotificationType::dontSendNotification);
        
    

    thumbnail.addChangeListener(this);
    thumbnailBounds.setBounds(leftControlsWidth + borderRectangleWidth, 0, waveformThumbnailXSize, 80);

    if (isCart == true)
    {
        waveformThumbnailXSize = getParentWidth() - leftControlsWidth - rightControlsWidth - borderRectangleWidth * 2 - volumeSliderWidth - 2 * cartButtonsControlWidth - 20 - 20;
        waveformThumbnailXEnd = waveformThumbnailXStart + waveformThumbnailXSize;
    }
    else if (isCart == false)
    {
        waveformThumbnailXSize = getParentWidth() - leftControlsWidth - rightControlsWidth - borderRectangleWidth * 2 - volumeSliderWidth - playlistButtonsControlWidth - 20;
        waveformThumbnailXEnd = waveformThumbnailXStart + waveformThumbnailXSize;
    }
    thumbnailBounds.setBounds(leftControlsWidth + borderRectangleWidth, 0, waveformThumbnailXSize, 80);
    rightControlsStart = leftControlsWidth + borderRectangleWidth + waveformThumbnailXSize;
    volumeLabelStart = rightControlsStart + rightControlsWidth;
    playButton.setBounds(rightControlsStart, 0, playStopButtonWidth, 39);
    openButton.setBounds(rightControlsStart, 80, openDeleteButtonWidth, 20);
    stopButton.setBounds(rightControlsStart + playStopButtonWidth, 0, playStopButtonWidth, 39);
    cueButton.setBounds(rightControlsStart + 2 * playStopButtonWidth, 0, playStopButtonWidth, 39);
    deleteButton.setBounds(rightControlsStart + openDeleteButtonWidth, 80, openDeleteButtonWidth, 20);
    remainingTimeLabel.setBounds(rightControlsStart, 40, 150, 39);
    volumeSlider.setBounds(volumeLabelStart, -5, volumeSliderWidth, 100);
    volumeLabel.setBounds(volumeLabelStart, 90, volumeSliderWidth, 10);
    soundName.setBounds(leftControlsWidth + borderRectangleWidth, 80, waveformThumbnailXSize - 50, 20);
    loopButton.setBounds(leftControlsWidth + borderRectangleWidth + waveformThumbnailXSize - 50, 80, 50, 20);

    /*auto preloadedFile = juce::File("D:\Sons\paris 22k.wav");
    loadFile(preloadedFile);*/

    //DBG(totalPlayerWidth);
    repaint();
    }

Player::~Player()
{
    //delete Player;
    //deleteAllChildren();
}

void Player::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */
    if (isCart == true)
        borderRectangleWidth = 0;
    g.setOpacity(1.0);
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
    if (state == Playing)
    {
        if ((transport.getLengthInSeconds() - transport.getCurrentPosition() < 6))
            g.setColour(juce::Colours::red);
        else
            g.setColour(juce::Colours::green);

    }
    else
    {
        if (isNextPlayer == true)
        {
            g.setColour(juce::Colours::orange);
            soundName.setColour(soundName.textColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        }
        else
        {
            g.setColour(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
            soundName.setColour(soundName.textColourId, juce::Colours::white);
        }
    }
    if (!fileLoaded)
        g.setColour(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.fillAll();   // clear the background
    float x = 0.0f, y = 0.0f, width = 650.0f, height = 100.0f;
    juce::Colour fillColour = juce::Colour(0x23000000);
    juce::Colour strokeColour = juce::Colours::black;

    /*g.setColour(fillColour);
    g.fillRoundedRectangle(x, y, width, height, 2.000f);
    g.setColour(strokeColour);
    g.drawRoundedRectangle(x, y, width, height, 2.000f, 1.200f);*/

    //Waveform
    if (thumbnail.getNumChannels() == 0)
        paintIfNoFileLoaded(g, thumbnailBounds);
    else
    {
        if (waveformPainted == false)
        {
            paintIfFileLoaded(g, thumbnailBounds, thumbnailZoomValue);
            //waveformPainted = true;
        }
        paintPlayHead(g, thumbnailBounds);
    }

    if (isCart == false)
    {
    {//rectangle Fader1
        int x = 0, y = 0, width = borderRectangleWidth, height = borderRectangleHeight;
        if ((fader1IsPlaying == false && isNextPlayer == true)
            || (faderLeftAssigned == true))
        {
            g.setColour(juce::Colour(255, 163, 0));
            g.fillRect(x, y, width, height);
        }
        else if (fader1IsPlaying == true)
        {
            g.setColour(juce::Colours::green);
            g.fillRect(x, y, width, height);
        }
        if (((transport.getLengthInSeconds() - transport.getCurrentPosition() < 6)) && fileLoaded == true && state == Playing)
        {
            g.setColour(juce::Colours::red);
            g.fillRect(x, y, width, height);
        }
    }

        {//rectangle Fader2
            int x = getParentWidth() - borderRectangleWidth - playlistButtonsControlWidth - 20, y = 0, width = borderRectangleWidth, height = borderRectangleHeight;
            if ((fader2IsPlaying == false && isNextPlayer == true)
                || (faderRightAssigned == true))
            {
                g.setColour(juce::Colour(255, 163, 0));
                g.fillRect(x, y, width, height);
            }
            else if (fader2IsPlaying == true)
            {
                g.setColour(juce::Colours::green);
                g.fillRect(x, y, width, height);
            }
            if (((transport.getLengthInSeconds() - transport.getCurrentPosition() < 6)) && fileLoaded == true && state == Playing)
            {
                g.setColour(juce::Colours::red);
                g.fillRect(x, y, width, height);
            }

        }
    }
    //else if (isCart == true)
    //{
    //    {//rectangle Fader2

    //        int x = volumeLabelStart + 100, y = 0, width = borderRectangleWidth, height = borderRectangleHeight;
    //        if ((fader2IsPlaying == false && isNextPlayer == true)
    //            || (faderRightAssigned == true))
    //        {
    //            g.setColour(juce::Colour(255, 163, 0));
    //            g.fillRect(x, y, width, height);
    //        }
    //        else if (fader2IsPlaying == true)
    //        {
    //            g.setColour(juce::Colours::green);
    //            g.fillRect(x, y, width, height);
    //        }
    //        if (((transport.getLengthInSeconds() - transport.getCurrentPosition() < 6)) && fileLoaded == true && state == Playing)
    //        {
    //            g.setColour(juce::Colours::red);
    //            g.fillRect(x, y, width, height);
    //        }

    //    }
    //}
}

//WAVEFORM DRAWING
void Player::thumbnailChanged()
{
    repaint();
}

void Player::paintIfNoFileLoaded(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds)
{
    //if (isNextPlayer)
    //    g.setColour(juce::Colours::orange);
    //else
        g.setColour(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.fillRect(thumbnailBounds);
    g.setColour(juce::Colours::white);
    g.drawFittedText("No File Loaded", thumbnailBounds, juce::Justification::centred, 1);
}

void Player::paintIfFileLoaded(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds, float thumbnailZoomValue)
{
    // waveform background
    


    //waveform green if playing, red if remaining time <5s, orange if next player
    if (state == Playing)
    {
        if ((transport.getLengthInSeconds() - transport.getCurrentPosition() < 6))
            g.setColour(juce::Colours::red);
        else
            g.setColour(juce::Colours::green);

    }
    else
        if (isNextPlayer == true) g.setColour(juce::Colour(255, 163, 0));
        else g.setColour(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.fillRect(thumbnailBounds);

    if (isNextPlayer || state == Playing)
        g.setColour(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    else
        g.setColour(juce::Colour(255, 170, 0));
    thumbnail.drawChannels(g,
        thumbnailBounds,
        0.0,                                    // start time
        thumbnail.getTotalLength(),             // end time
        thumbnailZoomValue);                                  // vertical zoom


}

void Player::paintPlayHead(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds)
{

    g.setColour(juce::Colours::black);

    auto audioPosition = (float)transport.getCurrentPosition();
    auto currentPosition = transport.getLengthInSeconds();

    auto drawPosition = ((audioPosition / currentPosition) * (float)thumbnailBounds.getWidth())
        + (float)thumbnailBounds.getX();                                // [13]


    g.drawLine(drawPosition, (float)thumbnailBounds.getY(), drawPosition,
        (float)thumbnailBounds.getBottom(), 2.0f);



}

void Player::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
    if (isCart == true)
    {
        waveformThumbnailXSize = getParentWidth() - leftControlsWidth - rightControlsWidth - volumeSliderWidth - 2*cartButtonsControlWidth - 20;
        waveformThumbnailXEnd = waveformThumbnailXStart + waveformThumbnailXSize;
        trimVolumeSlider.setBounds(0 - 8, 40, 64, 56);
        playerPositionLabel.setBounds(0 + 5, 5, 35, 35);
        borderRectangleWidth = 0;
    }
    else if (isCart == false)
    {
        waveformThumbnailXSize = getParentWidth() - leftControlsWidth - rightControlsWidth - borderRectangleWidth * 2 - volumeSliderWidth - playlistButtonsControlWidth - 20;
        waveformThumbnailXEnd = waveformThumbnailXStart + waveformThumbnailXSize;
    }
    thumbnailBounds.setBounds(leftControlsWidth + borderRectangleWidth, 0, waveformThumbnailXSize, 80);
    rightControlsStart = leftControlsWidth + borderRectangleWidth + waveformThumbnailXSize;
    volumeLabelStart = rightControlsStart + rightControlsWidth;
    playButton.setBounds(rightControlsStart, 0, playStopButtonWidth, 39);
    openButton.setBounds(rightControlsStart, 80, openDeleteButtonWidth, 20);
    stopButton.setBounds(rightControlsStart + playStopButtonWidth, 0, playStopButtonWidth, 39);
    cueButton.setBounds(rightControlsStart + 2 * playStopButtonWidth, 0, playStopButtonWidth, 39);
    deleteButton.setBounds(rightControlsStart + openDeleteButtonWidth, 80, openDeleteButtonWidth, 20);
    remainingTimeLabel.setBounds(rightControlsStart, 40, 150, 39);
    volumeSlider.setBounds(volumeLabelStart, -5, volumeSliderWidth, 100);
    volumeLabel.setBounds(volumeLabelStart, 90, volumeSliderWidth, 10);
    soundName.setBounds(leftControlsWidth + borderRectangleWidth, 80, waveformThumbnailXSize -50, 20);
    loopButton.setBounds(leftControlsWidth + borderRectangleWidth + waveformThumbnailXSize - 50, 80, 50, 20);
    

    //setSize(volumeLabelStart + 50 + 30, 100);
    repaint();
    //DBG(rightControlsStart);
    //DBG(volumeLabelStart);

}


void Player::setLabelPlayerPosition(int p)
{
    playerPosition = p + 1;
    playerPositionLabel.setButtonText(juce::String(p + 1));

}

//TIMER
void Player::timerCallback()
{
    updateRemainingTime();
    repaint();
    //paintPlayHead(g, thumbnailBounds);
}

void Player::updateRemainingTime()
{
    int remainingSeconds = juce::int16(trunc(transport.getLengthInSeconds() - transport.getCurrentPosition()));
    int remainingTimeSeconds = remainingSeconds % 60;
    int remainingTimeMinuts = trunc(remainingSeconds / 60);
    if (remainingTimeSeconds < 10)
        remainingTimeString = juce::String(remainingTimeMinuts) + ":0" + juce::String(remainingTimeSeconds);
    else
        remainingTimeString = juce::String(remainingTimeMinuts) + ":" + juce::String(remainingTimeSeconds);
    remainingTimeLabel.setText(remainingTimeString, juce::NotificationType::dontSendNotification);
}

//AUDIO OUTPUT
void Player::playerPrepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    resampledSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    actualSamplesPerBlockExpected = samplesPerBlockExpected;
    actualSampleRate = sampleRate;

    
}

void Player::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();
    resampledSource.getNextAudioBlock(bufferToFill);
    //meterSource.measureBlock(*bufferToFill.buffer);
}


//Play Control
void Player::play()
{
    const juce::MessageManagerLock mmLock;
    transportStateChanged(Starting);

}

void Player::stop()
{
    const juce::MessageManagerLock mmLock;
    transportStateChanged(Stopping);
}

//BUTTONS
void Player::openButtonClicked()
{
    //create a chooser to open file
    juce::FileChooser chooser("Choose an audio File", juce::File::getSpecialLocation(juce::File::userDesktopDirectory), "*.wav; *.mp3; *.bwf; *.aiff; *.opus; *.flac"); 

    if (chooser.browseForFileToOpen())
    {
        juce::File myFile;
        myFile = chooser.getResult();
        
        auto filePath = myFile.getFullPathName();
        verifyAudioFileFormat(filePath);
    }
}

void Player::deleteFile()
{
    transport.releaseResources();
    thumbnail.setSource(nullptr);
    transport.setSource(nullptr);
    playButton.setEnabled(false);
    stopButton.setEnabled(false);
    cueButton.setEnabled(false);
    loadedFilePath = "";
    newName = "";
    trimVolumeSlider.setValue(juce::Decibels::decibelsToGain(0.0));
    soundName.setText("", juce::NotificationType::dontSendNotification);
    fileName.setValue("");
    fileLoaded = false;
}


void Player::playButtonClicked()
{
    //const juce::MessageManagerLock mmLock;
    volumeSlider.setValue(1.0);
    play();
}


void Player::stopButtonClicked()
{
    const juce::MessageManagerLock mmLock;
    if (looping == false)
    {
        volumeSlider.setValue(0.0);
        transport.setPosition(0.0);
        stop();
        stopButtonClickedBool = false;
    }
    else if (looping == true)
    {
        volumeSlider.setValue(0.0);
        transport.setPosition(0.0);
        stopButtonClickedBool = true;
        transportStateChanged(Stopping);
        stopButtonClickedBool = false;
    }

}

void Player::cueButtonClicked()
{
    if (transport.getLengthInSeconds() >= 6)
        transport.setPosition(transport.getLengthInSeconds() - 6);
    if (state != Playing)
        transportStateChanged(Starting);
    volumeSlider.setValue(1.0);
}

void Player::updateLoopButton(juce::Button* button, juce::String name)
{
    auto state = button->getToggleState();
    looping = state;
    //DBG(int(state));
    //transport.isLooping();
    transport.setLooping(looping);
    
}

void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{

}

//LISTENERS
void Player::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &transport)
    {
        if (transport.isPlaying())
        {
            transportStateChanged(Playing);
        }
        else
        {
            transportStateChanged(Stopping);

        }
    }

    if (source == &thumbnail)
    {
        thumbnailChanged();
    }
}

void Player::transportStateChanged(TransportState newState)
{

    if (newState != state)
    {
        state = newState;


        switch (state)
        {
        case Stopped:
                transport.setPosition(0.0);
                playButton.setEnabled(true);
                break;
        case Starting:
            playButton.setEnabled(false);
            transport.start();
            break;
        case Stopping:
            if (looping == true && stopButtonClickedBool == false)
            {
                playButton.setEnabled(true);
                transport.setPosition(0.0);
                //transport.stop();
                transport.start();
            }
            else if ((looping == true && stopButtonClickedBool == true) || looping == false)
            {
                playButton.setEnabled(true);
                transport.setPosition(0.0);
                transport.stop();
                stopButtonClickedBool == false;
            }
                break;       
        }

    }
}

void Player::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &transportPositionSlider)
    {
        if (transportPositionSlider.isMouseButtonDown() == 1)
        {
            transport.setPosition(transportPositionSlider.getValue());
        }
    }
    if (slider == &volumeSlider)
    {
        actualSliderValue = volumeSlider.getValue();
        transport.setGain(juce::Decibels::decibelsToGain(trimVolumeSlider.getValue()) * volumeSlider.getValue());
        volumeLabel.setText(juce::String(round(juce::Decibels::gainToDecibels(volumeSlider.getValue()))), juce::NotificationType::dontSendNotification);
        if (previousSliderValue != actualSliderValue)
            volumeSliderValue = volumeSlider.getValue();
        previousSliderValue = actualSliderValue;
    }
    if (slider == &trimVolumeSlider)
    {
        transport.setGain(juce::Decibels::decibelsToGain(trimVolumeSlider.getValue()) * volumeSlider.getValue());
        thumbnailZoomValue = juce::Decibels::decibelsToGain(trimVolumeSlider.getValue());
        if (thumbnail.getNumChannels() != 0)
        {
            //repaint();
        }
    }

}



//WAVEFORM CONTROL
void Player::mouseDrag(const juce::MouseEvent &event)
{
    if (thumbnail.getNumChannels() != 0)
    {
        if (!fader1IsPlaying && !fader2IsPlaying && !keyIsPlaying)
        {
            mouseDragXPosition = getMouseXYRelative().getX();
            if (mouseDragXPosition >= waveformThumbnailXStart && mouseDragXPosition <= waveformThumbnailXEnd)
            {
                //get mouse drag position inside the waveform
                mouseDragRelativeXPosition = getMouseXYRelative().getX() - waveformThumbnailXStart;
                //rule of 3
                mouseDragInSeconds = ((float)mouseDragRelativeXPosition * transport.getLengthInSeconds()) / (float)waveformThumbnailXSize;
                transport.setPosition(mouseDragInSeconds);
                //repaint();
            }
        }
    }
}

void Player::mouseDown(const juce::MouseEvent& event)
{
    if (thumbnail.getNumChannels() != 0)
    {
        if (!fader1IsPlaying && !fader2IsPlaying && !keyIsPlaying)
        {
            mouseDragXPosition = getMouseXYRelative().getX();
            if (mouseDragXPosition >= waveformThumbnailXStart && mouseDragXPosition <= waveformThumbnailXEnd)
            {
                //get mouse drag position inside the waveform
                mouseDragRelativeXPosition = getMouseXYRelative().getX() - waveformThumbnailXStart;
                //rule of 3
                mouseDragInSeconds = ((float)mouseDragRelativeXPosition * transport.getLengthInSeconds()) / (float)waveformThumbnailXSize;
                transport.setPosition(mouseDragInSeconds);
                //repaint();
            }
        }
    }
}


//MIDI CONTROL
void Player::handleMidiMessage(int midiMessageNumber, int midiMessageValue)
{
    const juce::MessageManagerLock mmLock;

        floatMidiMessageValue = midiMessageValue / 127.;

        juce::NormalisableRange<float>rangedMidiMessage(0.0, juce::Decibels::decibelsToGain(Settings::maxFaderValueGlobal), 0.001, Settings::skewFactorGlobal, false);
        DBG(Settings::maxFaderValueGlobal);
        
        volumeSlider.setValue(rangedMidiMessage.convertFrom0to1(floatMidiMessageValue));
}

void Player::setNextPlayer(bool trueOrFalse)
{
    isNextPlayer = trueOrFalse;
}

void Player::setLeftFaderAssigned(bool isFaderLeftAssigned)
{
    faderLeftAssigned = isFaderLeftAssigned;
    repaint();
}



void Player::setRightFaderAssigned(bool isFaderRightAssigned)
{
    faderRightAssigned = isFaderRightAssigned;
    repaint();
}


void Player::verifyAudioFileFormat(const juce::String& path)
{
    juce::File file(path);
    if ((file.getFileExtension() == juce::String(".wav")) || (file.getFileExtension() == juce::String(".WAV"))
        || (file.getFileExtension() == juce::String(".mp3")) || (file.getFileExtension() == juce::String(".MP3"))
        || (file.getFileExtension() == juce::String(".flac")) || (file.getFileExtension() == juce::String(".FLAC"))
        || (file.getFileExtension() == juce::String(".aif")) || (file.getFileExtension() == juce::String(".AIF"))
        || (file.getFileExtension() == juce::String(".aiff")) || (file.getFileExtension() == juce::String(".AIFF")))
    {
        juce::File file(path);
        if (juce::AudioFormatReader* reader = formatManager.createReaderFor(file))
        {
            loadFile(file.getFullPathName());
            delete reader;
        }
        else
        {
            std::string pathstd = path.toStdString();
            juce::String newpath = startFFmpeg(pathstd);
            std::string newpathstd = newpath.toStdString();
            loadFile(newpathstd);
        }
    }
    else
    {
        juce::String correctedPath;
        correctedPath = startFFmpeg(path.toStdString());
        if (loadFile(correctedPath) == true)
        {
            extactName(correctedPath.toStdString());
        }
        else
        {
            std::string pathStd = path.toStdString();
            std::string filename = "D:\\SONS\\TECHNIQUE\\cf68d5d4-98c1-4502-adcc-f137d8cb4c62.BWF";
            std::string newfilename = std::regex_replace(pathStd, std::regex(R"(\\)"), R"(\\)");
            std::string correctedString = newfilename;
            const size_t last_slash_idx = correctedString.find_last_of("\\");
            if (std::string::npos != last_slash_idx)
            {
                correctedString.erase(0, last_slash_idx + 1);
            }
            std::string newPath = std::string("D:\\SONS\\ADMIN\\" + correctedString);

            
            correctedPath = (startFFmpeg(newPath));
            loadFile(correctedPath);
            extactName(correctedPath.toStdString());
        }
    }
}


bool Player::loadFile(const juce::String& path)
{
    loadedFilePath = path.toStdString();
    juce::File file(path);

    if (juce::AudioFormatReader* reader = formatManager.createReaderFor(file))
    {
        std::unique_ptr<juce::AudioFormatReaderSource> tempSource(new juce::AudioFormatReaderSource(reader, true));

        fileName = file.getFileName();
        soundName.setText(fileName.toString(), juce::NotificationType::dontSendNotification);

        transport.setSource(tempSource.get());
        transport.addChangeListener(this);
        transport.setGain(0.0);

        resampledSource.setResamplingRatio(reader->sampleRate / actualSampleRate);


        //replace temporary source by playing source
        playSource.reset(tempSource.release());

        playButton.setEnabled(true);
        cueButton.setEnabled(true);
        stopButton.setEnabled(true);
        trimVolumeSlider.setValue(0, juce::NotificationType::dontSendNotification);
        thumbnailZoomValue = 1.;

        transportPositionSlider.setEnabled(true);
        transportPositionSlider.setNumDecimalPlacesToDisplay(0);
        transportPositionSlider.setRange(0, transport.getLengthInSeconds());

        thumbnail.setSource(new juce::FileInputSource(file));
        waveformPainted = false;
        fileLoaded = true;
        return true;
    }
    else
    {
        DBG("fichier non chargé !");
        return false;
    }
}

const juce::String Player::startFFmpeg(std::string filePath)
{
    std::string USES_CONVERSION_EX;

    std::string ffmpegpath = juce::String(juce::File::getCurrentWorkingDirectory().getFullPathName() + "\\ffmpeg.exe").toStdString();
    //FFmpegPath = Settings::FFmpegPath.toStdString();
    convertedFilesPath = Settings::convertedSoundsPath.toStdString();
    //////////*****************Create FFMPEG command Line
    //add double slash to path
    std::string newFilePath = std::regex_replace(filePath, std::regex(R"(\\)"), R"(\\)");
    std::string newFFmpegPath = std::regex_replace(ffmpegpath, std::regex(R"(\\)"), R"(\\)");
    std::string newConvertedFilesPath = std::regex_replace(convertedFilesPath, std::regex(R"(\\)"), R"(\\)");
    //give Output Directory
    std::size_t botDirPos = filePath.find_last_of("\\");
    std::string outputFileDirectory = filePath.substr(0, botDirPos);
    //give file name with extension
    std::string fileOutputName = filePath.substr(botDirPos, filePath.length());
    std::string newFileOutputDir = std::regex_replace(outputFileDirectory, std::regex(R"(\\)"), R"(\\)");
    size_t lastindex = fileOutputName.find_last_of(".");
    //give file name without extension and add double dash before
    std::string rawname = fileOutputName.substr(0, lastindex);
    std::string rawnamedoubleslash = std::regex_replace(rawname, std::regex(R"(\\)"), R"(\\)");
    //create entire command string
    std::string cmdstring = std::string("\"" + newFFmpegPath + "\" -i \"" + newFilePath + "\" -ar 48000 -y \"" + newConvertedFilesPath + rawnamedoubleslash + ".wav\"");
    std::wstring w = (utf8_to_utf16(cmdstring));
    DBG(cmdstring);
    LPWSTR str = const_cast<LPWSTR>(w.c_str());
    ////////////Launch FFMPEG
    PROCESS_INFORMATION pi;
    STARTUPINFOW si;
    ZeroMemory(&si, sizeof(si));
    BOOL logDone = CreateProcessW(NULL, str, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    if (logDone)
        {
            WaitForSingleObject(pi.hProcess, INFINITE);
        }



    /////////////////////return created file path
    std::string returnFilePath = std::string(newConvertedFilesPath + "\\" + rawname + ".wav");
    //DBG(returnFilePath);
    std::string returnFilePathBackslah = std::regex_replace(returnFilePath, std::regex(R"(\\)"), R"(\\)");
    juce::String returnedFile = juce::String(returnFilePath);

    return returnedFile;
}

std::string Player::extactName(std::string Filepath)
{
    std::string ExiftoolPath = juce::String(juce::File::getCurrentWorkingDirectory().getFullPathName() + "\\exiftool.exe").toStdString();
    std::string filePathSlash = std::regex_replace(Filepath, std::regex(R"(\\\\)"), R"(\)");
    std::string cmdexif = std::string(ExiftoolPath + " -b -Comment " + filePathSlash);
    LPSTR s = const_cast<char*>(cmdexif.c_str());
    newName = exec(s);

    if (!newName.empty())
    {
        soundName.setText(newName, juce::NotificationType::dontSendNotification);
    }
    else
    {
        cmdexif = std::string(ExiftoolPath + " -b -Technician " + filePathSlash);
        LPSTR s = const_cast<char*>(cmdexif.c_str());
        newName = exec(s);
        if (!newName.empty())
        {
            soundName.setText(newName, juce::NotificationType::dontSendNotification);
        }
    }

    return newName;
}



void Player::handleAudioTags(std::string filePath)
{

}

std::string Player::getFilePath()
{
    return loadedFilePath;
}


float Player::getTrimVolume()
{
    return trimVolumeSlider.getValue();
}

void Player::setTrimVolume(double trimVolume)
{
    trimVolumeSlider.setValue(trimVolume);
}

bool Player::getIsLooping()
{
    return looping;
}

void Player::setIsLooping(bool isLooping)
{
    looping = isLooping;
    loopButton.setToggleState(looping, juce::NotificationType::dontSendNotification);
}

std::string Player::getName()
{
    return newName;
}

void Player::setName(std::string Name)
{
    newName = Name;
    if (!newName.empty())
    {
        soundName.setText(newName, juce::NotificationType::dontSendNotification);
    }
}


void Player::setOptions()
{
    //float volumeSliderMaximum = double(Settings::maxFaderValueGlobal);
    volumeSlider.setRange(0., juce::Decibels::decibelsToGain(Settings::maxFaderValueGlobal), 0.0001);
    //volumeSlider.setValue(0.);
    
}

void Player::handleOSCMessage(float faderValue)
{
    DBG(faderValue);
    juce::NormalisableRange<float>valueToSendNorm(0., juce::Decibels::decibelsToGain(Settings::maxFaderValueGlobal), 0.001, Settings::skewFactorGlobal, false);
    //volumeSlider.setValue(roundf((valueToSendNorm.convertFrom0to1(faderValue)) * 100) / 100);
    volumeSlider.setValue(valueToSendNorm.convertFrom0to1(faderValue));
}

void Player::assignNextPlayer()
{
    nextPlayerAssigned = playerPosition;
}