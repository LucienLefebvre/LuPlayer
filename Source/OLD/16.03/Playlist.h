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


//==============================================================================
/*
*/
class Playlist  : public juce::Component,
    //public juce::AudioAppComponent,
    private juce::MidiInputCallback,
    private juce::ChangeListener,
    //private juce::KeyListener,
    public juce::FileDragAndDropTarget,
    public juce::Value::Listener,
    public juce::Button::Listener,
    public juce::MouseListener,
    public juce::Timer
{
public:
    Playlist(int splaylistType);
    ~Playlist() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void Playlist::prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void Playlist::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill);


    void Playlist::handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message);
    bool Playlist::keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent);
    void Playlist::playersResetPositionClicked();
    void Playlist::playersPreviousPositionClicked();
    void Playlist::playersNextPositionClicked();

    void Playlist::addPlayer(int playerID);
    void Playlist::removePlayer(int playerID);
    void Playlist::removeButtonClicked();

    void Playlist::filesDropped(const juce::StringArray& files, int x, int y);
    void Playlist::fileDragMove(int x, int y);
    void Playlist::fileDragExit();

    juce::OwnedArray<Player> players;
    juce::MixerAudioSource playlistMixer;
    juce::MixerAudioSource playlistCueMixer;
    
    double actualSampleRate;
    int actualSamplesPerBlockExpected;

    void Playlist::setOptions();
    //void Playlist::setOptions(juce::String sFFmpegPath, juce::String sExiftoolPath, juce::String sconvertedFilesPath, float sskewFactor, int smaxFaderValue);
    void Playlist::setMidiShift(int MidiShift);


    void Playlist::assignLeftFader(int playerID);
    void Playlist::assignRightFader(int playerID);
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
    juce::Value minimumPlayer;
    juce::Value fader1Value;
    juce::Value fader1DisplayValue;
    juce::Value playerStoppedID;
    void Playlist::handleFader1(int faderValue);
    void Playlist::handleFader1OSC(float faderValue);
    void Playlist::handleFader2(int faderValue);
    void Playlist::handleFader2OSC(float faderValue);
    void Playlist::handleFader3(int faderValue);
    void Playlist::handleFader3OSC(float faderValue);
    void Playlist::handleFader4(int faderValue);
    void Playlist::handleFader4OSC(float faderValue);
    void Playlist::rearrangePlayers();

    juce::Value fader1Name;
    juce::Value fader2Name;
    void Playlist::fader1Stop(bool stoppedByFader);
    void Playlist::fader2Stop(bool stoppedByFader);
    void Playlist::mouseDrag(const juce::MouseEvent& event);
    void Playlist::mouseDown(const juce::MouseEvent& event);
    void Playlist::mouseUp(const juce::MouseEvent& event);
    void Playlist::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel);
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
    int fileDragPlayerSource = 0;
    bool mouseDraggedStartPositionOK = false;

    bool scrollbarShown = false;


private:
    int playlistType;

    void Playlist::updateNextPlayer();
    int previousvalueint = 0;
    int valueint = 0;
    
    void Playlist::swapNext(int playerID);

    void Playlist::updateButtonsStates();
    void Playlist::changeListenerCallback(juce::ChangeBroadcaster* source);
    void Playlist::spaceBarPressed();
    void Playlist::spaceBarStop();
    bool Playlist::isInterestedInFileDrag(const juce::StringArray& files);



    void Playlist::buttonClicked(juce::Button* b) override;


    void Playlist::fader1Start();
    void Playlist::fader2Start();


    juce::OwnedArray<juce::TextButton> swapNextButtons;
    juce::OwnedArray<juce::TextButton> removePlayersButtons;
    juce::OwnedArray<juce::TextButton> addPlayersButtons;
    juce::OwnedArray<juce::TextButton> assignLeftFaderButtons;
    juce::OwnedArray<juce::TextButton> assignRightFaderButtons;
    juce::OwnedArray<juce::Label> playersPositionLabels;

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



    int assignFaderButtonWidth = 30;
    int assignFaderButtonHeight = 30;

    int dragZoneWidth = 40;


    int controlButtonXStart;

    //Management Conduite

    int fader3Player = 0;
    int fader4Player = 1;



    //bool fader1Stopped = true;
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

    void Playlist::valueChanged(juce::Value& value);


    //FADER TEMPORISATION
    juce::int64 faderTempTime = 1000;
    juce::int64 fader1StartTime;
    juce::int64 fader1StopTime;
    juce::int64 fader2StartTime;
    juce::int64 fader2StopTime;

    int fader1StartPlayer;
    int fader2StartPlayer;
    void Playlist::playPlayer(int playerID);

    bool keypressed = 0;

    void Playlist::timerCallback();
    void Playlist::assignPlaylistFader(int playerToAssign);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Playlist)
};
