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
//#include "Settings.h"
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"
#include "PlayHead.h"
#include "nanodbc/nanodbc.h"

typedef char* LPSTR;



//==============================================================================
/*
*/
class Player :  public juce::Component,
                private juce::ChangeListener,
                public juce::Slider::Listener,
                public juce::Timer,
                public juce::MouseListener,
                //public juce::KeyListener,
                public juce::Button::Listener,
                public juce::Value::Listener
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
    juce::ChannelRemappingAudioSource channelRemappingSource;
    juce::IIRFilterAudioSource filterSource;

    std::unique_ptr<juce::AudioFormatReaderSource> cuePlaySource;
    juce::AudioTransportSource cueTransport;
    juce::ResamplingAudioSource cueResampledSource;
    juce::ChannelRemappingAudioSource cuechannelRemappingSource;

    juce::IIRFilterAudioSource cuefilterSource;

    juce::MixerAudioSource mixer;
    juce::MixerAudioSource cueMixer;

    void setNextPlayer(bool trueOrFalse);


    void Player::handleMidiMessage(int midiMessageNumber, int midiMessageValue);
    void Player::handleOSCMessage(float faderValue);
    void Player::handleMidiTrimMessage(int midiMessageValue);
    bool fileLoaded = false;
    bool isPlaying = false;

    void Player::play();
    void Player::stop();
    void playButtonClicked();
    void stopButtonClicked();
    void Player::spaceBarPlay();
    void Player::deleteFile();

    juce::Slider volumeSlider;
    juce::Label volumeLabel;

    void Player::setLabelPlayerPosition(int playerPosition);

    //FILE
    void Player::verifyAudioFileFormat(const juce::String& path);
    bool Player::loadFile(const juce::String& path);

    bool fader1IsPlaying = false;
    bool fader2IsPlaying = false;
    bool keyIsPlaying = false;
    float sliderValueToset;
    float trimValueToSet = 1.0;
    float trimValueInput;
    int totalPlayerWidth;

    std::string Player::getFilePath();


    float Player::getTrimVolume();
    void Player::setTrimVolume(double trimVolume);
    bool Player::getIsLooping();
    void Player::setIsLooping(bool isLooping);
    std::string Player::getName();
    void Player::setName(std::string Name);
    void Player::enableHPF(bool shouldBeEnabled);
    bool Player::isHpfEnabled();


    void Player::setStart();
    void Player::deleteStart();
    void Player::setStop();
    void Player::deleteStop();
    void Player::stopTimeClicked();
    void Player::setTimeClicked();

    float Player::getStart();
    float Player::getStop();
    void Player::setStartTime(float startTime);
    void Player::setStopTime(float stopTime);
    void Player::setActivePlayer(bool isActive);

    void Player::setEightPlayerMode(bool isEight);
    bool isEightPlayerMode = false;
    bool startTimeSet = false;
    bool stopTimeSet = false;

    juce::Value draggedPlayer;

    void Player::setOptions();
    //void Player::setOptions(juce::String FFmpegPath, juce::String sExiftoolPath, juce::String convertedFilesPath, float skewFactor, int maxFaderValue);
    std::string FFmpegPath;
    std::string convertedFilesPath;
    std::string ExiftoolPath;
    float skewFactor;
    float maxFaderValue = 1.0;


    juce::ToggleButton loopButton;

    void Player::setLeftFaderAssigned(bool isFaderLeftAssigned);
    void Player::setRightFaderAssigned(bool isFaderRightAssigned);
    bool faderLeftAssigned = false;
    bool faderRightAssigned = false;

    bool isCart = false;

    juce::Value volumeSliderValue;
    juce::String remainingTimeString;
    juce::Label soundName;
    juce::Value fileName;
    juce::Value cueStopped;

    void Player::assignNextPlayer();
    juce::Value nextPlayerAssigned;
    juce::TextButton playerPositionLabel;
    
    float startTime = 0;
    float stopTime;

    void Player::buttonClicked(juce::Button* button) override;
    void Player::buttonStateChanged(juce::Button* b) override;
    void Player::cueButtonClicked();

    void Player::setTimerTime(int timertime);

    PlayHead playPlayHead;
    PlayHead cuePlayHead;
    PlayHead inMark;
    PlayHead outMark;

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

    bool rightClickDown = false;

    int actualSamplesPerBlockExpected;
    double actualSampleRate;

    //juce::MessageManagerLock messageManager;

    int playerIndex;
 
    //WAVEFORM
    juce::AudioThumbnailCache thumbnailCache;
    juce::AudioThumbnail thumbnail;
    bool endRepainted = false;

    int leftControlsWidth = 50;
    int rightControlsWidth = 150;
    int playStopButtonWidth = rightControlsWidth / 2;
    int openDeleteButtonWidth = rightControlsWidth / 2;
    int borderRectangleWidth = 10;
    int borderRectangleHeight = 100;
    int volumeSliderWidth = 48;
    int waveformThumbnailXStart = leftControlsWidth + borderRectangleWidth;
    int waveformThumbnailXSize = 400;
    int waveformThumbnailXEnd = waveformThumbnailXStart + waveformThumbnailXSize;
    float thumbnailZoomValue = 1.0;
    int optionButtonWidth = 20;
    
    int rightControlsStart = leftControlsWidth + borderRectangleWidth + waveformThumbnailXSize;
    int volumeLabelStart = rightControlsStart + rightControlsWidth;

    int cueStopButtonsSize = 15;

    bool volumeSliderIsClicked = false;


    juce::Rectangle<int> thumbnailBounds;

    //WAVEFORMCONTROL
    int waveformMousePosition;
    juce::Point<int>componentMousePositionXY;
    int componentMousePositionX;  
    int mouseDragXPosition;
    int mouseDragRelativeXPosition;
    float mouseDragInSeconds;
    bool waveformPainted = false;
    float thumbnailHorizontalZoom = 1;
    float thumbnailOffset = 0;
    int  thumbnailDragStart = 0;

    float thumbnailDrawStart;
    float thumbnailDrawEnd;
    float thumbnailDrawSize = 0;
    float thumbnailMiddle;
    float drawPlayHeadPosition;

    bool isNextPlayer = false;

    int playlistButtonsControlWidth = 25;
    int cartButtonsControlWidth = 30;

    int dragZoneWidth = 40;
    //AUDIO
    juce::AudioFormatManager formatManager;
    bool looping = false;
    bool stopButtonClickedBool = false;
    bool stopCueTransportOut = false;
    //GUI


    juce::TextButton openButton;
    juce::TextButton playButton;
    juce::TextButton stopButton;
    juce::TextButton cueButton;
    juce::TextButton cueStopButton;
    double fileSampleRate;
    juce::TextButton deleteButton;

    juce::TextButton startTimeButton;
    juce::TextButton stopTimeButton;

    juce::TextButton optionButton;
    juce::Slider filterFrequencySlider;
    bool hpfEnabled = false;
    double filterFrequency = 100;
    juce::IIRCoefficients filterCoefficients;


    int playerPosition = 0;

    juce::Slider transportPositionSlider;
    //juce::Slider volumeSlider;


    juce::Slider trimVolumeSlider;


    juce::Label remainingTimeLabel;
    juce::Label trimLabel;

    juce::Label playerIDLabel;

   // bool Player::keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent);

    //bool Player::isInterestedInFileDrag(const juce::StringArray& files) override;
    //void Player::filesDropped(const juce::StringArray& files, int x, int y) override;


    std::string loadedFilePath;
    std::string newName;



    void Player::openButtonClicked();
    void Player::cueStopButtonClicked();
    void Player::optionButtonClicked();
    void Player::setStereoMapping(int mappingSelected);
    void Player::setFilter(int filterSelected);
    void Player::refreshPopupMenu();
    void Player::setChannelsMapping();
    float monoReductionGain;

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
    void Player::mouseUp(const juce::MouseEvent& event);
    void Player::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel);
    void Player::mouseDoubleClick(const juce::MouseEvent& event) override;
    void Player::repaintThumbnail();
    void Player::calculThumbnailBounds();

    juce::String Player::secondsToMMSS(int seconds);

    float oldThumbnailOffset = 0;
    juce::Label cueTimeLabel;
    juce::Label elapsedTimeLabel;
    bool drawCue = false;


    float floatMidiMessageValue = 0;
    int previousMidiLevel = 0;
    int actualMidiLevel = 0;

    void Player::valueChanged(juce::Value& value);


    //juce::File preloadedFile;

    const juce::String Player::startFFmpeg(std::string filePath);
    void Player::handleAudioTags(std::string filePath);

    std::string Player::extactName(std::string Filepath);



    inline bool exists_test(const std::string& name) {
        if (FILE* file = fopen(name.c_str(), "r")) {
            fclose(file);
            return true;
        }
        else {
            return false;
        }
    }
    std::wstring Player::s2ws(const std::string& s)
    {
        int len;
        int slength = (int)s.length() + 1;
        len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
        wchar_t* buf = new wchar_t[len];
        MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
        std::wstring r(buf);
        delete[] buf;
        return r;
    }

    std::wstring utf8_to_utf16(const std::string& utf8)
    {
        std::vector<unsigned long> unicode;
        size_t i = 0;
        while (i < utf8.size())
        {
            unsigned long uni;
            size_t todo;
            bool error = false;
            unsigned char ch = utf8[i++];
            if (ch <= 0x7F)
            {
                uni = ch;
                todo = 0;
            }
            else if (ch <= 0xBF)
            {
                throw std::logic_error("not a UTF-8 string");
            }
            else if (ch <= 0xDF)
            {
                uni = ch & 0x1F;
                todo = 1;
            }
            else if (ch <= 0xEF)
            {
                uni = ch & 0x0F;
                todo = 2;
            }
            else if (ch <= 0xF7)
            {
                uni = ch & 0x07;
                todo = 3;
            }
            else
            {
                throw std::logic_error("not a UTF-8 string");
            }
            for (size_t j = 0; j < todo; ++j)
            {
                if (i == utf8.size())
                    throw std::logic_error("not a UTF-8 string");
                unsigned char ch = utf8[i++];
                if (ch < 0x80 || ch > 0xBF)
                    throw std::logic_error("not a UTF-8 string");
                uni <<= 6;
                uni += ch & 0x3F;
            }
            if (uni >= 0xD800 && uni <= 0xDFFF)
                throw std::logic_error("not a UTF-8 string");
            if (uni > 0x10FFFF)
                throw std::logic_error("not a UTF-8 string");
            unicode.push_back(uni);
        }
        std::wstring utf16;
        for (size_t i = 0; i < unicode.size(); ++i)
        {
            unsigned long uni = unicode[i];
            if (uni <= 0xFFFF)
            {
                utf16 += (wchar_t)uni;
            }
            else
            {
                uni -= 0x10000;
                utf16 += (wchar_t)((uni >> 10) + 0xD800);
                utf16 += (wchar_t)((uni & 0x3FF) + 0xDC00);
            }
        }
        return utf16;
    }


    std::string exec(const char* cmd) {
        std::array<char, 128> buffer;
        std::string result;

        std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd, "r"), _pclose);
        if (!pipe) {
            throw std::runtime_error("popen() failed!");
        }
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        return result;
    }

    float previousSliderValue;
    float actualSliderValue;

    bool playButtonHasBeenClicked = false;

    bool trimSliderRejoignedValue = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Player)
};


