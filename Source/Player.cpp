/*
  ==============================================================================

    Player.cpp
    Created: 26 Jan 2021 7:36:59pm
    Author:  Lucien

  ==============================================================================
*/
#pragma once
#include <JuceHeader.h>
#include "Player.h"
#include "Windows.h"
#define BLUE juce::Colour(40, 134, 189)
#define ORANGE juce::Colour(229, 149, 0)
#include "Settings.h"
#ifdef UNICODE
typedef LPWSTR LPTSTR;
#else
typedef LPSTR LPTSTR;
#endif
//#include <string>

//==============================================================================
Player::Player(int index)
{
     setMouseClickGrabsKeyboardFocus(false);
     playerIndex = index;
     juce::Timer::startTimer(50);
     state = Stopped;
    
     if (!isCart)
         waveformThumbnailXStart = leftControlsWidth + borderRectangleWidth;
     else if (isCart)
     {
         borderRectangleWidth = 0;
         waveformThumbnailXStart = leftControlsWidth;
     }
     unfocusAllComponents();
     playBroadcaster = new juce::ActionBroadcaster();
     cueBroadcaster = new juce::ActionBroadcaster();
     draggedBroadcaster = new juce::ActionBroadcaster();
     fxButtonBroadcaster = new juce::ChangeBroadcaster();
     playerInfoChangedBroadcaster = new juce::ChangeBroadcaster();
     remainingTimeBroadcaster = new juce::ChangeBroadcaster();
     envButtonBroadcaster = new juce::ChangeBroadcaster();
     trimValueChangedBroacaster = new juce::ChangeBroadcaster();
     soundEditedBroadcaster = new juce::ChangeBroadcaster();
     playerDeletedBroadcaster = new juce::ChangeBroadcaster();

     addMouseListener(this, true);
     addAndMakeVisible(&openButton);
     openButton.onClick = [this] { openButtonClicked(); };
     openButton.setBounds(rightControlsStart, 80, openDeleteButtonWidth, 20);
     openButton.setWantsKeyboardFocus(false);
     openButton.setMouseClickGrabsKeyboardFocus(false);
    
     addAndMakeVisible(&deleteButton);
     deleteButton.onClick = [this] { deleteFile(); };
     deleteButton.setBounds(rightControlsStart + openDeleteButtonWidth, 80, openDeleteButtonWidth, 20);
     deleteButton.setWantsKeyboardFocus(false);
     deleteButton.setMouseClickGrabsKeyboardFocus(false);

     addAndMakeVisible(&playButton);
     playButton.onClick = [this] { playButtonClicked(); };
     playButton.setBounds(rightControlsStart, 0, playStopButtonWidth, 39);
     playButton.setEnabled(false);
     playButton.setWantsKeyboardFocus(false);
     playButton.setMouseClickGrabsKeyboardFocus(false);

     addAndMakeVisible(&envButton);
     envButton.setButtonText("ENV");
     envButton.onClick = [this] {envButtonClicked(); };
     envButton.addListener(this);
     envButton.setMouseClickGrabsKeyboardFocus(false);

     addAndMakeVisible(&fxButton);
     fxButton.setButtonText("FX");
     fxButton.onClick = [this] {fxButtonClicked(); };
     fxButton.addListener(this);
     fxButton.setMouseClickGrabsKeyboardFocus(false);

     addAndMakeVisible(&normButton);
     normButton.setButtonText("Norm");
     normButton.onClick = [this] {normButtonClicked(); };
     normButton.addListener(this);
     normButton.setMouseClickGrabsKeyboardFocus(false);

     addAndMakeVisible(&denoiseButton);
     denoiseButton.setButtonText("DNS");
     denoiseButton.onClick = [this] {denoiseButtonClicked(); };
     denoiseButton.addListener(this);
     denoiser.denoiseDoneBroadcaster->addChangeListener(this);
     denoiser.processStartedBroadcaster->addChangeListener(this);
     denoiseButton.setMouseClickGrabsKeyboardFocus(false);

     stopButton.onClick = [this] { stopButtonClicked(); };
     stopButton.setBounds(rightControlsStart + playStopButtonWidth, 0, playStopButtonWidth, 39);
     stopButton.setEnabled(false);
     stopButton.setWantsKeyboardFocus(false);
     stopButton.setMouseClickGrabsKeyboardFocus(false);

     addAndMakeVisible(&cueButton);
     cueButton.onClick = [this] { cueButtonClicked(); };
     cueButton.setBounds(rightControlsStart + playStopButtonWidth, 0, playStopButtonWidth, 39);
     cueButton.setEnabled(false);
     cueButton.setWantsKeyboardFocus(false);
     cueButton.addListener(this);
     cueButton.setMouseClickGrabsKeyboardFocus(false);

    
     addAndMakeVisible(&loopButton);
     loopButton.setBounds(380, 80, 70, 20);
     loopButton.setButtonText("loop");
     loopButton.setEnabled(true);
     loopButton.onClick = [this]{ updateLoopButton(&loopButton, "loop");; };
     loopButton.setToggleState(false, true);
     loopButton.setWantsKeyboardFocus(false);
     loopButton.setMouseClickGrabsKeyboardFocus(false);

     addAndMakeVisible(volumeSlider);
     volumeSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);    
     volumeSlider.setBounds(rightControlsStart + rightControlsWidth, -5, volumeSliderWidth, 100);
     volumeSlider.setRange(0., 1., 0.01);
     volumeSlider.setValue(1.0);
     volumeSlider.addListener(this);
     sliderValueToset = 1.0;
     formatManager.registerBasicFormats();
     volumeSlider.setSkewFactor(0.5, false);
     volumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 50, 15);
     volumeSlider.setNumDecimalPlacesToDisplay(2);
     volumeSlider.setWantsKeyboardFocus(false);
     volumeSlider.setDoubleClickReturnValue(true, 1.);
     if (! Settings::mouseWheelControlVolume)
         volumeSlider.setScrollWheelEnabled(false);
     Settings::maxFaderValue.addListener(this);
     volumeSlider.setMouseClickGrabsKeyboardFocus(false);

    
     addAndMakeVisible(volumeLabel);
     volumeLabel.setFont(juce::Font(10.00f, juce::Font::plain).withTypefaceStyle("Regular"));
     volumeLabel.setJustificationType(juce::Justification::centred);
     volumeLabel.setEditable(false, false, false);
     volumeLabel.setBounds(rightControlsStart + rightControlsWidth, 90, volumeSliderWidth, 10);
     volumeLabel.setText(juce::String(juce::Decibels::gainToDecibels(volumeSlider.getValue())), juce::NotificationType::dontSendNotification);
     volumeLabel.setWantsKeyboardFocus(false);
     volumeLabel.setMouseClickGrabsKeyboardFocus(false);
    
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
     trimVolumeSlider.setScrollWheelEnabled(false);
     trimVolumeSlider.setWantsKeyboardFocus(false);
     trimVolumeSlider.setTextValueSuffix("dB");
     trimVolumeSlider.setMouseClickGrabsKeyboardFocus(false);

     addAndMakeVisible(trimLabel);
     trimLabel.setFont(juce::Font(15.00f, juce::Font::plain).withTypefaceStyle("Regular"));
     trimLabel.setJustificationType(juce::Justification::centred);
     trimLabel.setEditable(false, false, false);
     trimLabel.setText(TRANS("Trim"), juce::NotificationType::dontSendNotification);
     if (isCart)
         trimLabel.setBounds(-8, 81, 50, 24);
         else
         trimLabel.setBounds(borderRectangleWidth, 81, 50, 24);
     trimLabel.setWantsKeyboardFocus(false);
     trimLabel.setMouseClickGrabsKeyboardFocus(false);
    
    
     addAndMakeVisible(remainingTimeLabel);
     remainingTimeLabel.setBounds(450, 40, 150, 39);
     remainingTimeLabel.setText(TRANS("0"), juce::NotificationType::dontSendNotification);
     remainingTimeLabel.setJustificationType(juce::Justification::centred);
     remainingTimeLabel.setFont(juce::Font(35.00f, juce::Font::plain).withTypefaceStyle("Regular"));
     remainingTimeLabel.setWantsKeyboardFocus(false);
     remainingTimeLabel.setMouseClickGrabsKeyboardFocus(false);
    
     addAndMakeVisible(soundName);
     soundName.setBounds(leftControlsWidth + borderRectangleWidth, 80, 330, 20);
     soundName.setJustificationType(juce::Justification::centred);
     soundName.setFont(juce::Font(19.0f, juce::Font::bold).withTypefaceStyle("Regular"));
     soundName.setText(juce::String(""), juce::NotificationType::dontSendNotification);
     soundName.setWantsKeyboardFocus(false);
     soundName.setEditable(false, true, false);
     soundName.setMouseClickGrabsKeyboardFocus(false);
    
     addChildComponent(&normalizingLabel);
     normalizingLabel.setText("Normalizing...", juce::NotificationType::dontSendNotification);
     normalizingLabel.setFont(juce::Font(20.00f, juce::Font::bold).withTypefaceStyle("Regular"));
     normalizingLabel.setJustificationType(juce::Justification::centred);
     normalizingLabel.setColour(juce::Label::ColourIds::textColourId, juce::Colour(40, 134, 189));
     normalizingLabel.setColour(juce::Label::ColourIds::backgroundColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
     normalizingLabel.setAlpha(0.7);
     normalizingLabel.setMouseClickGrabsKeyboardFocus(false);

     addAndMakeVisible(optionButton);
     optionButton.onClick = [this] {optionButtonClicked(); };
     optionButton.setButtonText("HPF");
     optionButton.addListener(this);
     optionButton.setMouseClickGrabsKeyboardFocus(false);

     addChildComponent(&filterFrequencySlider);
     filterFrequencySlider.setRange(20, 200);
     filterFrequencySlider.setValue(filterFrequency, juce::NotificationType::dontSendNotification);
     filterFrequencySlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
     filterFrequencySlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 50, 10);
     filterFrequencySlider.setDoubleClickReturnValue(true, 0.);
     filterFrequencySlider.setPopupDisplayEnabled(true, true, this, 2000);
     filterFrequencySlider.setScrollWheelEnabled(false);
     filterFrequencySlider.setWantsKeyboardFocus(false);
     filterFrequencySlider.setNumDecimalPlacesToDisplay(0);
     filterFrequencySlider.addListener(this);
     filterFrequencySlider.setColour(juce::Slider::ColourIds::rotarySliderFillColourId, juce::Colours::lightblue);
     filterFrequencySlider.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, juce::Colours::lightblue);
     filterFrequencySlider.setTextValueSuffix("Hz");
     filterFrequencySlider.setMouseClickGrabsKeyboardFocus(false);

     outputMeter.setMeterColour(juce::Colour(229, 149, 0));
     outputMeter.shouldDrawScaleNumbers(false);
     outputMeter.shouldDrawExteriorLines(false);
    
     filterSource.makeInactive();
     cuefilterSource.makeInactive();
    
     addAndMakeVisible(startTimeButton);
     startTimeButton.onClick = [this] {  setTimeClicked(); };
     startTimeButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colour(0, 115, 150));
     startTimeButton.addListener(this);
     startTimeButton.setMouseClickGrabsKeyboardFocus(false);

     addAndMakeVisible(stopTimeButton);
     stopTimeButton.onClick = [this] { stopTimeClicked(); };
     stopTimeButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colour(141, 150, 0));
     stopTimeButton.addListener(this);
     stopTimeButton.setMouseClickGrabsKeyboardFocus(false);

    
     //TODO régler bug accents dans string
    
     addAndMakeVisible(playerIDLabel);
     playerIDLabel.setText(juce::String(playerIndex), juce::NotificationType::dontSendNotification);
         
     draggedPlayer.setValue(-1);
    
     thumbnail.addChangeListener(this);
     thumbnailBounds.setBounds(leftControlsWidth + borderRectangleWidth, 0, waveformThumbnailXSize, 80);
    
    
     if (isCart == true)
     {
         waveformThumbnailXSize = getParentWidth() - leftControlsWidth - rightControlsWidth - borderRectangleWidth * 2 - volumeSliderWidth - 2 * cartButtonsControlWidth - 20 - dragZoneWidth - 30;
         waveformThumbnailXEnd = waveformThumbnailXStart + waveformThumbnailXSize;
     }
     else if (isCart == false)
     {
         waveformThumbnailXSize = getParentWidth() - leftControlsWidth - rightControlsWidth - borderRectangleWidth * 2 - volumeSliderWidth - playlistButtonsControlWidth - dragZoneWidth;
         waveformThumbnailXEnd = waveformThumbnailXStart + waveformThumbnailXSize;
     }
     thumbnailBounds.setBounds(leftControlsWidth + borderRectangleWidth, 0, waveformThumbnailXSize, 80);
     rightControlsStart = leftControlsWidth + borderRectangleWidth + waveformThumbnailXSize;
     volumeLabelStart = rightControlsStart + rightControlsWidth;
     playButton.setBounds(rightControlsStart, 0, playStopButtonWidth, 39);
     openButton.setBounds(rightControlsStart, 80, openDeleteButtonWidth, 20);
     stopButton.setBounds(rightControlsStart + playStopButtonWidth, 0, playStopButtonWidth, 39);
     cueStopButton.setBounds(rightControlsStart + 2 * playStopButtonWidth, 0, playStopButtonWidth, 39);
     deleteButton.setBounds(rightControlsStart + openDeleteButtonWidth, 80, openDeleteButtonWidth, 20);
     remainingTimeLabel.setBounds(rightControlsStart, 40, 150, 39);
     volumeSlider.setBounds(volumeLabelStart, -5, volumeSliderWidth, 100);
     volumeLabel.setBounds(volumeLabelStart, 90, volumeSliderWidth, 10);
     soundName.setBounds(leftControlsWidth + borderRectangleWidth + optionButtonWidth, 80, waveformThumbnailXSize - 50 - optionButtonWidth, 20);
     loopButton.setBounds(leftControlsWidth + borderRectangleWidth + waveformThumbnailXSize - 50, 80, 50, 20);
    

     addChildComponent(playPlayHead);
     addChildComponent(cuePlayHead);
     addChildComponent(inMark);
     addChildComponent(outMark);
     playPlayHead.setSize(2, 80);
     cuePlayHead.setSize(2, 80);
     inMark.setSize(2, 80);
     outMark.setSize(2, 80);
     playPlayHead.setColour(juce::Colours::white);
     cuePlayHead.setColour(juce::Colours::black);
     inMark.setColour(juce::Colour(0, 196, 255));
     outMark.setColour(juce::Colour(238, 255, 0));
    
     
    
     mixer.addInputSource(&channelRemappingSource, false);
     cueMixer.addInputSource(&cuechannelRemappingSource, false);
     cueMixer.addInputSource(&denoiser.resampledSource, false);
     Settings::sampleRateValue.addListener(this);
     Settings::audioOutputModeValue.addListener(this);
     setChannelsMapping();
    
     convertingBar.reset(new juce::ProgressBar(progress));
     addChildComponent(*convertingBar);
     convertingBar->setTextToDisplay("Converting...");
    
     enableButtons(false);
     createDefaultEnveloppePath();
     repaint();
     for (int i = 0; i < getNumChildComponents(); i++)
     {
         getChildComponent(i)->setMouseClickGrabsKeyboardFocus(false);
     }
}


