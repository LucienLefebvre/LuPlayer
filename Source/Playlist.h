/*
  ==============================================================================

    Playlist.h
    Created: 10 Feb 2021 2:39:14pm
    Author:  Lucien

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <string>
#include <iostream>
#include "Player.h"
#include "Mixer/Meter.h"
#include "Settings/MidiMapper.h"
//==============================================================================
/*
*/
class Playlist  : public juce::Component,
    //private juce::MidiInputCallback,
    private juce::ChangeListener,
    public juce::FileDragAndDropTarget,
    public juce::Value::Listener,
    public juce::Button::Listener,
    public juce::MouseListener,
    public juce::Timer,
    public juce::ChangeBroadcaster,
    public juce::ActionListener,
    public juce::ActionBroadcaster,
    public juce::ApplicationCommandTarget
{
public:
    Playlist(int splaylistType);
    ~Playlist() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void rearrangePlayers();

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill);

    void assignLeftFader(int playerID);
    void assignRightFader(int playerID);
    void fader1Stop(bool stoppedByFader);
    void fader2Stop(bool stoppedByFader);

    void handleFader1(int faderValue);
    void handleFader1OSC(float faderValue);
    void handleFader2(int faderValue);
    void handleFader2OSC(float faderValue);
    void handleFader3(int faderValue);
    void handleFader3OSC(float faderValue);
    void handleFader4(int faderValue);
    void handleFader4OSC(float faderValue);

    void handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message, MidiMapper* mapper);
    void handleMidiTrim(int value, int number, MidiMapper* mapper); //normal mode
    void handleMidiRelativeTrim(int value, int number, MidiMapper* mapper);

    void handleIncomingMidiMessageEightPlayers(juce::MidiInput* source, const juce::MidiMessage& message, MidiMapper* mapper, int startIndex);
    void handleMidiTrimEightPlayers(int value, int number, MidiMapper* mapper, int startIndex);
    void handleTrimMidiMessage(int value, int number);//8 players mode

    void spaceBarPressed();
    void playersResetPositionClicked();
    void playersPreviousPositionClicked();
    void playersNextPositionClicked();

    void addPlayer(int playerID);
    void removePlayer(int playerID);
    void removeButtonClicked();

    void filesDropped(const juce::StringArray& files, int x, int y);
    void fileDragMove(const juce::StringArray& files, int x, int y);
    void fileDragEnter(const juce::StringArray& files, int x, int y);
    void fileDragExit(const juce::StringArray& files);
    void playPlayer(int playerID);
    void setDroppedSoundName(juce::String name);

    void isEightPlayerMode(bool eightPlayersMode);
    void setEightPlayersSecondCart(bool isSecondCart);
    void setOptions();
    void setMidiShift(int MidiShift);

    void mouseDrag(const juce::MouseEvent& event);
    void mouseDown(const juce::MouseEvent& event);
    void mouseUp(const juce::MouseEvent& event);
    void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel);

    void stopCues();
    void setPlaylistType(int t);
    void setPlaylistPosition(int p);
    void resetFxEditedButtons();

    bool isPlaying();


    juce::ApplicationCommandTarget* getNextCommandTarget();
    void getAllCommands(juce::Array<juce::CommandID>& commands);
    void getCommandInfo(juce::CommandID commandID, juce::ApplicationCommandInfo& result);
    bool perform(const InvocationInfo& info);

    bool eightPlayerMode = false;
    bool isEightPlayerSecondCart = false;

    juce::OwnedArray<Player> players;
    juce::OwnedArray<juce::TextButton> assignLeftFaderButtons;
    juce::OwnedArray<juce::TextButton> assignRightFaderButtons;
    juce::MixerAudioSource playlistMixer;
    juce::MixerAudioSource playlistCueMixer;
    
    double actualSampleRate;
    int actualSamplesPerBlockExpected;

    int fader1Player = 0;
    int fader2Player = 0;
    int nextPlayer = 0;
    bool spaceBarIsPlaying = false;
    int spaceBarPlayerId = 0;
    bool fader1IsPlaying = false;
    bool fader2IsPlaying = false;
    int fader1PreviousMidiLevel = 0;
    int fader1ActualMidiLevel = 0;
    int fader2PreviousMidiLevel = 0;
    int fader2ActualMidiLevel = 0;
    
    std::array<int, 300> previousMidiLevels = { 0, 0, 0, 0, 0, 0, 0, 0 };
    std::array<int, 300> actualMidiLevels = { 0, 0, 0, 0, 0, 0, 0, 0 };
    std::array<int, 300> trimMidiLevels = { 0, 0, 0, 0, 0, 0, 0, 0 };


    juce::Value minimumPlayer;
    juce::Value fader1Value;
    juce::Value fader1DisplayValue;
    juce::Value playerStoppedID;
    juce::Value fader1Name;
    juce::Value fader2Name;

    juce::Value mouseDragX;
    juce::Value mouseDragY;
    juce::Value mouseDragSource;
    juce::Value mouseDraggedUp;
    juce::Value draggedPlayer;

    bool mouseCtrlModifier = false;
    bool mouseAltModifier = false;
    int fileDragPlayerDestination = 0;
    bool fileDragPaintLine = false;
    bool fileDragPaintRectangle = false;
    bool dragPaintSourceRectangle = false;
    int fileDragPlayerSource = -1;
    bool mouseDraggedStartPositionOK = false;

    bool scrollbarShown = false;

    void Playlist::setTimerTime(int timertime);
    void Playlist::shouldShowMeters(bool b);
    juce::ChangeBroadcaster* playBroadcaster;
    juce::ChangeBroadcaster* cuePlaylistBroadcaster;
    juce::ActionBroadcaster* cuePlaylistActionBroadcaster;
    juce::ChangeBroadcaster* fxButtonBroadcaster;
    juce::ChangeBroadcaster* envButtonBroadcaster;
    juce::ChangeBroadcaster* grabFocusBroadcaster;

    int cuedPlayer = 0;

