#pragma once

#include <JuceHeader.h>
#include "Player.h"
#include "Playlist.h"
#include "Settings.h"
#include "SoundPlayer.h"
#include "BottomComponent.h"
#include <string>
#include <iostream>
#include <ff_meters\ff_meters.h>
#include <memory>
#include <math.h>
#include <Ebu128LoudnessMeter.h>
#include "Mixer/Mixer.h"
#include "R128IntegratedThread.h"
#include "SoundBoard/KeyboardMappedSoundboard.h"
//#include <MacrosAndJuceHeaders.h>
//#include <SecondOrderIIRFilter.h>
//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent :   public juce::AudioAppComponent,
                        private juce::MidiInputCallback,
                        public juce::ChangeListener,
                        private juce::KeyListener,
                        public juce::MultiTimer,
                        public juce::Value::Listener,
                        private juce::Button::Listener,
                        public juce::MouseListener,
                        public juce::Slider::Listener,
                        public juce::ChangeBroadcaster,
                        public juce::ComboBox::Listener
{
public:
    //==============================================================================
    MainComponent();

    static bool MainComponent::exitAnswered;

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

    juce::AudioDeviceManager deviceManager;

    bool MainComponent::keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent);

    void MainComponent::timerCallback(int timerID);
    void MainComponent::changeListenerCallback(juce::ChangeBroadcaster* source) override;
    
    void MainComponent::setOptions(juce::String FFmpegPath, juce::String convertedSoundFolder, float skewFactor);
    void MainComponent::exitRequested();
    void MainComponent::deleteConvertedFiles();

    juce::Viewport playlistViewport;
    juce::Viewport playlistbisViewport;

    bool MainComponent::isPlayingOrRecording();
    void MainComponent::setCommandLine(juce::String commandLine);