Player::~Player()
{
    playerDeletedBroadcaster->sendSynchronousChangeMessage();
    killThreads();
    denoiser.denoiseDoneBroadcaster->removeChangeListener(this);
    denoiser.processStartedBroadcaster->removeChangeListener(this);
    delete playBroadcaster;
    delete cueBroadcaster;
    delete draggedBroadcaster;
    delete fxButtonBroadcaster;
    delete playerInfoChangedBroadcaster;
    delete remainingTimeBroadcaster;
    delete envButtonBroadcaster;
    delete trimValueChangedBroacaster;
    delete soundEditedBroadcaster;
    delete playerDeletedBroadcaster;
    Settings::maxFaderValue.removeListener(this);
    Settings::audioOutputModeValue.removeListener(this);
    trimVolumeSlider.removeListener(this);
    volumeSlider.removeListener(this);
    cueButton.removeListener(this);
    Settings::sampleRateValue.removeListener(this);
}

void Player::paint (juce::Graphics& g)
{
    if (isCart == true)
        borderRectangleWidth = 0;
    g.setOpacity(1.0);
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));  
    outputMeter.setColour(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    if (state == Playing)
    {
        if (stopTime - transport.getCurrentPosition() < 6)
        {
            g.setColour(juce::Colours::red);
            outputMeter.setColour(juce::Colours::red);
        }
        else
        {
            g.setColour(juce::Colours::green);
            outputMeter.setColour(juce::Colours::green);
        }

    }
    else
    {
        if (isNextPlayer == true)
        {
            g.setColour(juce::Colour(229, 149, 0));
            outputMeter.setColour(juce::Colour(229, 149, 0));
            soundName.setColour(soundName.textColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        }
        else
        {
            g.setColour(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
            soundName.setColour(soundName.textColourId, juce::Colours::white);
            outputMeter.setColour(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        }
    }
    if (!fileLoaded)
    {
        g.setColour(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        outputMeter.setColour(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    }
    g.fillRoundedRectangle(0, 0, getWidth(), getHeight(), roundCornerSize);   // clear the background
    float x = 0.0f, y = 0.0f, width = 650.0f, height = 100.0f;
    juce::Colour fillColour = juce::Colour(0x23000000);
    juce::Colour strokeColour = juce::Colours::black;

    //Waveform
    if (thumbnail.getNumChannels() == 0)
        paintIfNoFileLoaded(g, thumbnailBounds);
    else
    {
        if (waveformPainted == false)
        {
            paintIfFileLoaded(g, thumbnailBounds, thumbnailZoomValue);
        }

    }

   if (isCart == false)
   {
   {//rectangle Fader1
       int x = 0, y = 40, width = 15, height = 20;
       if (faderLeftAssigned == true)
       {
           g.setColour(juce::Colours::red);
           g.fillRoundedRectangle(x, y, width, height, 3);
       }
   
       if (((stopTime - transport.getCurrentPosition() < 6)) && fileLoaded == true && state == Playing)
       {
           if (faderLeftAssigned == true)
           {
               g.setColour(juce::Colours::orange);
               g.fillRoundedRectangle(x, y, width, height, 3);
           }
       }
   }

       {//rectangle Fader2
           int x = getWidth() - 16, y = 40, width = 15, height = 20;
           if (faderRightAssigned == true)
           {
               g.setColour(juce::Colours::red);
               g.fillRoundedRectangle(x, y, width, height, 3);
           }
     
           if ((stopTime - transport.getCurrentPosition() < 6) && fileLoaded == true && state == Playing)
           {
               if (faderRightAssigned == true)
               {
                   g.setColour(juce::Colours::orange);
                   g.fillRoundedRectangle(x, y, width, height, 3);
               }
           }

       }
   }
    paintPlayHead(g, thumbnailBounds);
}

//WAVEFORM DRAWING
void Player::thumbnailChanged()
{
    waveformPainted = false;
    repaint();
}

void Player::paintIfNoFileLoaded(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds)
{
    g.setColour(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.fillRect(thumbnailBounds);
    g.setColour(juce::Colours::white);
    g.drawFittedText("No File Loaded", thumbnailBounds, juce::Justification::centred, 1);
}

void Player::paintIfFileLoaded(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds, float thumbnailZoomValue)
{
    if (state == Playing)
    {
        if ((stopTime - transport.getCurrentPosition() < 6))
            g.setColour(juce::Colours::red);
        else
            g.setColour(juce::Colours::green);
    }
    else
        if (isNextPlayer == true)   g.setColour(juce::Colour(229, 149, 0));
        else g.setColour(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.fillRect(thumbnailBounds);

    //waveform if next player or playing
    if (isNextPlayer || state == Playing)
        g.setColour(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    else
        g.setColour(juce::Colour(229, 149, 0));

    thumbnail.drawChannels(g,
        thumbnailBounds,
        0,                                    // start time
        (float)thumbnail.getTotalLength(),             // end time
        thumbnailZoomValue);                                  // vertical zoom
    

    //HIDE THE WAVEFORM WHEN DRAGGED

    /*if ((stopTime - transport.getCurrentPosition() < 6) && state == Playing)
        g.setColour(juce::Colours::red);
    else if (state == Playing)
        g.setColour(juce::Colours::green);
    else if (isNextPlayer == true)
        g.setColour(juce::Colour(229, 149, 0));
    else if (isNextPlayer == false)
          g.setColour(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    if (!isCart)
    {
        g.fillRoundedRectangle(0, 0, leftControlsWidth + borderRectangleWidth, borderRectangleHeight,4 );
        g.fillRoundedRectangle(rightControlsStart, 0, rightControlsWidth + volumeSliderWidth + 10, borderRectangleHeight, 4);
        if (isNextPlayer == false)
        {
            g.fillRoundedRectangle(0, 0, borderRectangleWidth, borderRectangleHeight, 4);
            g.fillRoundedRectangle(getParentWidth() - borderRectangleWidth - playlistButtonsControlWidth - 20, 0, borderRectangleWidth, borderRectangleHeight, 4);
        }
    }
    else if (isCart)
    {
        g.fillRoundedRectangle(0, 0, leftControlsWidth, borderRectangleHeight, 4);
        g.fillRoundedRectangle(rightControlsStart, 0, rightControlsWidth + volumeSliderWidth, borderRectangleHeight, 4);
        if (isNextPlayer == false)
        {
            g.fillRoundedRectangle(0, 0, borderRectangleWidth, borderRectangleHeight, 4);
            g.fillRoundedRectangle(getParentWidth() - borderRectangleWidth - playlistButtonsControlWidth - 20, 0, borderRectangleWidth, borderRectangleHeight, 4);
        }
    }*/
}

void Player::paintPlayHead(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds)
{
}

void Player::resized()
{
    envButton.setBounds(leftControlsWidth + borderRectangleWidth + 1, 81, fxButtonSize, 16);
    fxButton.setBounds(envButton.getRight() + 1, 81, fxButtonSize, 16);
    if (isCart == true)
    {
        if (!isEightPlayerMode)
            waveformThumbnailXSize = getWidth() - leftControlsWidth - rightControlsWidth - volumeSliderWidth;
        else
            waveformThumbnailXSize = getWidth() - leftControlsWidth - rightControlsWidth - volumeSliderWidth;
        waveformThumbnailXEnd = waveformThumbnailXStart + waveformThumbnailXSize;
        trimVolumeSlider.setBounds(0 - 8, 40, 64, 56);
        borderRectangleWidth = 0;
        soundName.setBounds(fxButton.getRight() + 1, 80, waveformThumbnailXSize - 50 - 2 * fxButtonSize, 20);
        trimLabel.setBounds(0, 81, 50, 24);
    }
    else if (isCart == false)
    {
        waveformThumbnailXSize = getWidth() - leftControlsWidth - rightControlsWidth - borderRectangleWidth * 2 - volumeSliderWidth;
        waveformThumbnailXEnd = waveformThumbnailXStart + waveformThumbnailXSize;
        soundName.setBounds(fxButton.getRight() + 1, 80, waveformThumbnailXSize - 2 * fxButtonSize, 20);
        trimLabel.setBounds(borderRectangleWidth, 81, 50, 24);
    }
    thumbnailBounds.setBounds(leftControlsWidth + borderRectangleWidth, 0, waveformThumbnailXSize, 80);
    normalizingLabel.setBounds(leftControlsWidth + borderRectangleWidth, 0, 150, 30);
    rightControlsStart = leftControlsWidth + borderRectangleWidth + waveformThumbnailXSize;
    volumeLabelStart = rightControlsStart + rightControlsWidth;
    playButton.setBounds(rightControlsStart, 0, playStopButtonWidth, 39);
    openButton.setBounds(rightControlsStart, 80, openDeleteButtonWidth, 20);
    cueButton.setBounds(rightControlsStart + playStopButtonWidth, 0, playStopButtonWidth, 39);
    cueStopButton.setBounds(rightControlsStart + 2 * playStopButtonWidth, 0, playStopButtonWidth, 39);
    deleteButton.setBounds(rightControlsStart + openDeleteButtonWidth, 80, openDeleteButtonWidth, 20);
    remainingTimeLabel.setBounds(rightControlsStart, 40, 150, 39);
    volumeSlider.setBounds(volumeLabelStart + 5, -5, volumeSliderWidth, 100);
    outputMeter.setBounds(volumeLabelStart + 3, 0, 17, 100);
    volumeLabel.setBounds(volumeLabelStart, 90, volumeSliderWidth, 10);
    loopButton.setBounds(leftControlsWidth + borderRectangleWidth + waveformThumbnailXSize - 50, 80, 50, 20);
    startTimeButton.setBounds(borderRectangleWidth + 4 , 26, cueStopButtonsSize + 4, cueStopButtonsSize);
    stopTimeButton.setBounds(borderRectangleWidth + 4 + cueStopButtonsSize + 8, 26, cueStopButtonsSize + 4, cueStopButtonsSize);
    optionButton.setBounds(borderRectangleWidth + 4, 2, 42, 20);
    convertingBar->setCentrePosition(thumbnailBounds.getCentre());
    convertingBar->setSize(200, 25);
    calculThumbnailBounds();
}


void Player::setLabelPlayerPosition(int p)
{
    playerPosition = p + 1;
}

void Player::setCuePlayHeadVisible(bool isVisible)
{
    cuePlayHead.setVisible(isVisible);
    drawCue = isVisible;
}
//TIMER
void Player::timerCallback()
{

    if (((stopTime - transport.getCurrentPosition() < 6)) && fileLoaded == true && state == Playing)
    {
        if (endRepainted == false)
        {
            repaint();
            endRepainted = true;
        }
    }


    volumeSlider.setValue(sliderValueToset);
    trimVolumeSlider.setValue(trimValueToSet);

    if ((float)transport.getCurrentPosition() > stopTime)
    {

    }
    if ((float)cueTransport.getCurrentPosition() > stopTime)
    {
        if (stopCueTransportOut)
        {
            cueTransport.stop();
            soundEditedBroadcaster->sendChangeMessage();
        }
    }
    updateRemainingTime();

    //Cue PlayHead
    auto cueaudioPosition = (float)cueTransport.getCurrentPosition();
    auto cuecurrentPosition = cueTransport.getLengthInSeconds();
    auto cuedrawPosition = ((cueaudioPosition / cuecurrentPosition) * (float)thumbnailBounds.getWidth())
            + (float)thumbnailBounds.getX();
    if (cueTransport.isPlaying() || mouseIsDragged)
    {
        if (cuedrawPosition > (leftControlsWidth + borderRectangleWidth) && cuedrawPosition < rightControlsStart)
        {
            cuePlayHead.setVisible(true);
            cuePlayHead.setTopLeftPosition(cuedrawPosition, 0);
        }
        else
            cuePlayHead.setVisible(false);
    }


    //CUE TIME LABEL
    int remainingSeconds = juce::int16(trunc((float)cueTransport.getLengthInSeconds() - (float)cueTransport.getCurrentPosition()));
    int remainingTimeSeconds = remainingSeconds % 60;
    int remainingTimeMinuts = trunc(remainingSeconds / 60);
    juce::String cueremainingTimeString;
    if (remainingTimeSeconds < 10)
        cueremainingTimeString = juce::String(remainingTimeMinuts) + ":0" + juce::String(remainingTimeSeconds);
    else
        cueremainingTimeString = juce::String(remainingTimeMinuts) + ":" + juce::String(remainingTimeSeconds);



    int elapsedSeconds = (float)cueTransport.getCurrentPosition();
    int elapsedTimeSeconds = elapsedSeconds % 60;
    int elapsedTimeMinuts = trunc(elapsedSeconds / 60);
    juce::String cueelapsedTimeString;
    if (elapsedTimeSeconds < 10)
        cueelapsedTimeString = juce::String(elapsedTimeMinuts) + ":0" + juce::String(elapsedTimeSeconds);
    else
        cueelapsedTimeString = juce::String(elapsedTimeMinuts) + ":" + juce::String(elapsedTimeSeconds);

    cueTimeString = (cueelapsedTimeString + " / " + cueremainingTimeString);

    if (cueTransport.isPlaying() || mouseIsDragged)
    {
        addAndMakeVisible(cueTimeLabel);
        cueTimeLabel.setText(cueTimeString, juce::NotificationType::dontSendNotification);
        if (cuedrawPosition > waveformThumbnailXEnd - 100)
            cueTimeLabel.setTopLeftPosition(waveformThumbnailXEnd - 100, 55);
        else
            cueTimeLabel.setTopLeftPosition(cuedrawPosition + 2, 55);
        cueTimeLabel.setSize(100, 25);
        cueTimeLabel.setFont(juce::Font(20.0f, juce::Font::bold).withTypefaceStyle("Regular"));
        cueTimeLabel.setColour(juce::Label::ColourIds::textColourId, juce::Colours::white);
        cueTimeLabel.setColour(juce::Label::ColourIds::backgroundColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        cueTimeLabel.setAlpha(0.7);
    }
    else
    {
        cueTimeLabel.setVisible(false);
    }

    if (transport.isPlaying())
    {
        //PLAYHEAD
        auto audioPosition = (float)transport.getCurrentPosition();
        auto currentPosition = transport.getLengthInSeconds();
        auto drawPosition = ((audioPosition / currentPosition) * (float)thumbnailBounds.getWidth())
            + (float)thumbnailBounds.getX();
        if (drawPosition > (leftControlsWidth + borderRectangleWidth) && drawPosition < rightControlsStart)
        {
            playPlayHead.setVisible(true);
            playPlayHead.setTopLeftPosition(drawPosition, 0);
        }
        else
            playPlayHead.setVisible(false);



        //ELAPSED TIME
        juce::String elapsedTimeString = secondsToMMSS(juce::int16((float)transport.getCurrentPosition()));
        addAndMakeVisible(elapsedTimeLabel);
        elapsedTimeLabel.setText(elapsedTimeString, juce::NotificationType::dontSendNotification);
        if (drawPosition > waveformThumbnailXEnd - 50)
            elapsedTimeLabel.setTopLeftPosition(waveformThumbnailXEnd - 50, 0);
        else
            elapsedTimeLabel.setTopLeftPosition(drawPosition + 2, 0);
        elapsedTimeLabel.setSize(50, 25);
        elapsedTimeLabel.setFont(juce::Font(20.0f, juce::Font::bold).withTypefaceStyle("Regular"));
        elapsedTimeLabel.setColour(juce::Label::ColourIds::textColourId, juce::Colours::white);
        elapsedTimeLabel.setColour(juce::Label::ColourIds::backgroundColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        elapsedTimeLabel.setAlpha(0.7);
    }
    else
    {
        elapsedTimeLabel.setVisible(false);
        playPlayHead.setVisible(false);
    }


    //Start Time
    if (startTimeSet)
    {
        auto startDrawPosition = (startTime * (float)thumbnailBounds.getWidth() / transport.getLengthInSeconds()) + (float)thumbnailBounds.getX();
        if (startDrawPosition > waveformThumbnailXStart && startDrawPosition < rightControlsStart)
        {
            inMark.setVisible(true);
            inMark.setTopLeftPosition(startDrawPosition, 0);
        }
        else
            inMark.setVisible(false);
    }
    else
        inMark.setVisible(false);
    if (stopTimeSet)
    {

        auto startDrawPosition = (stopTime * (float)thumbnailBounds.getWidth() / transport.getLengthInSeconds()) + (float)thumbnailBounds.getX();
        if (startDrawPosition > waveformThumbnailXStart && startDrawPosition < rightControlsStart)
        {
            outMark.setVisible(true);
            outMark.setTopLeftPosition(startDrawPosition, 0);
        }
        else
            outMark.setVisible(false);
    }
    else
            outMark.setVisible(false);
    
    if (shouldRepaint.load())
    {
        repaint();
        shouldRepaint.store(false);
    }

}

void Player::repaintPlayHead()
{
    auto cueaudioPosition = (float)cueTransport.getCurrentPosition();
    auto cuecurrentPosition = cueTransport.getLengthInSeconds();
    auto cuedrawPosition = ((cueaudioPosition / cuecurrentPosition) * (float)thumbnailBounds.getWidth())
        + (float)thumbnailBounds.getX();

    if (cuedrawPosition > (leftControlsWidth + borderRectangleWidth) && cuedrawPosition < rightControlsStart)
    {
        cuePlayHead.setVisible(true);
        cuePlayHead.setTopLeftPosition(cuedrawPosition, 0);
    }
    else
        cuePlayHead.setVisible(false);
}
void Player::updateRemainingTime()
{
    int remainingSeconds = juce::int16(trunc((float)transport.getLengthInSeconds() - (float)transport.getCurrentPosition()));
    if (stopTimeSet)
        remainingSeconds = juce::int16(trunc(stopTime - (float)transport.getCurrentPosition()));
    int remainingTimeSeconds = remainingSeconds % 60;
    int remainingTimeMinuts = trunc(remainingSeconds / 60);
    if (remainingTimeSeconds < 10)
        remainingTimeString = juce::String(remainingTimeMinuts) + ":0" + juce::String(remainingTimeSeconds);
    else
        remainingTimeString = juce::String(remainingTimeMinuts) + ":" + juce::String(remainingTimeSeconds);
    remainingTimeLabel.setText(remainingTimeString, juce::NotificationType::dontSendNotification);

    if (remainingTimeString.equalsIgnoreCase("0:05"))
    {
        remainingTimeBroadcaster->sendChangeMessage();
        repaint();
    }
}

//AUDIO OUTPUT
void Player::playerPrepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    actualSamplesPerBlockExpected = samplesPerBlockExpected;
    actualSampleRate = sampleRate;
    channelRemappingSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    cuechannelRemappingSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    resampledSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    cueResampledSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    filterSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    cuefilterSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    mixer.prepareToPlay(samplesPerBlockExpected, sampleRate);
    cueMixer.prepareToPlay(samplesPerBlockExpected, sampleRate);
    

    playerBuffer = std::make_unique<juce::AudioBuffer<float>>(2, actualSamplesPerBlockExpected);
    cueBuffer = std::make_unique<juce::AudioBuffer<float>>(2, actualSamplesPerBlockExpected);

    outputSource = std::make_unique <juce::MemoryAudioSource>(*playerBuffer, false);
    outputSource->prepareToPlay(samplesPerBlockExpected, sampleRate);
    playerBuffer->setSize(2, actualSamplesPerBlockExpected, false, true, false);
    cueOutputSource = std::make_unique <juce::MemoryAudioSource>(*cueBuffer, false);
    cueOutputSource->prepareToPlay(samplesPerBlockExpected, sampleRate);
    cueBuffer->setSize(2, actualSamplesPerBlockExpected, false, true, false);

    filterProcessor.prepareToPlay(actualSamplesPerBlockExpected, actualSampleRate);
    cueFilterProcessor.prepareToPlay(actualSamplesPerBlockExpected, actualSampleRate);
    compProcessor.prepareToPlay(actualSamplesPerBlockExpected, actualSampleRate);
    compProcessor.setGateBypass(true);
    compProcessor.setLimitBypass(true);

    inputMeter.prepareToPlay(actualSamplesPerBlockExpected, actualSampleRate);
    outputMeter.prepareToPlay(actualSamplesPerBlockExpected, actualSampleRate);
    compMeter.prepareToPlay(actualSamplesPerBlockExpected, actualSampleRate);

    meterSource.resize(2, sampleRate * 0.1 / samplesPerBlockExpected);
    outMeterSource.resize(2, sampleRate * 0.1 / samplesPerBlockExpected);
}

void Player::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (playerBuffer != nullptr)
    {
        juce::AudioSourceChannelInfo* playerSource = new juce::AudioSourceChannelInfo(*playerBuffer);
        playerBuffer->clear();
        mixer.getNextAudioBlock(*playerSource);

        juce::AudioSourceChannelInfo* cuePlayerSource = new juce::AudioSourceChannelInfo(*cueBuffer);
        cueBuffer->clear();
        cueMixer.getNextAudioBlock(*cuePlayerSource);

        float nextReadPosition = transport.getNextReadPosition();
        float cueNextReadPosition = cueTransport.getNextReadPosition();
        if (stopTimeSet || looping)
        {
            for (int i = 0; i < bufferToFill.numSamples; i++)
            {
                if (stopTimePosition < nextReadPosition + i)
                {
                    if (looping)
                    {
                        transport.setPosition(startTime);
                        shouldRepaint.store(true);
                    }
                    else
                    {
                        //bufferToFill.buffer->addSample(0, i, 0.0);
                        //bufferToFill.buffer->addSample(1, i, 0.0);
                        transport.setPosition(transport.getLengthInSeconds());
                        //transport.stop();
                    }
                }
            }
        }

        if (enveloppeEnabled)
        {
            juce::Path envP = enveloppePath;
            for (int i = 0; i < bufferToFill.numSamples; i++)
            {
                float enveloppePosition = (nextReadPosition + i) / (float)transport.getTotalLength();
                float gainToApply = juce::Decibels::decibelsToGain(getEnveloppeValue(enveloppePosition, envP).load() * 24);
                playerBuffer->applyGain(i, 1, gainToApply);

                float cueEnveloppePosition = (cueNextReadPosition + i) / (float)cueTransport.getTotalLength();
                float cueGainToApply = juce::Decibels::decibelsToGain(getEnveloppeValue(cueEnveloppePosition, envP).load() * 24);
                cueBuffer->applyGain(i, 1, cueGainToApply);
            }
        }

        if (transport.isPlaying() && playerBuffer != nullptr)
            inputMeter.measureBlock(playerBuffer.get());
        else if (cueBuffer != nullptr)
            inputMeter.measureBlock(cueBuffer.get());
        filterProcessor.getNextAudioBlock(playerBuffer.get());
        cueFilterProcessor.getNextAudioBlock(cueBuffer.get());
        compProcessor.getNextAudioBlock(playerBuffer.get());
        compMeter.setReductionGain(compProcessor.getCompReductionDB());
        meterSource.measureBlock(*playerBuffer.get());
        if (transport.isPlaying() && playerBuffer != nullptr)
        {
            outputMeter.measureBlock(playerBuffer.get());
            outMeterSource.measureBlock(*playerBuffer.get());
        }
        else if (cueBuffer != nullptr)
        {
            outputMeter.measureBlock(cueBuffer.get());
            outMeterSource.measureBlock(*playerBuffer.get());
        }
    }
}

std::atomic<float> Player::getEnveloppeValue(float x, juce::Path& p)
{//get the enveloppe value (between 0 and 1) for a given x position (between 0 and 1)
    std::atomic<float> value = 0.0;

    auto path = p;
    juce::Path::Iterator iterator(path);
    //creates an array of lines representing the enveloppe
    float lineStartX = 0.0;
    float lineStartY = 0.0;
    float lineEndX = 0.0;
    float lineEndY = 0.0;
    juce::Array<juce::Line<float>> linesArray;
    while (iterator.next())
    {
        if (iterator.elementType == juce::Path::Iterator::PathElementType::lineTo)
        {
            lineEndX = iterator.x1;
            lineEndY = iterator.y1;
            linesArray.add(juce::Line<float>(lineStartX, lineStartY, lineEndX, lineEndY));
            lineStartX = lineEndX;
            lineStartY = lineEndY;
        }
    }

    juce::Line<float> valueLine(x, 1.0, x, -1.0); //playhead line

    for (int i = 0; i < linesArray.size(); i++)
    {//for each line in the array, check if it intersect with the playhead
        if (linesArray[i].intersects(valueLine))
        {
            //If yes, get the y value corresponding
            juce::Point<float> intersectPoint = linesArray[i].getIntersection(valueLine);
            value.store(intersectPoint.getY());
        }
    }
    return value.load();
}

//Play Control
void Player::play()
{
    const juce::MessageManagerLock mmLock;
    transport.setPosition(startTime);
    transportStateChanged(Starting);
    soundEditedBroadcaster->sendChangeMessage();
}

void Player::stop()
{
    const juce::MessageManagerLock mmLock;
    transportStateChanged(Stopping);
    soundEditedBroadcaster->sendChangeMessage();
}

//BUTTONS
void Player::openButtonClicked()
{
    //create a chooser to open file
    juce::FileChooser chooser("Choose an audio File", juce::File::getSpecialLocation(juce::File::userDesktopDirectory), "*.wav; *.WAV; *.MP3; *.mp3; *.bwf; *.BWF; *.aiff; *.opus; *.flac"); 

    if (chooser.browseForFileToOpen())
    {
        deleteStart();
        deleteStop();
        enableHPF(false);
        juce::File myFile;
        myFile = chooser.getResult();
        
        auto filePath = myFile.getFullPathName();
        verifyAudioFileFormat(filePath);
    }
}

void Player::deleteFile()
{
    fxButton.setColour(juce::TextButton::ColourIds::buttonColourId,
        getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    envButton.setColour(juce::TextButton::ColourIds::buttonColourId,
        getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    transport.releaseResources();
    thumbnail.setSource(nullptr);
    transport.setSource(nullptr);
    enableButtons(false);
    loadedFilePath = "";
    deleteStart(false);
    deleteStop(false);
    enableHPF(false, false);
    newName = "";
    trimVolumeSlider.setValue(juce::Decibels::decibelsToGain(0.0));
    soundName.setText("", juce::NotificationType::dontSendNotification);
    fileName.setValue("");
    fileLoaded = false;
    sliderValueToset = 1.0;
    hasBeenNormalized = false;
    setFilterParameters(filterProcessor.makeDefaultFilter());
    playerDeletedBroadcaster->sendChangeMessage();
    createDefaultEnveloppePath();
    setEnveloppeEnabled(false);
    //bypassFX(true);
}


void Player::playButtonClicked()
{
    if (!transport.isPlaying())
    {
        if (Settings::lauchAtZeroDB)
            volumeSlider.setValue(1.0);
        play();
    }
    else
    {
        const juce::MessageManagerLock mmLock;
        if (looping == false)
        {
            stop();
            transport.setPosition(startTime);
            stopButtonClickedBool = false;
        }
        else if (looping == true)
        {
            transport.setPosition(startTime);
            repaint();
            stopButtonClickedBool = true;
            transportStateChanged(Stopping);
            stopButtonClickedBool = false;
        }
    }

    soundEditedBroadcaster->sendChangeMessage();
}

void Player::spaceBarPlay()
{
    if (juce::ModifierKeys::getCurrentModifiersRealtime() == juce::ModifierKeys::ctrlModifier)
    {
    }
    else
    {
        if (Settings::lauchAtZeroDB)
            volumeSlider.setValue(1.0);
        play();
    }
    soundEditedBroadcaster->sendChangeMessage();
}


void Player::stopButtonClicked()
{

    const juce::MessageManagerLock mmLock;
    if (looping == false)
    {
        stop();
        transport.setPosition(startTime);
        stopButtonClickedBool = false;
    }
    else if (looping == true)
    {
        transport.setPosition(startTime);
        stopButtonClickedBool = true;
        transportStateChanged(Stopping);
        stopButtonClickedBool = false;
    }
    soundEditedBroadcaster->sendChangeMessage();
}

void Player::buttonClicked(juce::Button* b)
{

}

void Player::buttonStateChanged(juce::Button* b)
{
    auto& modifiers = juce:: ModifierKeys::getCurrentModifiers();
    if (modifiers.isRightButtonDown())
    {
        rightClickDown = true;
    }
}
void Player::cueButtonClicked()
{
    if (fileLoaded)
    {
        if (rightClickDown)
        {
            if (cueTransport.isPlaying())
            {
                cueTransport.stop();
                cueTransport.setPosition(0.0);
                cueButton.setButtonText("Cue");
                startTimeButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colour(0, 196, 255));
                cueButton.setColour(juce::TextButton::ColourIds::buttonColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
            }
            else
            {
                cueBroadcaster->sendActionMessage(juce::String(playerIndex));

                cueTransport.setPosition(stopTime - 6);
                cueTransport.start();
                cueButton.setButtonText("Stop");
                cueButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::green);
                stopCueTransportOut = true;
            }
            rightClickDown = false;
        }

        else if (!cueTransport.isPlaying())
        {
            cueBroadcaster->sendActionMessage(juce::String(playerIndex));

            cueTransport.start();
            cueButton.setButtonText("Stop");
            cueButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::green);
        }
        else if (cueTransport.isPlaying())
        {
            cueTransport.stop();
            cueTransport.setPosition(0.0);
            cueButton.setButtonText("Cue");
            cueButton.setColour(juce::TextButton::ColourIds::buttonColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        }
    }
    soundEditedBroadcaster->sendChangeMessage();
}
void Player::cueStopButtonClicked()
{
    if (cueTransport.isPlaying())
    {
        cueTransport.stop();
        cueTransport.setPosition(0.0);
        cueStopButton.setButtonText("6s");
        cueButton.setButtonText("Cue");
    }
    else
    {
        cueTransport.setPosition(cueTransport.getLengthInSeconds() - 6);
        cueTransport.start();
        cueStopButton.setButtonText("Stop");
        cueButton.setButtonText("Stop");
    }
    soundEditedBroadcaster->sendChangeMessage();
}

void Player::updateLoopButton(juce::Button* button, juce::String name)
{
    auto state = button->getToggleState();
    loopButton.setToggleState(state, juce::NotificationType::sendNotification);
    looping = state;
    transport.setLooping(looping);
    soundEditedBroadcaster->sendChangeMessage();
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
            repaint();
        }
        else
        {
            transportStateChanged(Stopping);
            repaint();
            endRepainted = false;
        }
        soundEditedBroadcaster->sendChangeMessage();
    }
    else if (source == &thumbnail)
    {
        thumbnailChanged();
    }
    else if (source == &cueTransport)
    {
        if (!cueTransport.isPlaying())
        {
            cueTransport.stop();
            cueTransport.setPosition(0.0);
            cueButton.setButtonText("Cue");
            cueButton.setColour(juce::TextButton::ColourIds::buttonColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        }
    }
    else if (source == luThread.loudnessCalculatedBroadcaster)
    {
        integratedLoudness = luThread.getILU();
        double loudnessDifference = -23. - integratedLoudness;
        trimValueToSet = loudnessDifference;
        luThread.deleteProgressFile();
        normalizingLabel.setVisible(false);
        convertingBar->setVisible(false);
        convertingBar->setTextToDisplay("Converting...");
        hasBeenNormalized = true;
        soundEditedBroadcaster->sendChangeMessage();
    }
    else if (source == ffmpegThread.conversionEndedBroadcaster)
    {
        juce::String convertedFilePath = ffmpegThread.getFile();
        convertingBar->setVisible(false);
        loadFile(convertedFilePath, true);
        extactName(convertedFilePath.toStdString());
        ffmpegThread.conversionEndedBroadcaster->removeChangeListener(this);
        soundEditedBroadcaster->sendChangeMessage();
    }
    else if (source == denoiser.denoiseDoneBroadcaster)
    {
        std::string n = newName;
        std::string oldFilePath = loadedFilePath;
        denoisedFile = denoiser.getDenoisedFile();
        denoisedFileLoaded = true;
        loadFile(denoisedFile, true);
        setName(n);
        loadedFilePath = oldFilePath;
        convertingBar->setVisible(false);
        denoiseButton.setColour(juce::TextButton::ColourIds::buttonColourId, BLUE);
        soundEditedBroadcaster->sendChangeMessage();
    }
    else if (source == denoiser.processStartedBroadcaster)
    {
        if (denoiseWindow != nullptr)
        {
            if (juce::DialogWindow* dw = denoiser.findParentComponentOfClass<juce::DialogWindow>())
                dw->exitModalState(1234);
            convertingBar->setTextToDisplay("Denoising...");
            convertingBar->setVisible(true);
        }
    }
    playerInfoChangedBroadcaster->sendChangeMessage();
    soundEditedBroadcaster->sendChangeMessage();
}

void Player::transportStateChanged(TransportState newState)
{

    if (newState != state)
    {
        state = newState;


        switch (state)
        {
        case Stopped:
            playerInfoChangedBroadcaster->sendChangeMessage();
            transport.setPosition(startTime);
            playButton.setButtonText("Play");
                break;
        case Starting:
            playerInfoChangedBroadcaster->sendChangeMessage();
            playBroadcaster->sendActionMessage("Play");
            transport.start();
            playButton.setButtonText("Stop");
            break;
        case Stopping:
            playerInfoChangedBroadcaster->sendChangeMessage();
            playButton.setButtonText("Play");
            transport.stop();
            transport.setPosition(startTime);
            stopButtonClickedBool == false;
            break;       
        }

    }
    soundEditedBroadcaster->sendChangeMessage();
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
        sliderValueToset = slider->getValue();
        actualSliderValue = volumeSlider.getValue();
        transport.setGain(juce::Decibels::decibelsToGain(trimVolumeSlider.getValue()) * volumeSlider.getValue() * monoReductionGain);
        volumeLabel.setText(juce::String(round(juce::Decibels::gainToDecibels(volumeSlider.getValue()))), juce::NotificationType::dontSendNotification);
        if (previousSliderValue != actualSliderValue)
            volumeSliderValue = volumeSlider.getValue();
        previousSliderValue = actualSliderValue;

    }
    if (slider == &trimVolumeSlider)
    {
        trimValueToSet = slider->getValue();
        transport.setGain(juce::Decibels::decibelsToGain(trimVolumeSlider.getValue()) * volumeSlider.getValue() * monoReductionGain);
        cueTransport.setGain(juce::Decibels::decibelsToGain(trimVolumeSlider.getValue()) * monoReductionGain);
        thumbnailZoomValue = juce::Decibels::decibelsToGain(trimVolumeSlider.getValue());
        if (thumbnail.getNumChannels() != 0)
        {
            repaint();
        }
        trimValueChangedBroacaster->sendChangeMessage();
    }
    if (slider == &filterFrequencySlider)
    {
        filterFrequency = filterFrequencySlider.getValue();
        if (hpfEnabled)
        {
            filterSource.setCoefficients(filterCoefficients.makeHighPass(actualSampleRate, filterFrequency, 0.4));
            cuefilterSource.setCoefficients(filterCoefficients.makeHighPass(actualSampleRate, filterFrequency, 0.4));
        }
    }
    playerInfoChangedBroadcaster->sendChangeMessage();
    soundEditedBroadcaster->sendChangeMessage();
}


//WAVEFORM CONTROL
void Player::mouseDown(const juce::MouseEvent& event)
{
    thumbnailDragStart = getMouseXYRelative().getX();
    if (event.getNumberOfClicks() == 2)
    {
        thumbnailHorizontalZoom = 1;
        oldThumbnailOffset = 0;
        thumbnailOffset = 0;
    }
    if (thumbnail.getNumChannels() != 0)
    {
            mouseDragXPosition = getMouseXYRelative().getX();
            mouseDragYPosition = getMouseXYRelative().getY();
            if (mouseDragXPosition > leftControlsWidth + borderRectangleWidth && mouseDragXPosition < waveformThumbnailXEnd && mouseDragYPosition < 80)
            {
                mouseDragRelativeXPosition = getMouseXYRelative().getX() - thumbnailBounds.getPosition().getX();
                mouseDragInSeconds = (((float)mouseDragRelativeXPosition * cueTransport.getLengthInSeconds()) / (float)thumbnailBounds.getWidth());
                cueTransport.setPosition(mouseDragInSeconds);
                thumbnailMiddle = waveformThumbnailXSize / 2;
                thumbnailDrawStart = thumbnailMiddle - (thumbnailMiddle * thumbnailHorizontalZoom);
                thumbnailDrawEnd = thumbnailMiddle + (thumbnailMiddle * thumbnailHorizontalZoom);
                thumbnailDrawSize = thumbnailDrawEnd - thumbnailDrawStart;
                drawCue = true;
                mouseIsDragged = true;
                envButtonBroadcaster->sendChangeMessage();
                repaintThumbnail();
            }
    }

}

void Player::mouseUp(const juce::MouseEvent& event)
{
    draggedPlayer.setValue(-1);
    oldThumbnailOffset = thumbnailOffset;
    thumbnailDragStart = 0;
    setMouseCursor(juce::MouseCursor::NormalCursor);
    drawCue = false;
    mouseIsDragged = false;
}

void Player::mouseDrag(const juce::MouseEvent& event)
{
    if (thumbnail.getNumChannels() != 0
        && !event.mods.isCtrlDown())
    {
            mouseDragXPosition = getMouseXYRelative().getX();
            mouseDragYPosition = getMouseXYRelative().getY();
            if (mouseDragXPosition >= leftControlsWidth + borderRectangleWidth && mouseDragXPosition <= waveformThumbnailXEnd && mouseDragYPosition < 80)
            {
                mouseDragRelativeXPosition = getMouseXYRelative().getX() - thumbnailBounds.getPosition().getX();
                mouseDragInSeconds = (((float)mouseDragRelativeXPosition* cueTransport.getLengthInSeconds()) / (float)thumbnailBounds.getWidth());
                cueTransport.setPosition(mouseDragInSeconds);
                mouseIsDragged = true;

            }
    }
}

void Player::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
    if (getParentComponent() != nullptr) // passes only if it's not a listening event 
        getParentComponent()->mouseWheelMove(event, wheel);
}

void Player::mouseDoubleClick(const juce::MouseEvent& event)
{
}

void Player::calculThumbnailBounds()
{
    thumbnailMiddle = waveformThumbnailXSize / 2;
    thumbnailDrawStart = thumbnailMiddle - (thumbnailMiddle * thumbnailHorizontalZoom);
    thumbnailDrawEnd = thumbnailMiddle + (thumbnailMiddle * thumbnailHorizontalZoom);
    thumbnailDrawSize = thumbnailDrawEnd - thumbnailDrawStart;
    repaintThumbnail();
}

void Player::repaintThumbnail()
{
    thumbnailBounds.setBounds(leftControlsWidth + borderRectangleWidth + thumbnailDrawStart - thumbnailOffset, 0, thumbnailDrawSize, 80);
    repaint();
}
void Player::setStart()
{
    if ((float)cueTransport.getCurrentPosition() < stopTime && (float)cueTransport.getCurrentPosition() > 0)
    {
        startTime = (float)cueTransport.getCurrentPosition();
        startTimeSet = true;
        endRepainted = false;
        startTimeButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colour(0, 196, 255));
        if (!transport.isPlaying())
            transport.setPosition(startTime);
        updateRemainingTime();
        soundEditedBroadcaster->sendChangeMessage();
    }
}

void Player::setStartTime(float time, bool shouldSendMessage)
{
    startTime = time;
    startTimeSet = true;
    startTimeButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colour(0, 196, 255));
    if (!transport.isPlaying())
        transport.setPosition(startTime);
    updateRemainingTime();
    if (shouldSendMessage)
        soundEditedBroadcaster->sendChangeMessage();
}
void Player::setStopTime(float time, bool shouldSendMessage)
{
    stopTime = time;
    stopTimeSet = true;
    stopTimePosition = (stopTime / cueTransport.getLengthInSeconds()) * cueTransport.getTotalLength();
    stopTimeButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colour(238, 255, 0));
    endRepainted = false;
    if (shouldSendMessage)
        soundEditedBroadcaster->sendChangeMessage();
    repaint();
}


float Player::getStart()
{
    return startTime;
}

float Player::getStop()
{
    return stopTime;
}

void Player::deleteStart(bool shouldSendMessage)
{
    startTime = 0;
    startTimeSet = false;
    repaint();
    if (!transport.isPlaying())
        transport.setPosition(startTime);
    updateRemainingTime();
    startTimeButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colour(0, 115, 150));
    if (shouldSendMessage)
        soundEditedBroadcaster->sendChangeMessage();
}

void Player::setStop()
{
    if ((float)cueTransport.getCurrentPosition() > startTime && (float)cueTransport.getCurrentPosition() > 0)
    {
    stopTime = (float)cueTransport.getCurrentPosition();
    stopTimePosition = (cueTransport.getCurrentPosition() / cueTransport.getLengthInSeconds()) * cueTransport.getTotalLength();
    stopTimeSet = true;
    endRepainted = false;
    repaint();
    stopTimeButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colour(238, 255, 0));
    soundEditedBroadcaster->sendChangeMessage();
    }


}

void Player::deleteStop(bool shouldSendMessage)
{
    stopTime = (float)transport.getLengthInSeconds();
    stopTimeSet = false;
    repaint();
    stopTimeButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colour(141, 150, 0));
    if (shouldSendMessage)
        soundEditedBroadcaster->sendChangeMessage();
}


