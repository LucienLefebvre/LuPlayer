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
#include "Settings.h"
#include <string>
#include <iostream>
#include <ff_meters\ff_meters.h>
#include <memory>
#include <math.h>
#include <Ebu128LoudnessMeter.h>
#include "LoudnessBar.h"
#include "Mixer/Meter.h"
#include "Soundboard/KeyboardMappedSoundboard.h"
//==============================================================================
/*
*/
class SoundPlayer : public juce::Component,
                    private juce::MidiInputCallback,
                    public juce::ChangeListener,
                    private juce::KeyListener,
                    public juce::Timer,
                    public juce::Value::Listener,
                    private juce::OSCReceiver,
                    private juce::OSCReceiver::ListenerWithOSCAddress<juce::OSCReceiver::MessageLoopCallback>,
                    private juce::OSCReceiver::Listener<juce::OSCReceiver::MessageLoopCallback>,
                    public juce::ActionListener
                    //private juce::Button::Listener,
                    //public juce::DragAndDropContainer
{
public:
    enum  class Mode : int
    {
        OnePlaylistOneCart = 1,
        EightFaders = 2,
        KeyMap = 3
    };
    SoundPlayer(SoundPlayer::Mode m);
    ~SoundPlayer() override;

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill);

    void SoundPlayer::prepareToPlay(int samplesPerBlockExpected, double sampleRate);

    void paint (juce::Graphics&) override;
    void resized() override;

    void SoundPlayer::handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message);
    void SoundPlayer::handleIncomingMidiMessageEightPlayers(juce::MidiInput* source, const juce::MidiMessage& message);
    bool SoundPlayer::keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent);
    void SoundPlayer::OSCInitialize();
    void SoundPlayer::oscMessageReceived(const juce::OSCMessage& message);
    void SoundPlayer::metersInitialize();
    void SoundPlayer::savePlaylist();
    void SoundPlayer::loadPlaylist();
    void SoundPlayer::positionViewport(int player);
    void SoundPlayer::updateDraggedPlayerDisplay(int playerDragged, int playlistDragged);
    juce::MixerAudioSource myMixer;
    juce::MixerAudioSource myCueMixer;
    juce::OwnedArray<Playlist> myPlaylists;
    std::unique_ptr<KeyboardMappedSoundboard> keyMappedSoundboard;
    //METERS
    foleys::LevelMeterLookAndFeel lnf;
    foleys::LevelMeter meter{ foleys::LevelMeter::MaxNumber }; // See foleys::LevelMeter::MeterFlags for options
    foleys::LevelMeterSource meterSource;
    foleys::LevelMeter cuemeter{ foleys::LevelMeter::MaxNumber }; // See foleys::LevelMeter::MeterFlags for options
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

    void SoundPlayer::setTimerTime(int timertime);
    void SoundPlayer::setEightPlayersMode(bool isEightPlayers);

    juce::ChangeBroadcaster* playerSelectionChanged;

    SoundPlayer::Mode soundPlayerMode;
    void initializeKeyMapPlayer();
private:
    void SoundPlayer::timerCallback();
    void SoundPlayer::valueChanged(juce::Value& value);
    void SoundPlayer::OSCClean();

    void SoundPlayer::OSCsend(int destination, float value);
    void SoundPlayer::OSCsend(int destination, juce::String string);
    void SoundPlayer::mouseDragGetInfos(int playlistSource, int playerID);
    void SoundPlayer::drawDragLines();
    void SoundPlayer::mouseDragDefinePlayer();
    bool SoundPlayer::isDraggable(int playlistSource, int playerSource, int playlistDestination, int playerDestination);
    void SoundPlayer::mouseDragSetInfos(int playlistDestination, int playerIdDestination);
    void SoundPlayer::setSoundInfos(int playlistDestination, int playerIdDestination);
    void SoundPlayer::clearDragInfos();
    void SoundPlayer::mouseDragEnd(int playlistDestination);
    void SoundPlayer::checkAndRemovePlayer(int playlist, int player);
    void SoundPlayer::reassignFaders(int playerIdDestination, int playerDragSource, int playlistDestination);
    void SoundPlayer::copyPlayingSound();
    void SoundPlayer::changeListenerCallback(juce::ChangeBroadcaster* source);
    void SoundPlayer::actionListenerCallback(const juce::String& message);

    //SAVE
    juce::File myFile;
    double loadingProgress = 0;

    double actualSampleRate;
    int actualSamplesPerBlockExpected;


    //GRAPHIC
    int playersStartHeightPosition = 30;
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
    int upDownButtonsHeight = 25;
    int cartsStartX = 800;
    int defaultWindowWidth = 1560;
    int bottomButtonsHeight = 30;

    //METERS
    int levelMeterMaximumHeight = 510;
    int levelMeterHeight;
    int cuelevelMeterMaximumHeight = 300;
    int cuelevelMeterMinimumHeight = 150;
    int cuelevelMeterHeight;

    int meterNumericDisplayHeight = 28;
    float shortTermLoudness = 0;
    float cueShortTermLoudness = 0;

    //Buttons
    juce::TextButton playersResetPosition;
    juce::TextButton playersPreviousPosition;
    juce::TextButton playersNextPosition;
    juce::TextButton addPlayerPlaylist;
    juce::TextButton removePlayerPlaylist;
    juce::TextButton addPlayerCart;
    juce::TextButton removePlayerCart;

    //MIDI
    int midiMessageNumber;
    int midiMessageValue;

    //OSC

    juce::OSCSender sender;
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
    bool draggedFxBypassed;

    LoudnessBar loudnessBarComponent;

    bool isEightPlayerMode = false;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoundPlayer)
};
