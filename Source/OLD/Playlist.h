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
    public juce::Value::Listener
    //public juce::Timer
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

    juce::OwnedArray<Player> players;
    juce::MixerAudioSource playlistMixer;
    
    double actualSampleRate;
    int actualSamplesPerBlockExpected;

    void Playlist::setOptions(juce::String sFFmpegPath, juce::String sExiftoolPath, juce::String sconvertedFilesPath, float sskewFactor, int smaxFaderValue);
    void Playlist::setMidiShift(int MidiShift);


    void Playlist::assignLeftFader(int playerID);
    void Playlist::assignRightFader(int playerID);

    int fader1Player = 0;
    int fader2Player = 0;
    bool fader1IsPlaying = false;
    bool fader2IsPlaying = false;
    juce::Value minimumPlayer;
    juce::Value fader1Value;
    juce::Value fader1DisplayValue;
    juce::Value playerStoppedID;
    void Playlist::handleFader1(int faderValue);
    void Playlist::handleFader2(int faderValue);
    void Playlist::handleFader3(int faderValue);
    void Playlist::handleFader4(int faderValue);

private:
    int playlistType;

    void Playlist::updateNextPlayer();
    int previousvalueint = 0;
    int valueint = 0;
    
    void Playlist::swapNext(int playerID);
    void Playlist::rearrangePlayers();
    void Playlist::updateButtonsStates();
    void Playlist::changeListenerCallback(juce::ChangeBroadcaster* source);
    void Playlist::spaceBarPressed();
    void Playlist::spaceBarStop();
    bool Playlist::isInterestedInFileDrag(const juce::StringArray& files);
    void Playlist::filesDropped(const juce::StringArray& files, int x, int y);
    void Playlist::fileDragMove(const juce::StringArray& files, int x, int y);
    void Playlist::fileDragExit(const juce::StringArray& files);


    juce::OwnedArray<juce::TextButton> swapNextButtons;
    juce::OwnedArray<juce::TextButton> removePlayersButtons;
    juce::OwnedArray<juce::TextButton> addPlayersButtons;
    juce::OwnedArray<juce::TextButton> assignLeftFaderButtons;
    juce::OwnedArray<juce::TextButton> assignRightFaderButtons;

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

    int fileDragPlayerDestination = 0;
    bool fileDragPaintLine = false;
    bool fileDragPaintRectangle = false;

    int assignFaderButtonWidth = 30;
    int assignFaderButtonHeight = 30;


    int controlButtonXStart;

    //Management Conduite

    int fader3Player = 0;
    int fader4Player = 1;

    int fader1PreviousMidiLevel = 0;
    int fader1ActualMidiLevel = 0;
    int fader2PreviousMidiLevel = 0;
    int fader2ActualMidiLevel = 0;




    int nextPlayer = 0;

    bool spaceBarIsPlaying = false;
    int spaceBarPlayerId = 0;


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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Playlist)
};