//MIDI CONTROL
void Player::handleMidiMessage(int midiMessageNumber, int midiMessageValue)
{

        floatMidiMessageValue = midiMessageValue / 127.;

        juce::NormalisableRange<float>rangedMidiMessage(0.0, juce::Decibels::decibelsToGain(Settings::maxFaderValueGlobal), 0.001, Settings::skewFactorGlobal, false);
        sliderValueToset = rangedMidiMessage.convertFrom0to1(floatMidiMessageValue);
        transport.setGain(juce::Decibels::decibelsToGain(trimVolumeSlider.getValue()) * sliderValueToset * monoReductionGain);
}

void Player::handleMidiTrimMessage(int midiMessageValue)
{
    float trimValue = midiMessageValue / 127.;

    juce::NormalisableRange<float>rangedTrimValue(-24, 24);

     trimValueInput = rangedTrimValue.convertFrom0to1(trimValue);
     if (trimValueInput == trimVolumeSlider.getValue())
         trimSliderRejoignedValue = true;
     else if (trimValueInput > trimVolumeSlider.getValue() - 1
         && trimValueInput < trimVolumeSlider.getValue() + 1)
         trimSliderRejoignedValue = true;
     if (trimSliderRejoignedValue)
         trimValueToSet = trimValueInput;
     transport.setGain(juce::Decibels::decibelsToGain(trimValueToSet) * sliderValueToset * monoReductionGain);
}

