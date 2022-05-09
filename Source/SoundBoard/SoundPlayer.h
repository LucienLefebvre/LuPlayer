/*
  ==============================================================================

    SoundPlayer.h
    Created: 15 Mar 2021 2:18:44pm
    Author:  Lucien

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "Player.h"
#include "Playlist.h"

#include "../Settings/Settings.h"
#include <ff_meters\ff_meters.h>
#include <Ebu128LoudnessMeter.h>
#include "../Others/LoudnessBar.h"
#include "../Mixer/Meter.h"
#include "KeyboardMappedSoundboard.h"
#include "../Settings/KeyMapper.h"
#include "../Settings/MidiMapper.h"
#include "../Mixer/Meter.h"
#include "../Others/StopWatch.h"

//==============================================================================
class SoundPlayer : public juce::Component,
                    public juce::ChangeListener,
                    public juce::Timer,
                    public juce::Value::Listener,
                    private juce::OSCReceiver,
                    private juce::OSCReceiver::ListenerWithOSCAddress<juce::OSCReceiver::MessageLoopCallback>,
                    private juce::OSCReceiver::Listener<juce::OSCReceiver::MessageLoopCallback>,
                    public juce::ApplicationCommandTarget
{
public:
    enum  class Mode : int
    {
        OnePlaylistOneCart = 1,
        EightFaders,
        KeyMap
    };
    enum CommandIDs
    {
        cuePlay = 100,
        cueStart,
        cueEnd,
        toggleClipEffects,
        toggleClipEnveloppe,
        toggleClipLooping,
        toggleHPF,
        upOneDb,
        downOneDb,
        dummy
    };
    SoundPlayer(SoundPlayer::Mode m, Settings* s);
    ~SoundPlayer() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill, const juce::AudioSourceChannelInfo& cueBuffer);

    Player* getActivePlayer();
    void playPlayer(int playerID);

    void setSettings(Settings* s);

    bool handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message, MidiMapper* mapper);
    void handleIncomingMidiMessageEightPlayers(juce::MidiInput* source, const juce::MidiMessage& message);
    void OSCInitialize();
    void oscMessageReceived(const juce::OSCMessage& message);
    void handleOSCKeyMap(const juce::OSCMessage& message);
    void handleOSCEightFaders(const juce::OSCMessage& message);
    void handleOSCPlaylist(const juce::OSCMessage& message);

    void metersInitialize();

    void savePlaylist();
    juce::XmlElement* createPlayerXmlElement(int player, int playlist, juce::XmlElement* e);
    void loadPlaylist();
    void SoundPlayer::positionViewport(int player);
    void SoundPlayer::updateDraggedPlayerDisplay(int playerDragged, int playlistDragged);

    juce::MixerAudioSource myMixer;
    juce::MixerAudioSource myCueMixer;
    juce::OwnedArray<Playlist> myPlaylists;
    std::unique_ptr<KeyboardMappedSoundboard> keyMappedSoundboard;
    //METERS
    foleys::LevelMeterLookAndFeel lnf;
    foleys::LevelMeter meter{ foleys::LevelMeter::MaxNumber };
    foleys::LevelMeterSource meterSource;
    foleys::LevelMeter cuemeter{ foleys::LevelMeter::MaxNumber };
    foleys::LevelMeterSource cuemeterSource;
    Ebu128LoudnessMeter loudnessMeter;
    Ebu128LoudnessMeter cueloudnessMeter;
    juce::Viewport playlistViewport;
    juce::Viewport playlistbisViewport;
    std::unique_ptr<Meter> newMeter;

    //WAVEFORM CONTROL
    int draggedPlayer = 0;
    int draggedPlaylist = 0;
    bool oscConnected = false;

    void loadXMLElement(juce::XmlElement* e, int playerID, int playlistID);

    void SoundPlayer::setTimerTime(int timertime);
    void SoundPlayer::setEightPlayersMode(bool isEightPlayers);

    juce::ChangeBroadcaster* playerSelectionChanged;
    juce::ChangeBroadcaster* playlistLoadedBroadcaster;

    SoundPlayer::Mode soundPlayerMode;
    void initializeKeyMapPlayer();
    SoundPlayer::Mode getSoundPlayerMode();
    bool isPlaying();

    juce::ApplicationCommandTarget* getNextCommandTarget();
    void getAllCommands(juce::Array<juce::CommandID>& commands);
    void getCommandInfo(juce::CommandID commandID, juce::ApplicationCommandInfo& result);
    bool perform(const InvocationInfo& info);

    StopWatch mainStopWatch;
private:
    void timerCallback();
    void valueChanged(juce::Value& value);
    void OSCClean();

    void OSCsend(int destination, float value);
    void OSCsend(int destination, juce::String string);
    void mouseDragGetInfos(int playlistSource, int playerID);
    void drawDragLines();
    void mouseDragDefinePlayer();
    bool isDraggable(int playlistSource, int playerSource, int playlistDestination, int playerDestination);
    void mouseDragSetInfos(int playlistDestination, int playerIdDestination);
    void setSoundInfos(int playlistDestination, int playerIdDestination);
    void clearDragInfos();
    void mouseDragEnd(int playlistDestination);
    void checkAndRemovePlayer(int playlist, int player);
    void reassignFaders(int playerIdDestination, int playerDragSource, int playlistDestination);
    void copyPlayingSound();
    void changeListenerCallback(juce::ChangeBroadcaster* source);
    //SAVE
    juce::File myFile;
    double loadingProgress = 0;

    double actualSampleRate;
    int actualSamplesPerBlockExpected;

    //GRAPHIC
    int playersStartHeightPosition = 10;
    int playerHeight = 100;
    int playerWidth = 670;
    int spaceBetweenPlayer = 5;
    int playerInsertDragZoneHeight = 20;
    int controlButtonWidth = 25;
    int borderRectangleWidth = 10;
    int borderRectangleHeight = 80;
    int totalPlayerWidth = playerWidth;
    int totalPlayerWidthWithButtons = totalPlayerWidth + controlButtonWidth;
    int spaceBetwennPlaylistAndControls = 30;
    int upDownButtonsWidth = 100;
    int upDownButtonsHeight = 0;
    int cartsStartX = 800;
    int defaultWindowWidth = 1560;
    int bottomButtonsHeight = 0;
    int meterWidth = 80;
    int loudnessBarWidth = 25;

    //METERS
    int levelMeterMaximumHeight = 510;
    int levelMeterHeight;
    int cuelevelMeterMaximumHeight = 300;
    int cuelevelMeterMinimumHeight = 150;
    int cuelevelMeterHeight;

    int meterNumericDisplayHeight = 28;
    float shortTermLoudness = 0;
    float cueShortTermLoudness = 0;

    Meter mainMeter{Meter::Mode::Stereo};
    LoudnessBar loudnessBarComponent;

    //Buttons
    juce::TextButton playersResetPosition;
    juce::TextButton playersPreviousPosition;
    juce::TextButton playersNextPosition;
    juce::TextButton addPlayerPlaylist;
    juce::TextButton removePlayerPlaylist;
    juce::TextButton addPlayerCart;
    juce::TextButton removePlayerCart;

    //Label
    juce::Label timeLabel;
    int timeLabelWidth = 120;
    int timeLabelHeight = 35;
    //MIDI
    int midiMessageNumber;
    int midiMessageValue;

    //OSC
    juce::OSCSender* sender;
    juce::OSCReceiver receiver;
    juce::String ipAdress;
    int outPort;
    int inPort = -1;
    int currentPortNumber = -1;
    juce::int64 start1;
    juce::int64 delta1;
    juce::int64 start2;
    juce::int64 delta2;
    juce::int64 start3;
    juce::int64 delta3;
    juce::int64 start4;
    juce::int64 delta4;
    bool fader1OSCLaunched = false;
    bool fader2OSCLaunched = false;
    int deltaTime = 200;

    //DRAG AND DROP
    bool destinationPlayerFound = false;
    int dragZoneHeight = 25;
    int playerMouseDrag;
    int playerMouseDragUp = -1;
    int playlistDragSource = 0;
    int playlistDragDestination;
    int playerToLoad;
    int playerDragSource;
    int playerDragDest;
    int mouseDragX;
    int mouseDragY;
    int mouseDragged;
    int dragZoneWidth = 20;
    bool insertTop = false;

    //Dragged sound infos
    std::string draggedPath;
    float draggedTrim;
    bool draggedLooping;
    std::string draggedName;
    bool draggedHpfEnabled;
    bool draggedStartTimeSet;
    bool draggedStopTimeSet;
    float draggedStartTime;
    float draggedStopTime;
    int playerSourceLatch;
    int playlistSourceLatch;
    bool draggedNormalized;
    FilterProcessor::GlobalParameters draggedFilterParameters{FilterProcessor::makeDefaultFilter()};
    juce::Array<float> draggedEnveloppeXArray;
    juce::Array<float> draggedEnveloppeYArray;
    bool draggedEnveloppeEnabled;
    bool draggedFxBypassed;
    juce::Colour draggedColour;
    bool draggedColourChanged;

    bool isEightPlayerMode = false;
    int stopWatchHeight = 25;

    Settings* settings;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoundPlayer)
};
