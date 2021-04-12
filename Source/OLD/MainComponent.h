#pragma once

#include <JuceHeader.h>
#include "Player.h"
#include <string>
#include <iostream>
//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent : public juce::AudioAppComponent,
                      private juce::MidiInputCallback,
                      private juce::ChangeListener,
                      private juce::KeyListener,
                      public juce::FileDragAndDropTarget,
                      public juce::Timer

    
                       //private juce::MidiKeyboardStateListener
{
public:
    //==============================================================================
    MainComponent();

        

    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;

    double actualSampleRate;
    int actualSamplesPerBlockExpected;

    bool MainComponent::keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent);

    void MainComponent::timerCallback();

    

private:
    //==============================================================================
    // Your private member variables go here...

    juce::OwnedArray<Player> players;
    juce::MixerAudioSource myMixer;

    juce::AudioDeviceManager deviceManager;
    juce::AudioDeviceSelectorComponent audioSetupComp;
    juce::AudioDeviceManager::AudioDeviceSetup deviceSetup;
    juce::AudioDeviceManager::LevelMeter levelMeter;
    

    double *ptrplayer1;
    double playersAdress[8]{ 0 };
    int playerNumber;


    //GRAPHIC
    int playersStartHeightPosition = 30;

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

    

    //AUDIO
    void MainComponent::audioInitialize();


    juce::Viewport playlistViewport;
    //Player viewportPlayer;


    //MIDI
    void midiInitialize();
    juce::ComboBox midiInputList;                     // [2]
    juce::Label midiInputListLabel;
    int lastInputIndex = 0;                           // [3]
    bool isAddingFromMidiInput = false;               // [4]

    //Management Conduite
    int fader1Player = 0;
    int fader2Player = 0;

    int fader1PreviousMidiLevel = 0;
    int fader1ActualMidiLevel = 0;
    int fader2PreviousMidiLevel = 0;
    int fader2ActualMidiLevel = 0;

    bool fader1IsPlaying = false;
    bool fader2IsPlaying = false;

    int nextPlayer = 0;


    //Buttons
    juce::TextButton playersResetPosition;
    juce::TextButton playersPreviousPosition;
    juce::TextButton playersNextPosition;
    juce::TextButton addPlayerEnd;

    juce::TextButton saveButton;
    juce::TextButton loadButton;


    juce::OwnedArray<juce::TextButton> swapNextButtons;
    juce::OwnedArray<juce::TextButton> removePlayersButtons;
    juce::OwnedArray<juce::TextButton> addPlayersButtons;


    //double startTime;

    void MainComponent::setMidiInput(int index);
    void handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message) override;
    int midiMessageNumber;
    int midiMessageValue;


    class IncomingMessageCallback : public juce::CallbackMessage
    {
    public:
        IncomingMessageCallback(MainComponent* o, const juce::MidiMessage& m, const juce::String& s)
            : owner(o), message(m), source(s)
        {}

        void messageCallback() override
        {
            //if (owner != nullptr)
                //owner->addMessageToList(message, source);
        }

        Component::SafePointer<MainComponent> owner;
        juce::MidiMessage message;
        juce::String source;
    };

    void handleFader1(int faderValue);
    void handleFader2(int faderValue);
    void updateNextPlayer();
    void MainComponent::playersResetPositionClicked();
    void MainComponent::playersPreviousPositionClicked();
    void MainComponent::playersNextPositionClicked();

    void MainComponent::addPlayer(int playerID);
    void MainComponent::removePlayer(int playerID);
    void MainComponent::swapNext(int playerID);
    void MainComponent::rearrangePlayers();
    void MainComponent::updateButtonsStates();


    void MainComponent::changeListenerCallback(juce::ChangeBroadcaster* source);
    void MainComponent::spaceBarPressed();
    void MainComponent::spaceBarStop();

    bool spaceBarIsPlaying = false;
    int spaceBarPlayerId = 0;

    bool MainComponent::isInterestedInFileDrag(const juce::StringArray& files);
    void MainComponent::filesDropped(const juce::StringArray& files, int x, int y);
    void MainComponent::fileDragMove(const juce::StringArray& files, int x, int y);
    void MainComponent::fileDragExit(const juce::StringArray& files);
    
    int fileDragPlayerDestination = 0;
    bool fileDragPaintLine = false;
    bool fileDragPaintRectangle = false;


    void MainComponent::savePlaylist();
    void MainComponent::loadPlaylist();

    //juce::XmlElement playlist;

    //juce::ValueTree playlist;
    juce::TreeView tree;
    juce::File myFile;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