void Player::handleMidiTrimMessage(bool upordown)
{
    if (upordown)
        trimValueToSet += 1.;
    else
        trimValueToSet -= 1.;
    transport.setGain(juce::Decibels::decibelsToGain(trimValueToSet) * sliderValueToset * monoReductionGain);
}

void Player::setNextPlayer(bool trueOrFalse)
{
    isNextPlayer = trueOrFalse;
    if (trueOrFalse)
        setDraggedPlayer();
}

void Player::setLeftFaderAssigned(bool isFaderLeftAssigned)
{
    faderLeftAssigned = isFaderLeftAssigned;
    if (isCart && isFaderLeftAssigned)
        trimSliderRejoignedValue = false;
    else if (!isFaderLeftAssigned && !faderRightAssigned)
        trimSliderRejoignedValue = false;
    repaint();
}



void Player::setRightFaderAssigned(bool isFaderRightAssigned)
{
    faderRightAssigned = isFaderRightAssigned;
    if (isCart && isFaderRightAssigned)
        trimSliderRejoignedValue = false;
    else if (!isFaderRightAssigned && !faderLeftAssigned)
        trimSliderRejoignedValue = false;
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
            loadFile(file.getFullPathName(), true);
            delete reader;
        }
        else
        {
            std::string pathstd = path.toStdString();
            juce::String newpath = startFFmpeg(pathstd);
            std::string newpathstd = newpath.toStdString();
            loadFile(newpathstd, false);
        }
    }
    else
    {
        juce::String correctedPath;
        ffmpegThread.conversionEndedBroadcaster->addChangeListener(this);
        ffmpegThread.setFilePath(path);
        ffmpegThread.shouldMakeProgressFile(false);
        ffmpegThread.startThread();
        convertingBar->setVisible(true);
        convertingBar->setTextToDisplay("Converting...");
    }
}