private:
    //==============================================================================
    // Your private member variables go here...
    //juce::Time time;
    bool showMixer = false;

    //juce::OwnedArray<Player> players;
    juce::OwnedArray<SoundPlayer> soundPlayers;

    juce::MixerAudioSource myMixer;
    juce::MixerAudioSource myCueMixer;
    juce::OwnedArray<Playlist> myPlaylists;


    //juce::OwnedArray<juce::AudioBuffer<float>> myRemappingBuffers;
    //juce::OwnedArray<juce::AudioBuffer> myBuffers;

    void MainComponent::settingsButtonClicked();
    juce::AudioBuffer<float> outputBuffer;

    juce::AudioDeviceSelectorComponent audioSetupComp;
    juce::AudioDeviceManager::AudioDeviceSetup deviceSetup;
    juce::AudioDeviceManager::LevelMeter levelMeter;

    juce::DialogWindow audioSetupWindow;

    juce::DialogWindow settingsWindow;

    juce::Slider timerSlider;


    //Settings* settings = new Settings();
    std::unique_ptr<Settings> settings = std::make_unique<Settings>();

    double *ptrplayer1;
    double playersAdress[8]{ 0 };
    int playerNumber;


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


    int bottomHeight = 100;
    //int upDownButtonsHeight = 100;


    

    //AUDIO
    void MainComponent::audioInitialize();


    

    //Playlist viewportPlayer;


    //MIDI
    void midiInitialize();
    void MainComponent::updateMidiInputs();
    juce::ComboBox midiInputList;                     // [2]
    juce::Label midiInputListLabel;
    int lastInputIndex = 0;                           // [3]
    bool isAddingFromMidiInput = false;               // [4]
    juce::TextButton updateMidiInputsButton;

    std::unique_ptr<juce::MidiOutput>* midiOutputptr;

    bool midiInitialized = false;

   /* int fader1PreviousMidiLevel = 0;
    int fader1ActualMidiLevel = 0;
    int fader2PreviousMidiLevel = 0;
    int fader2ActualMidiLevel = 0;*/

  /*  bool fader1IsPlaying = false;
    bool fader2IsPlaying = false;

    int nextPlayer = 0;*/

    int bottomButtonsHeight = 30;

    float oscSkewFactor = 0.5;


    //Buttons
    juce::TextButton playersResetPosition;
    juce::TextButton playersPreviousPosition;
    juce::TextButton playersNextPosition;
    //juce::TextButton addPlayerEnd;

    juce::TextButton audioSetupButton;
    juce::TextButton settingsButton;
    juce::ComboBox soundPlayerTypeSelector;
    juce::TextButton saveButton;
    juce::TextButton loadButton;

    juce::TextButton addPlayerPlaylist;
    juce::TextButton removePlayerPlaylist;

    juce::TextButton addPlayerCart;
    juce::TextButton removePlayerCart;


    juce::TextButton connectOSCButton;
    juce::Label oscStatusLabel;

    juce::Label timeLabel;

  /*  juce::OwnedArray<juce::TextButton> swapNextButtons;
    juce::OwnedArray<juce::TextButton> removePlayersButtons;
    juce::OwnedArray<juce::TextButton> addPlayersButtons;*/


    //double startTime;

    //MIDI
    void MainComponent::setMidiInput(int index);
    void handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message) override;
    int midiMessageNumber;
    int midiMessageValue;


    void MainComponent::OSCInitialize();
  /*  void MainComponent::oscMessageReceived(const juce::OSCMessage& message) override;*/

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


    juce::OSCSender sender;
    juce::OSCReceiver receiver;

    //Settings settingsa;


    bool spaceBarIsPlaying = false;
    int spaceBarPlayerId = 0;

    
   /* int fileDragPlayerDestination = 0;
    bool fileDragPaintLine = false;
    bool fileDragPaintRectangle = false;*/


    void MainComponent::savePlaylist();
    void MainComponent::loadPlaylist();

    //juce::XmlElement playlist;

    //juce::ValueTree playlist;
    juce::TreeView tree;
    juce::File myFile;

    void MainComponent::showSettings();
    void MainComponent::MidiOutput(const juce::MidiMessage& message);


    void MainComponent::valueChanged(juce::Value& value);

    void MainComponent::OSCsend(int destination, float value);
    void MainComponent::OSCsend(int destination, juce::String string);
    void MainComponent::OSCClean();
    juce::String ipAdress;
    int outPort;
    int inPort = -1;
    int currentPortNumber = -1;
    int maxFaderValue;
    bool oscConnected = false;
    bool isConnected() const
    {
        return inPort != -1;
    }

    foleys::LevelMeterLookAndFeel lnf;
    foleys::LevelMeter meter{ foleys::LevelMeter::MaxNumber }; // See foleys::LevelMeter::MeterFlags for options
    foleys::LevelMeterSource meterSource;
    foleys::LevelMeter cuemeter{ foleys::LevelMeter::MaxNumber }; // See foleys::LevelMeter::MeterFlags for options
    foleys::LevelMeterSource cuemeterSource;


    int levelMeterMaximumHeight = 600;
    int levelMeterHeight;
    int cuelevelMeterMaximumHeight = 300;
    int cuelevelMeterMinimumHeight = 150;
    int cuelevelMeterHeight;

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

    juce::ApplicationProperties properties;

    juce::PropertiesFile::Options options;
    juce::PropertiesFile settingsFile;

    void MainComponent::buttonClicked(juce::Button* button) override;

    juce::DragAndDropContainer dragAndDrop;
    
    //DRAG AND DROP
    void MainComponent::mouseDown(const juce::MouseEvent& event);
    void MainComponent::mouseDragGetInfos(int playlistSource, int playerID);
    void MainComponent::mouseDragDefinePlayer();
    void MainComponent::mouseDragSetInfos(int playlistDestination, int playerIdDestination);
    void MainComponent::setSoundInfos(int playlistDestination, int playerIdDestination);
    void MainComponent::clearDragInfos();
    void MainComponent::drawDragLines();
    void MainComponent::mouseDragEnd(int playlistDestination);
    bool MainComponent::isDraggable(int playlistSource, int playerSource, int playlistDestination, int playerDestination);
    void MainComponent::checkAndRemovePlayer(int playlist, int player);
    void MainComponent::reassignFaders(int playerIdDestination, int playerDragSource, int playlistDestination);
    void MainComponent::copyPlayingSound();
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

    //WAVEFORM CONTROL
    int draggedPlayer = 0;
    int draggedPlaylist = 0;

    int playerSourceLatch;
    int playlistSourceLatch;

    //METERS
    void MainComponent::channelsMapping();
    void MainComponent::metersInitialize();
    void MainComponent::tryPreferedAudioDevice(int outputChannelsNeeded);

    Ebu128LoudnessMeter loudnessMeter;
    Ebu128LoudnessMeter cueloudnessMeter;
    int meterNumericDisplayHeight = 28;
    float shortTermLoudness = 0;
    float cueShortTermLoudness = 0;

    void MainComponent::defaultPlayersPlaylist(int playersToAdd);

    //LAYOUT
    juce::StretchableLayoutManager myLayout;
    BottomComponent bottomComponent;
    std::unique_ptr<juce::StretchableLayoutResizerBar> horizontalDividerBar;
    //juce::ProgressBar progressBar;


    //RECORDER
    juce::AudioBuffer<float>* micBuffer;
    //std::unique_ptr<juce::AudioBuffer<float>> inputBuffer;
    //juce::AudioBuffer<float>* inputBuffer{0};

    juce::Label cpuUsage;

    void MainComponent::sliderValueChanged(juce::Slider* slider);

    void MainComponent::launchRecord();


    static void  MainComponent::alertBoxResultChosen(int result, MainComponent*);

    bool isEightPlayerMode = false;
    void MainComponent::launchSoundPlayer(SoundPlayer::Mode m);
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged);
    void modifierKeysChanged(const juce::ModifierKeys& modifiers);
    bool eightPlayersLaunched = false;

    //MIXER
    std::unique_ptr<juce::AudioBuffer<float>> mixerOutputBuffer;
    Mixer mixer;
    int mixerHeight = 260;

    bool playerSwitching = false;

    std::unique_ptr<KeyboardMappedSoundboard> keymapSoundboard;
    juce::ModifierKeys currentModifier;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
