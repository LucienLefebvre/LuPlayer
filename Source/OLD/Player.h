/*
  ==============================================================================

    Player.h
    Created: 26 Jan 2021 7:36:59pm
    Author:  Lucien

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <string>
#include <iostream>
#include <regex>
#define NOMINMAX
#include "Windows.h"





typedef char* LPSTR;



//==============================================================================
/*
*/
class Player :  public juce::Component,
                private juce::ChangeListener,
                public juce::Slider::Listener,
                public juce::Timer
                //public juce::FileDragAndDropTarget
                //public juce::AudioFormatReaderSource
                //public juce::Component::MouseListener
                //public juce::MultiTimer
                //public juce::ComboBox::Listener,
                //public juce::Button::Listener
{
public:
    Player(int index);
    ~Player() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void Player::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill);
    void Player::playerPrepareToPlay(int samplesPerBlockExpected, double sampleRate);

    void Player::timerCallback();

    void sliderValueChanged(juce::Slider* slider) override;

    std::unique_ptr<juce::AudioFormatReaderSource> playSource;
    juce::AudioTransportSource transport;
    juce::ResamplingAudioSource resampledSource;

    void setNextPlayer(bool trueOrFalse);

    int sampleRate = 0;

    void Player::handleMidiMessage(int midiMessageNumber, int midiMessageValue);
    bool fileLoaded = false;
    bool isPlaying = false;

    void Player::play();
    void Player::stop();
    void playButtonClicked();
    void stopButtonClicked();

    juce::Slider volumeSlider;

    void Player::setLabelPlayerPosition(int playerPosition);

    //FILE
    void Player::verifyAudioFileFormat(const juce::String& path);
    void Player::loadFile(const juce::String& path);

    bool fader1IsPlaying = false;
    bool fader2IsPlaying = false;
    int totalPlayerWidth;

    std::string Player::getFilePath();
    float Player::getTrimVolume();
    void Player::setTrimVolume(double trimVolume);
    bool Player::getIsLooping();
    void Player::setIsLooping(bool isLooping);




private:
    enum TransportState
    {
        Stopped ,
        Starting,
        Stopping,
        Playing
    };

    TransportState state;
    TransportState newState;

    

    int actualSamplesPerBlockExpected;
    double actualSampleRate;

    //juce::MessageManagerLock messageManager;

    int playerIndex;

    
    //WAVEFORM
    juce::AudioThumbnailCache thumbnailCache;
    juce::AudioThumbnail thumbnail;

    int leftControlsWidth = 50;
    int rightControlsWidth = 150;
    int playStopButtonWidth = rightControlsWidth / 3;
    int openDeleteButtonWidth = rightControlsWidth / 2;
    int borderRectangleWidth = 10;
    int borderRectangleHeight = 100;
    int volumeSliderWidth = 50;
    int waveformThumbnailXStart = 50;
    int waveformThumbnailXSize = 400;
    int waveformThumbnailXEnd = waveformThumbnailXStart + waveformThumbnailXSize;
    float thumbnailZoomValue = 1.0;
    
    int rightControlsStart = leftControlsWidth + borderRectangleWidth + waveformThumbnailXSize;
    


    juce::Rectangle<int> thumbnailBounds;

    //WAVEFORMCONTROL
    int waveformMousePosition;
    juce::Point<int>componentMousePositionXY;
    int componentMousePositionX;  
    int mouseDragXPosition;
    int mouseDragRelativeXPosition;
    float mouseDragInSeconds;
    bool waveformPainted = false;

    bool isNextPlayer = false;




    //AUDIO
    juce::AudioFormatManager formatManager;
    bool looping = false;
    bool stopButtonClickedBool = false;
    //GUI
    juce::Label playerPositionLabel;

    juce::TextButton openButton;
    juce::TextButton playButton;
    juce::TextButton stopButton;
    juce::TextButton cueButton;
    juce::TextButton deleteButton;
    juce::ToggleButton loopButton;

    juce::Slider transportPositionSlider;
    //juce::Slider volumeSlider;
    juce::Label volumeLabel;

    juce::Slider trimVolumeSlider;


    juce::Label remainingTimeLabel;
    juce::Label trimLabel;
    juce::Label soundName;
    juce::Label playerIDLabel;



    //bool Player::isInterestedInFileDrag(const juce::StringArray& files) override;
    //void Player::filesDropped(const juce::StringArray& files, int x, int y) override;

    void Player::deleteFile();
    std::string loadedFilePath;


    void openButtonClicked();
    
    void cueButtonClicked();

    //void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;

    void transportStateChanged(TransportState);

    void Player::thumbnailChanged();
    void Player::paintIfNoFileLoaded(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds);
    void Player::paintIfFileLoaded(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds, float thumbnailZoomValue);
    void Player::paintPlayHead(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds);
    void Player::updateLoopButton(juce::Button* button, juce::String name);

    void Player::updateRemainingTime();

    void changeListenerCallback(juce::ChangeBroadcaster* source);

    void Player::mouseDown(const juce::MouseEvent &event);
    void Player::mouseDrag(const juce::MouseEvent &event);



    float floatMidiMessageValue;
    int previousMidiLevel = 0;
    int actualMidiLevel = 0;

    //juce::File preloadedFile;

    const juce::String Player::startFFmpeg(std::string filePath);
    void Player::handleAudioTags(std::string filePath);


    inline bool exists_test(const std::string& name) {
        if (FILE* file = fopen(name.c_str(), "r")) {
            fclose(file);
            return true;
        }
        else {
            return false;
        }
    }


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Player)
};