bool Player::loadFile(const juce::String& path, bool shouldSendChangeMessage)
{
    loadedFilePath = path.toStdString();
    juce::File file(path);
    if (juce::AudioFormatReader* reader = formatManager.createReaderFor(file))
    {
        //transport
        std::unique_ptr<juce::AudioFormatReaderSource> tempSource(new juce::AudioFormatReaderSource(reader, true));
        transport.setSource(tempSource.get());
        transport.addChangeListener(this);
        transport.setGain(1.0);
        sliderValueToset = 1.0;

        resampledSource.setResamplingRatio(reader->sampleRate / Settings::sampleRate);
        fileSampleRate = reader->sampleRate;
        playSource.reset(tempSource.release());

        setChannelsMapping();



        fileName = file.getFileNameWithoutExtension();
        newName = file.getFileName().toStdString();
        soundName.setText(fileName.toString(), juce::NotificationType::dontSendNotification);
        playButton.setEnabled(true);
        cueStopButton.setEnabled(true);
        stopButton.setEnabled(true);
        trimVolumeSlider.setValue(0, juce::NotificationType::dontSendNotification);
        trimSliderRejoignedValue = false;
        thumbnailZoomValue = 1.;

        transportPositionSlider.setEnabled(true);
        transportPositionSlider.setNumDecimalPlacesToDisplay(0);
        transportPositionSlider.setRange(0, transport.getLengthInSeconds());

        startTime = 0;
        stopTime = (float)transport.getLengthInSeconds();
        stopTimePosition = transport.getTotalLength();
        endRepainted = false;

        thumbnail.setSource(new juce::FileInputSource(file));
        waveformPainted = false;
        fileLoaded = true;
        enableButtons(true);
        //soundEditedBroadcaster->sendChangeMessage();
    }
    //R128
    if (Settings::autoNormalize && !hasBeenNormalized)
    {
        normalize(loadedFilePath);
    }
    //cue transport
    if (juce::AudioFormatReader* cuereader = formatManager.createReaderFor(file))
    {
        std::unique_ptr<juce::AudioFormatReaderSource> cuetempSource(new juce::AudioFormatReaderSource(cuereader, true));
        cueTransport.setSource(cuetempSource.get());
        cueTransport.addChangeListener(this);
        cueTransport.setGain(1.0);
        cueResampledSource.setResamplingRatio(cuereader->sampleRate / Settings::sampleRate);
        cuePlaySource.reset(cuetempSource.release());
        cueButton.setEnabled(true);
        //setDraggedPlayer();
    }
    else
    {
        return false;
    }
    if (shouldSendChangeMessage)
    {
        playerInfoChangedBroadcaster->sendChangeMessage();
        soundEditedBroadcaster->sendChangeMessage();
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
    std::string returnFilePathBackslah = std::regex_replace(returnFilePath, std::regex(R"(\\)"), R"(\\)");
    juce::String returnedFile = juce::String(returnFilePath);
    Settings::tempFiles.add(returnedFile);
    return returnedFile;
}

std::string Player::extactName(std::string Filepath)
{
    auto const connection_string = NANODBC_TEXT("Driver=ODBC Driver 17 for SQL Server;Server=localhost\\NETIA;Database=ABC4;Uid=SYSADM;Pwd=SYSADM;");
    nanodbc::connection conn;
    try
    {
        nanodbc::result row;
        conn.connect(connection_string, 1000);
        juce::File bwfFile(Filepath);
        std::string fileName = bwfFile.getFileNameWithoutExtension().toStdString();
        std::string searchString = std::string("SELECT * FROM ABC4.SYSADM.T_ITEM WHERE ABC4.SYSADM.T_ITEM.[FILE] LIKE '%" + fileName + "%'");
        row = execute(
            conn,
            searchString);
        row.next();
        newName = row.get<nanodbc::string>(45);
    }
    catch (std::runtime_error const& e)
    {
        std::cerr << e.what() << std::endl;
    }

    if (!newName.empty())
    {
        soundName.setText(newName, juce::NotificationType::dontSendNotification);
        soundEditedBroadcaster->sendChangeMessage();
    }

    return newName;
}



void Player::handleAudioTags(std::string filePath)
{

}


Player::PlayerInfo Player::getPlayerInfo()
{
    Player::PlayerInfo info;
    info.filePath = getFilePath();
    info.name = getName();
    info.volume = getVolume();
    info.trimVolume = getTrimVolume();
    info.loop = getIsLooping();
    return info;
}

std::string Player::getFilePath()
{
    return loadedFilePath;
}

float Player::getVolume()
{
    return volumeSlider.getValue();
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

void Player::setIsLooping(bool isLooping, bool shouldSendMessage)
{
    looping = isLooping;
    loopButton.setToggleState(looping, juce::NotificationType::dontSendNotification);
    if (shouldSendMessage)
        soundEditedBroadcaster->sendChangeMessage();
}

std::string Player::getName()
{
    return soundName.getText().toStdString();
}

bool Player::getHasBeenNormalized()
{
    return hasBeenNormalized;
}

void Player::setHasBeenNormalized(bool b)
{
    hasBeenNormalized = b;
}

void Player::setName(std::string Name)
{
    newName = Name;
    if (!newName.empty())
    {
        soundName.setText(newName, juce::NotificationType::dontSendNotification);
    }
}

bool Player::isStartTimeSet()
{
    return startTimeSet;
}

bool Player::isStopTimeSet()
{
    return stopTimeSet;
}

void Player::setOptions()
{
    volumeSlider.setRange(0., juce::Decibels::decibelsToGain(Settings::maxFaderValueGlobal), 0.0001);    
}

void Player::handleOSCMessage(float faderValue)
{
    juce::NormalisableRange<float>valueToSendNorm(0., juce::Decibels::decibelsToGain(Settings::maxFaderValueGlobal), 0.001, Settings::skewFactorGlobal, false);
    volumeSlider.setValue(valueToSendNorm.convertFrom0to1(faderValue));
}

void Player::assignNextPlayer()
{
    nextPlayerAssigned = playerPosition;
}


void Player::setTimeClicked()
{
    if (rightClickDown)
    {
        deleteStart();
        rightClickDown = false;
    }
    else
        setStart();
}

void Player::stopTimeClicked()
{
    if (rightClickDown)
    {
        deleteStop();
        rightClickDown = false;
    }
    else 
        setStop();
}

void Player::optionButtonClicked()
{
    if (!rightClickDown)
    {
        if (hpfEnabled)
        {
            enableHPF(false);
            filterFrequencySlider.setVisible(false);
            trimVolumeSlider.setVisible(true);
            trimLabel.setText("Trim", juce::NotificationType::dontSendNotification);
        }
        else if (!hpfEnabled)
        {
            enableHPF(true);
        }
    }
    else
    {
        if (!filterFrequencySlider.isVisible())
        {
            if (!hpfEnabled)
                enableHPF(true);
            trimVolumeSlider.setVisible(false);
            filterFrequencySlider.setVisible(true);
            filterFrequencySlider.setTopLeftPosition(trimVolumeSlider.getPosition());
            filterFrequencySlider.setSize(trimVolumeSlider.getWidth(), trimVolumeSlider.getHeight());
            trimLabel.setText("HPF", juce::NotificationType::dontSendNotification);
            rightClickDown = false;
        }
        else
        {
            filterFrequencySlider.setVisible(false);
            trimVolumeSlider.setVisible(true);
            trimLabel.setText("Trim", juce::NotificationType::dontSendNotification);
            rightClickDown = false;
        }

    }
    soundEditedBroadcaster->sendChangeMessage();
}


void Player::enableHPF(bool shouldBeEnabled, bool shouldSendMessage)
{
    if (!shouldBeEnabled)
    {
        filterSource.makeInactive();
        cuefilterSource.makeInactive();
        optionButton.setColour(juce::TextButton::ColourIds::buttonColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        optionButton.setColour(juce::TextButton::ColourIds::textColourOffId, juce::Colours::white);
        hpfEnabled = false;
    }
    else if (shouldBeEnabled)
    {
        filterSource.setCoefficients(filterCoefficients.makeHighPass(actualSampleRate, filterFrequency, 0.4));
        cuefilterSource.setCoefficients(filterCoefficients.makeHighPass(actualSampleRate, filterFrequency, 0.4));
        optionButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::lightblue);
        optionButton.setColour(juce::TextButton::ColourIds::textColourOffId, juce::Colours::black);
        hpfEnabled = true;
    }
    if (shouldSendMessage)
        soundEditedBroadcaster->sendChangeMessage();
}

bool Player::isHpfEnabled()
{
    if (hpfEnabled)
        return true;
    else
        return false;
}

void Player::valueChanged(juce::Value& value)
{
    if (value.refersToSameSourceAs(Settings::audioOutputModeValue))
    {
        setChannelsMapping();
    }
    else if (value.refersToSameSourceAs(Settings::sampleRateValue))
    {
        actualSampleRate = Settings::sampleRate;
        if (fileLoaded)
        {
            resampledSource.setResamplingRatio(fileSampleRate / Settings::sampleRate);
            cueResampledSource.setResamplingRatio(fileSampleRate / Settings::sampleRate);
        }
    }
    else if (value.refersToSameSourceAs(Settings::maxFaderValue))
        volumeSlider.setRange(0, juce::Decibels::decibelsToGain(Settings::maxFaderValue.toString().getDoubleValue()));

}

void Player::setChannelsMapping()
{
    if (Settings::audioOutputMode == 1)
    {

        channelRemappingSource.clearAllMappings();
        channelRemappingSource.setOutputChannelMapping(0, 0);
        channelRemappingSource.setOutputChannelMapping(1, 0);
        cuechannelRemappingSource.clearAllMappings();
        cuechannelRemappingSource.setOutputChannelMapping(0, 0);
        cuechannelRemappingSource.setOutputChannelMapping(1, 0);
        monoReductionGain = juce::Decibels::decibelsToGain(-6.0);
        transport.setGain(juce::Decibels::decibelsToGain(trimVolumeSlider.getValue()) * volumeSlider.getValue() * monoReductionGain);
        cueTransport.setGain(monoReductionGain * juce::Decibels::decibelsToGain(trimVolumeSlider.getValue()));
    }
    else
    {
        channelRemappingSource.clearAllMappings();
        channelRemappingSource.setOutputChannelMapping(0, 0);
        channelRemappingSource.setOutputChannelMapping(1, 1);        
        cuechannelRemappingSource.clearAllMappings();
        cuechannelRemappingSource.setOutputChannelMapping(0, 0);
        cuechannelRemappingSource.setOutputChannelMapping(1, 1);
        monoReductionGain = juce::Decibels::decibelsToGain(0.0);
        transport.setGain(juce::Decibels::decibelsToGain(trimVolumeSlider.getValue()) * volumeSlider.getValue() * monoReductionGain);
        cueTransport.setGain(monoReductionGain * juce::Decibels::decibelsToGain(trimVolumeSlider.getValue()));
    }
}

juce::String Player::secondsToMMSS(int seconds)
{
    int timeSeconds = seconds % 60;
    int timeMinuts = trunc(seconds / 60);
    juce::String timeString;
    if (timeSeconds < 10)
        timeString = juce::String(timeMinuts) + ":0" + juce::String(timeSeconds);
    else
        timeString = juce::String(timeMinuts) + ":" + juce::String(timeSeconds);
    return timeString;
}


void Player::setTimerTime(int timertime)
{
    juce::Timer::startTimer(timertime);
}



void Player::setEightPlayerMode(bool isEight)
{
    isEightPlayerMode = isEight;
}

void Player::setActivePlayer(bool isActive)
{
    if (isActive)
    {
        isActivePlayer = true;
        setCuePlayHeadVisible(true);
        repaint();
    }
    else
    {
        isActivePlayer = false;
        setCuePlayHeadVisible(false);
        repaint();
    }
}

void Player::setPlayerIndex(int i)
{
    playerIndex = i;
}

double Player::CalculateR128Integrated(std::string filePath)
{
    luThread.loudnessCalculatedBroadcaster->addChangeListener(this);
    luThread.setFilePath(filePath);
    luThread.startThread();

    return -1.;
}

FilterProcessor& Player::getFilterProcessor()
{
    return filterProcessor;
}

std::unique_ptr<juce::AudioBuffer<float>>& Player::getBuffer()
{
    return playerBuffer;
}

FilterProcessor::GlobalParameters Player::getFilterParameters()
{
    FilterProcessor::GlobalParameters globalParams;
    globalParams.lowBand = filterProcessor.getFilterParameters(0);
    globalParams.lowMidBand = filterProcessor.getFilterParameters(1);
    globalParams.highMidBand = filterProcessor.getFilterParameters(2);
    globalParams.highBand = filterProcessor.getFilterParameters(3);
    return globalParams;
}

void Player::setFilterParameters(FilterProcessor::GlobalParameters g)
{
    filterProcessor.setFilterParameters(0, g.lowBand);
    filterProcessor.setFilterParameters(1, g.lowMidBand);
    filterProcessor.setFilterParameters(2, g.highMidBand);
    filterProcessor.setFilterParameters(3, g.highBand);    
    cueFilterProcessor.setFilterParameters(0, g.lowBand);
    cueFilterProcessor.setFilterParameters(1, g.lowMidBand);
    cueFilterProcessor.setFilterParameters(2, g.highMidBand);
    cueFilterProcessor.setFilterParameters(3, g.highBand);
}

void Player::setFilterParameters(int i, FilterProcessor::FilterParameters p)
{
    filterProcessor.setFilterParameters(i, p);
    cueFilterProcessor.setFilterParameters(i, p);
}

Meter& Player::getInputMeter()
{
    return inputMeter;
}

Meter& Player::getOutputMeter()
{
    return outputMeter;
}

Meter& Player::getCompMeter()
{
    return compMeter;
}

void Player::setDraggedPlayer()
{
    Settings::draggedPlayer = playerIndex;
    draggedPlayer.setValue(-1);
    draggedPlayer.setValue(playerIndex);
}

void Player::fxButtonClicked()
{
    if (rightClickDown)
    {
        fxEnabled = !fxEnabled;
        bypassFX(!fxEnabled);
        fxButtonBroadcaster->sendChangeMessage();
        soundEditedBroadcaster->sendChangeMessage();
        rightClickDown = false;
    }
    else
    {
        if (!fxEnabled)
        {
            fxEnabled = true;
            bypassFX(!fxEnabled);
            fxButtonBroadcaster->sendChangeMessage();
            soundEditedBroadcaster->sendChangeMessage();
        }
        else
        {
            fxButtonBroadcaster->sendChangeMessage();
            soundEditedBroadcaster->sendChangeMessage();
        }

    }

}

void Player::envButtonClicked()
{
    if (rightClickDown)
    {
        setEnveloppeEnabled(!enveloppeEnabled);
        rightClickDown = false;
    }
    else
    {
        if (!enveloppeEnabled)
        {
            setEnveloppeEnabled(true);
        }
        envButtonBroadcaster->sendChangeMessage();
    }
    soundEditedBroadcaster->sendChangeMessage();
}

void Player::bypassFX(bool isBypassed, bool shouldSendMessage)
{
    fxEnabled = !isBypassed;
    filterProcessor.setBypassed(isBypassed);
    cueFilterProcessor.setBypassed(isBypassed);
    compProcessor.setBypass(isBypassed);
    if (isEdited)
        fxButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::red);
    else if (isBypassed)
        fxButton.setColour(juce::TextButton::ColourIds::buttonColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    else
        fxButton.setColour(juce::TextButton::ColourIds::buttonColourId, BLUE);
    if (shouldSendMessage)
    {
        soundEditedBroadcaster->sendChangeMessage();
        fxButtonBroadcaster->sendChangeMessage();
    }
}

bool Player::getBypassed()
{
    return !fxEnabled;
}

void Player::normButtonClicked()
{
    normalize(loadedFilePath);
}

void Player::normalize(std::string p)
{
    CalculateR128Integrated(p);
    convertingBar->setTextToDisplay("Normalizing...");
    convertingBar->setVisible(true);
}

void Player::setEditedPlayer(bool b)
{
    isEdited = b;
    if (isEdited)
    {
        fxButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::red);
        envButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::red);
    }
    else
    {
        if (fxEnabled)
            fxButton.setColour(juce::TextButton::ColourIds::buttonColourId, BLUE);
        else
            fxButton.setColour(juce::TextButton::ColourIds::buttonColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        if (enveloppeEnabled)
            envButton.setColour(juce::TextButton::ColourIds::buttonColourId, BLUE);
        else
            envButton.setColour(juce::TextButton::ColourIds::buttonColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    }
    soundEditedBroadcaster->sendChangeMessage();
}

juce::TextButton* Player::getfxButton()
{
    return &fxButton;
}

void Player::denoiseButtonClicked()
{
    if (rightClickDown)
    {
        if (denoisedFileLoaded)
        {
            std::string n = newName;
            loadFile(loadedFilePath, true);
            denoiseButton.setColour(juce::TextButton::ColourIds::buttonColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
            denoisedFileLoaded = false;
            setName(n);
        }
        else
        {
            if (!denoisedFile.empty())
            {
                std::string n = newName;
                std::string oldFilePath = loadedFilePath;
                loadFile(denoisedFile, true);
                denoiseButton.setColour(juce::TextButton::ColourIds::buttonColourId, BLUE);
                denoisedFileLoaded = true;
                setName(n);
                loadedFilePath = oldFilePath;
            }
        }
        rightClickDown = false;
    }
    else
    {
        denoiseWindow = std::make_unique<juce::DialogWindow>("Denoiser" + soundName.getText(), getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), true, true);
        denoiseWindow->showDialog("Denoiser - " + soundName.getText(), &denoiser, this, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), true);
        denoiser.setFilePath(loadedFilePath);
        denoiser.setTransportGain(trimVolumeSlider.getValue());
    }
    soundEditedBroadcaster->sendChangeMessage();
}

void Player::killThreads()
{
    ffmpegThread.conversionEndedBroadcaster->removeChangeListener(this);
    denoiser.denoiseDoneBroadcaster->removeChangeListener(this);
    denoiser.processStartedBroadcaster->removeChangeListener(this);
    luThread.killThread();
    denoiser.killThread();
    ffmpegThread.killThread();
}

bool Player::isPlayerPlaying()
{
    return transport.isPlaying();
}

bool Player::isLastSeconds()
{
    if (stopTime - transport.getCurrentPosition() < 6)
        return true;
    else
        return false;
}

bool Player::isFileLoaded()
{
    auto b = fileLoaded;
    return b;
}

juce::AudioThumbnailCache& Player::getAudioThumbnailCache()
{
    return thumbnailCache;
}

juce::AudioFormatManager& Player::getAudioFormatManager()
{
    return formatManager;
}

juce::AudioThumbnail& Player::getAudioThumbnail()
{
    return thumbnail;
}
juce::String Player::getRemainingTimeAsString()
{
    return remainingTimeString;
}

juce::String Player::getCueTimeAsString()
{
    return cueTimeString;
}

void Player::setEnveloppePath(juce::Path& p)
{
    const juce::ScopedLock sl();
    enveloppePath = p;
}

void Player::createDefaultEnveloppePath()
{
    enveloppePath.clear();
    enveloppePath.startNewSubPath(0.0, 0.0);
    enveloppePath.lineTo(1.0, 0.0);
    enveloppePath.closeSubPath();
}

juce::Path* Player::getEnveloppePath()
{
    return &enveloppePath;
}

bool Player::getIsCart()
{
    return isCart;
}

bool Player::isEnveloppeEnabled()
{
    return enveloppeEnabled;
}

void Player::setEnveloppeEnabled(bool b, bool shouldSendMessage, bool switchToEnveloppePanel)
{
    enveloppeEnabled = b;
    if (enveloppeEnabled && !isEdited)
        envButton.setColour(juce::TextButton::ColourIds::buttonColourId, BLUE);
    else if (isEdited)
        envButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::red);
    else
        envButton.setColour(juce::TextButton::ColourIds::buttonColourId, 
            getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    if (shouldSendMessage)
        soundEditedBroadcaster->sendChangeMessage();
    if (switchToEnveloppePanel)
        envButtonBroadcaster->sendChangeMessage();
}

bool Player::isFxEnabled()
{
    return fxEnabled;
}

void Player::enableButtons(bool b)
{
    playButton.setEnabled(b);
    stopButton.setEnabled(b);
    cueButton.setEnabled(b);
    loopButton.setEnabled(b);
    volumeSlider.setEnabled(b);
    trimVolumeSlider.setEnabled(b);
    envButton.setEnabled(b);
    fxButton.setEnabled(b);
}

bool Player::isEditedPlayer()
{
    return isEdited;
}

float Player::getLenght()
{
    if (fileLoaded)
        return transport.getLengthInSeconds();
    else
        return 0.0;
}

juce::Array<float> Player::getEnveloppeXArray()
{
    juce::Path::Iterator iterator(enveloppePath);

    juce::Array<float> xArray;
    xArray.clear();
    while (iterator.next())
    {
        if (iterator.elementType == juce::Path::Iterator::PathElementType::startNewSubPath)
        {
            xArray.add(iterator.x1);
        }
        else if (iterator.elementType == juce::Path::Iterator::PathElementType::lineTo)
        {
            xArray.add(iterator.x1);
        }
        else if (iterator.elementType == juce::Path::Iterator::PathElementType::closePath)
        {
            return xArray;
        }
    }
}

juce::Array<float> Player::getEnveloppeYArray()
{
    juce::Path::Iterator iterator(enveloppePath);

    juce::Array<float> yArray;
    yArray.clear();
    while (iterator.next())
    {
        if (iterator.elementType == juce::Path::Iterator::PathElementType::startNewSubPath)
        {
            yArray.add(iterator.y1);
        }
        else if (iterator.elementType == juce::Path::Iterator::PathElementType::lineTo)
        {
            yArray.add(iterator.y1);
        }
        else if (iterator.elementType == juce::Path::Iterator::PathElementType::closePath)
        {
            return yArray;
        }
    }
}

juce::Path Player::createEnveloppePathFromArrays(juce::Array<float> xArray, juce::Array<float> yArray)
{
    juce::Path p;
    p.clear();
    p.startNewSubPath(0.0, 0.0);
    for (int i = 0; i < xArray.size(); i++)
    {
        p.lineTo(xArray[i], yArray[i]);
    }
    p.closeSubPath();
    return p;
}