private:
    void swapNext(int playerID);
    void updateNextPlayer();
    void updateButtonsStates();
    void changeListenerCallback(juce::ChangeBroadcaster* source);

    void spaceBarStop();
    bool isInterestedInFileDrag(const juce::StringArray& files);
    void timerCallback();
    void assignPlaylistFader(int playerToAssign);
    void buttonClicked(juce::Button* b) override;
    void fader1Start();
    void fader2Start();
    void valueChanged(juce::Value& value);
    void updateDraggedPlayerDisplay();
    void actionListenerCallback(const juce::String& message);

    int playlistType;
    int playlistPosition = 0;
    juce::String droppedName;

    int previousvalueint = 0;
    int valueint = 0;

    juce::OwnedArray<juce::TextButton> swapNextButtons;
    juce::OwnedArray<juce::TextButton> removePlayersButtons;
    juce::OwnedArray<juce::TextButton> addPlayersButtons;

    juce::OwnedArray<juce::Label> playersPositionLabels;
    juce::OwnedArray<Meter> meters;

    //GRAPHICS
    int playersStartHeightPosition = 2;
    int playerHeight = 100;
    int playerWidth = 670;
    int spaceBetweenPlayer = 5;
    int playerInsertDragZoneHeight = 20;
    int controlButtonHeight = 33;
    int controlButtonWidth = 25;
    int borderRectangleWidth = 10;
    int borderRectangleHeight = 80;
    int totalPlayerWidth = playerWidth + (borderRectangleWidth * 2);
    int totalPlayerWidthWithButtons = totalPlayerWidth + controlButtonWidth;
    int playerNumber;
    int meterWidth = 20;
    int assignFaderButtonWidth = 30;
    int assignFaderButtonHeight = 30;
    int dragZoneWidth = 40;
    int controlButtonXStart;

    int fader3Player = 0;
    int fader4Player = 1;

    bool fader1IsFlying = false;
    bool fader2IsFlying = false;

    bool fader1Stopped = false;
    bool fader2Stopped = false;

    //Midi
    int midiMessageNumber;
    int midiMessageValue;
    bool isAddingFromMidiInput = false;
    int midiChannelShift = 1;

    juce::String FFmpegPath;
    juce::String ExiftoolPath;
    juce::String convertedFilesPath;

    float skewFactor;
    int maxFaderValue;

    //FADER TEMPORISATION
    juce::int64 faderTempTime = 1000;
    juce::int64 fader1StartTime;
    juce::int64 fader1StopTime;
    juce::int64 fader2StartTime;
    juce::int64 fader2StopTime;
    juce::int64 spaceBarStartTime;
    juce::int64 spaceBarStopTime;

    int fader1StartPlayer;
    int fader2StartPlayer;
    bool keypressed = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Playlist)
};
