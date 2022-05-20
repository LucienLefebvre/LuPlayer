/*
  ==============================================================================

    Player.h
    Created: 26 Jan 2021 7:36:59pm
    Author:  Lucien

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

#include "../Others/PlayHead.h"
#include "../Others/R128IntegratedThread.h"
#include "../Mixer/FilterProcessor.h"
#include "../Mixer/CompProcessor.h"
#include "../Mixer/Meter.h"
#include "../Others/gainThumbnail.h"
#include "../Settings/Settings.h"
#if RFBUILD
#include "../RF/Denoiser.h"
#include "../RF/ffmpegConvert.h"
#include "../RF/convertObject.h"
#endif

//==============================================================================
class Player : public juce::Component,
    private juce::ChangeListener,
    public juce::Slider::Listener,
    public juce::MultiTimer,
    public juce::MouseListener,
    public juce::Button::Listener,
    public juce::Value::Listener,
    public juce::ChangeBroadcaster,
    public juce::ActionBroadcaster,
    public juce::Label::Listener
{
public:
    struct PlayerInfo
    {
        juce::String filePath = "";
        juce::String name = "";
        double volume = 0.0;
        double trimVolume = 0.0;
        bool hasBeenNormalized = false;
        bool loop = false;
        bool hpfEnabled = false;
        bool startTimeSet = false;
        bool stopTimeSet = false;
        float startTime = 0.0;
        float stopTime = 0.0;
        int playMode = 0;
        bool fxBypassed = true;
        FilterProcessor::GlobalParameters filterParams;
        CompProcessor::CompParameters compParams;
        bool enveloppeEnabled = false;
        juce::Path enveloppePath;
        juce::Colour playerColour;
        bool colourHasChanged;
        juce::KeyPress shortcut;
    };

    Player(int index, Settings* s);
    ~Player() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill, const juce::AudioSourceChannelInfo& cue);
    std::atomic<float> getEnveloppeValue(float x, juce::Path& p);
    void playerPrepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void timerCallback(int timerID);
    void sliderValueChanged(juce::Slider* slider) override;

    void handleMidiMessage(int midiMessageNumber, int midiMessageValue);
    void handleOSCMessage(float faderValue);
    void handleMidiTrimMessage(int midiMessageValue);
    void handleMidiTrimMessage(bool upordown);

    void setLeftFaderAssigned(bool isFaderLeftAssigned);
    void setRightFaderAssigned(bool isFaderRightAssigned);
    void assignNextPlayer();

    void play(bool launchedByMidi = false);
    void launch();
    void stop();
    void playButtonClicked();
    void stopButtonClicked();
    void spaceBarPlay();
    void deleteFile();

    void setNextPlayer(bool trueOrFalse);
    void setPlayerIndex(int i);
    void setOSCIndex(int i);

    void setLabelPlayerPosition(int playerPosition);
    void setCuePlayHeadVisible(bool isVisible);

    void verifyAudioFileFormat(const juce::String& path);
    bool loadFile(const juce::String& path, bool shouldSendChangeMessage);
    Player::PlayerInfo getPlayerInfo();
    void setPlayerInfo(Player::PlayerInfo p);
    void isDraggedOver(bool b);
    void setSortcut(juce::KeyPress s);
    juce::KeyPress getShortcut();
    std::string getFilePath();

    void setGain(float g);
    void nudgeGain(float g);
    float getVolume();
    float getTrimVolume();
    void setTrimVolume(double trimVolume);
    bool getIsLooping();
    void setIsLooping(bool isLooping, bool shouldSendMessage = false);
    std::string getName();
    bool getHasBeenNormalized();
    void setHasBeenNormalized(bool b);
    void setName(std::string Name, bool sendMessage = false);
    bool isStartTimeSet();
    bool isStopTimeSet();
    void enableHPF(bool shouldBeEnabled, bool shouldSendMessage = true);
    bool isHpfEnabled();
    void updateLoopButton(juce::Button* button, juce::String name);

    void setStart();
    void deleteStart(bool shouldSendMessage = true);
    void setStop();
    void deleteStop(bool shouldSendMessage = true);
    void stopTimeClicked();
    void setTimeClicked();

    float getStart();
    float getStop();
    void setStartTime(float startTime, bool shouldSendMessage = true);
    void setStopTime(float stopTime, bool shouldSendMessage = true);
    void setActivePlayer(bool isActive);

    void setEightPlayerMode(bool isEight);

    void setOptions();

    void buttonClicked(juce::Button* button) override;
    void buttonStateChanged(juce::Button* b) override;
    void cueButtonClicked();

    void setTimerTime(int timertime);

    FilterProcessor& getFilterProcessor();
    std::unique_ptr<juce::AudioBuffer<float>>& getBuffer();
    FilterProcessor::GlobalParameters getFilterParameters();
    CompProcessor::CompParameters getCompParameters();
    void setCompParameters(CompProcessor::CompParameters p);
    void setFilterParameters(FilterProcessor::GlobalParameters g);
    void setFilterParameters(int i, FilterProcessor::FilterParameters p);

    Meter& getInputMeter();
    Meter& getOutputMeter();
    Meter& getCompMeter();

    void setDraggedPlayer();

    void fxButtonClicked();
    void envButtonClicked(bool enable = true);
    juce::TextButton* getfxButton();

    void bypassFX(bool isBypassed, bool shouldSendMessage = true);
    bool getBypassed();

    void normButtonClicked();
    void normalize(std::string p);

    void setEditedPlayer(bool isEdited);

    void killThreads();
    bool isPlayerPlaying();

    bool isLastSeconds();
    bool isFileLoaded();

    juce::AudioThumbnailCache& getAudioThumbnailCache();
    juce::AudioFormatManager& getAudioFormatManager();
    GainThumbnail& getAudioThumbnail();
    GainThumbnail& getPlayThumbnail();

    juce::String getRemainingTimeAsString();
    juce::String getElapsedTimeAsString();
    juce::String getCueTimeAsString();

    void setEnveloppePath(juce::Path& p);
    void createDefaultEnveloppePath();
    juce::Path* getEnveloppePath();
    bool isEnveloppeEnabled();
    void setEnveloppeEnabled(bool isEnabled, bool shouldSendMessage = true, bool switchToEnveloppePanel = false);

    bool getIsCart();
    void setIsCart(bool b);

    bool isFxEnabled();
    void enableButtons(bool isEnabled);

    bool isEditedPlayer();
    double getLenght();

    juce::Array<float> getEnveloppeXArray();
    juce::Array<float> getEnveloppeYArray();
    static juce::Path createEnveloppePathFromArrays(juce::Array<float> xArray, juce::Array<float> yArray);

    juce::String secondsToMMSS(int seconds);

    bool isThreadRunning();
    void openButtonClicked();

    void updateCuePlayHeadPosition(bool forceUpdate = false);
    void updatePlayHeadPosition();
    void updateInOutMarkPosition();

#if RFBUILD
    void denoiseButtonClicked();
    void setDenoisedFile(bool loadDenoisedFile);
    bool getDenoisedFileLoaded();
    std::string FFmpegPath;
#endif

    void setPlayerColour(juce::Colour c, bool sendMessage = true);
    juce::Colour getPlayerColour();
    bool getColourHasChanged();

    void setPlayMode(int m);
    int getPlayMode();

    double CalculateR128Integrated(std::string filePath);

    void setIsSecondCart(bool t);

    std::unique_ptr<juce::AudioFormatReaderSource> playSource;
    juce::AudioTransportSource transport;
    juce::ChannelRemappingAudioSource channelRemappingSource        { &filterSource, false };
    juce::IIRFilterAudioSource filterSource                         { &transport, false };
    std::unique_ptr<juce::AudioSourceChannelInfo> playerSource;

    std::unique_ptr<juce::AudioFormatReaderSource> cuePlaySource;
    juce::AudioTransportSource cueTransport;
    juce::ChannelRemappingAudioSource cuechannelRemappingSource     { &cuefilterSource, false };
    juce::IIRFilterAudioSource cuefilterSource                      { &cueTransport, false };
    std::unique_ptr<juce::AudioSourceChannelInfo> cuePlayerSource;

    bool fileLoaded = false;
    bool isPlaying = false;

    juce::Slider volumeSlider;
    juce::Label volumeLabel;

    bool fader1IsPlaying = false;
    bool fader2IsPlaying = false;
    bool keyIsPlaying = false;
    float sliderValueToset;
    float trimValueToSet = 0.0;
    float trimValueInput;
    int totalPlayerWidth;

    bool isEightPlayerMode = false;
    bool startTimeSet = false;
    bool stopTimeSet = false;
    float startTime = 0;
    float stopTime;
    double stopTimePosition = 0;

    juce::Value draggedPlayer;

    std::string convertedFilesPath;
    std::string ExiftoolPath;
    float skewFactor;
    float maxFaderValue = 1.0;

    juce::ToggleButton loopButton;

    bool faderLeftAssigned = false;
    bool faderRightAssigned = false;

    bool isCart = false;

    juce::Value volumeSliderValue;
    juce::String remainingTimeString = "";
    juce::String cueTimeString = "";
    juce::Label soundName;
    juce::Value fileName;
    juce::Value cueStopped;

    juce::Value nextPlayerAssigned;
    juce::TextButton playerPositionLabel;
    
    PlayHead playPlayHead;
    PlayHead cuePlayHead;
    PlayHead inMark;
    PlayHead outMark;

    juce::ActionBroadcaster* playBroadcaster;
    juce::ActionBroadcaster* cueBroadcaster;
    juce::ActionBroadcaster* draggedBroadcaster;
    juce::ChangeBroadcaster* fxButtonBroadcaster;
    juce::ChangeBroadcaster* envButtonBroadcaster;
    juce::ChangeBroadcaster* remainingTimeBroadcaster;
    juce::ChangeBroadcaster* trimValueChangedBroacaster;
    juce::ChangeBroadcaster* soundEditedBroadcaster;
    std::unique_ptr<juce::ChangeBroadcaster> enveloppePathChangedBroadcaster;

    std::unique_ptr<juce::MemoryAudioSource> outputSource;
    std::unique_ptr<juce::MemoryAudioSource> cueOutputSource;

    FilterProcessor filterProcessor;
    FilterProcessor cueFilterProcessor;
    CompProcessor compProcessor{CompProcessor::Mode::Stereo};
    CompProcessor cueCompProcessor{ CompProcessor::Mode::Stereo };

    juce::ChangeBroadcaster* playerInfoChangedBroadcaster;
    juce::ChangeBroadcaster* playerDeletedBroadcaster;
    std::unique_ptr<juce::ChangeBroadcaster> playerLaunchedBroadcaster;
    std::unique_ptr<juce::ChangeBroadcaster> normalizationLaunchedBroadcaster = std::make_unique<juce::ChangeBroadcaster>();
    std::unique_ptr<juce::ChangeBroadcaster> normalizationFinishedBroadcaster = std::make_unique<juce::ChangeBroadcaster>();
    std::unique_ptr<juce::ChangeBroadcaster> conversionLaunchedBroadcaster = std::make_unique<juce::ChangeBroadcaster>();
    std::unique_ptr<juce::ChangeBroadcaster> conversionFinishedBroadcaster = std::make_unique<juce::ChangeBroadcaster>();
    std::unique_ptr<juce::ChangeBroadcaster> shortcutKeyChanged = std::make_unique<juce::ChangeBroadcaster>();

    foleys::LevelMeterSource meterSource;
    foleys::LevelMeterSource outMeterSource;

    R128IntegratedThread luThread{ "Thread" };

private:
    enum TransportState
    {
        Stopped ,
        Starting,
        Stopping,
        Playing
    };

    void cueStopButtonClicked();
    void optionButtonClicked();
    void setStereoMapping(int mappingSelected);
    void setFilter(int filterSelected);
    void refreshPopupMenu();
    void setChannelsMapping();

    void transportStateChanged(TransportState);

    void thumbnailChanged();
    void paintIfNoFileLoaded(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds);
    void paintIfFileLoaded(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds, float thumbnailZoomValue);
    void paintPlayHead(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds);

    void updateRemainingTime();

    void changeListenerCallback(juce::ChangeBroadcaster* source);

    void mouseDown(const juce::MouseEvent& event);
    void mouseDrag(const juce::MouseEvent& event);
    void mouseUp(const juce::MouseEvent& event);
    void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel);
    void mouseDoubleClick(const juce::MouseEvent& event) override;
    void repaintThumbnail();
    void calculThumbnailBounds();
    void repaintPlayHead();
    void valueChanged(juce::Value& value);
    void handleAudioTags(std::string filePath);
    void labelTextChanged(juce::Label* labelThatHasChanged);

    juce::Point<int> getPointPosition(float x, float y);

    TransportState state;
    TransportState newState;

    bool rightClickDown = false;
    bool isActivePlayer = false;

    int actualSamplesPerBlockExpected;
    double actualSampleRate;

    int playerIndex;
    int oscIndex = 0;
    bool isSecondCart = false;
    //WAVEFORM
    juce::AudioThumbnailCache thumbnailCache{ 5 };
    GainThumbnail thumbnail          {521, formatManager, thumbnailCache};
    GainThumbnail playThumbnail      { 521, formatManager, thumbnailCache };
    juce::Rectangle<int> thumbnailBounds;

    bool endRepainted = false;

    bool isDragged = false;

    int roundCornerSize = 10;
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
    int fxButtonSize = 30;
    int normButtonSize = 40;
    int rightControlsStart = leftControlsWidth + borderRectangleWidth + waveformThumbnailXSize;
    int volumeLabelStart = rightControlsStart + rightControlsWidth;
    int cueStopButtonsSize = 15;

    bool volumeSliderIsClicked = false;

    //WAVEFORMCONTROL
    int waveformMousePosition;
    juce::Point<int>componentMousePositionXY;
    int componentMousePositionX;  
    int mouseDragXPosition;
    int mouseDragYPosition;
    int mouseDragRelativeXPosition;
    float mouseDragInSeconds;
    bool waveformPainted = false;
    float thumbnailHorizontalZoom = 1;
    float thumbnailOffset = 0;
    int  thumbnailDragStart = 0;
    bool mouseIsDragged = false;

    float thumbnailDrawStart;
    float thumbnailDrawEnd;
    float thumbnailDrawSize = 0;
    float thumbnailMiddle;
    float drawPlayHeadPosition;

    bool isNextPlayer = false;

    int playlistButtonsControlWidth = 25;
    int cartButtonsControlWidth = 30;

    int dragZoneWidth = 40;

    bool drawEnveloppe = true;
    //AUDIO
    juce::AudioFormatManager formatManager;

    bool looping = false;
    bool stopButtonClickedBool = false;
    bool stopCueTransportOut = false;

    //GUI
    juce::TextButton envButton;
    juce::TextButton fxButton;
    juce::TextButton normButton;

    juce::TextButton openButton     { "open" };
    juce::TextButton playButton     { "Play" };
    juce::TextButton stopButton     { "Stop" };
    juce::TextButton cueButton      { "cue" };
    juce::TextButton cueStopButton  { "6s" };
    juce::TextButton deleteButton   { "Delete" };

    juce::TextButton startTimeButton;
    juce::TextButton stopTimeButton;

    double fileSampleRate;

    juce::TextButton optionButton;
    juce::Slider filterFrequencySlider;
    bool hpfEnabled = false;
    double filterFrequency = 100;
    bool trimIsHpfFrequency = false;
    juce::IIRCoefficients filterCoefficients;

    int playerPosition = 0;

    juce::Slider transportPositionSlider;
    juce::Slider trimVolumeSlider;
    juce::Label remainingTimeLabel;
    juce::Label trimLabel;
    juce::Label playerIDLabel;
    juce::Label cueTimeLabel;
    juce::Label elapsedTimeLabel;

    std::string loadedFilePath;
    std::string newName;

    float monoReductionGain;

    float oldThumbnailOffset = 0;

    bool drawCue = false;

    float floatMidiMessageValue = 0;
    int previousMidiLevel = 0;
    int actualMidiLevel = 0;


    double integratedLoudness = 0.0;
    juce::Label normalizingLabel;
    bool hasBeenNormalized = false;

    float previousSliderValue;
    float actualSliderValue;

    bool playButtonHasBeenClicked = false;
    bool trimSliderRejoignedValue = false;

    std::unique_ptr<juce::AudioBuffer<float>> playerBuffer;
    std::unique_ptr<juce::AudioBuffer<float>> cueBuffer;

    Meter inputMeter                                        { Meter::Mode::Stereo };
    Meter outputMeter                                       { Meter::Mode::Stereo };
    Meter compMeter                                         { Meter::Mode::Stereo_ReductionGain };


    std::unique_ptr<juce::ProgressBar> convertingBar;
    double progress = -1.0;

    bool fxEnabled = false;
    bool isEdited = false;

#if RFBUILD
    ffmpegConvert ffmpegThread{ "convertThread" };
    Denoiser denoiser;
    std::string denoisedFile;
    std::unique_ptr<juce::DialogWindow> denoiseWindow;
    bool denoisedFileLoaded = false;
#endif

    juce::Path enveloppePath;
    bool enveloppeEnabled = false;

    std::atomic<bool> shouldRepaint = false;

    std::atomic<float> bufferGain = 0.0f;

    juce::Colour playerColour;
    bool colourHasChanged = false;

    int playMode = 2;

    juce::CriticalSection lock;

    juce::KeyPress shortcutKey;

    Settings* settings;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Player)
};