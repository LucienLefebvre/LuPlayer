#pragma once

#include <JuceHeader.h>

#include "SoundBoard/Player.h"
#include "SoundBoard/Playlist.h"
#include "Settings/Settings.h"
#include "SoundBoard/SoundPlayer.h"
#include "BottomComponent/BottomComponent.h"
#include "Externals/LUFSMeter/Ebu128LoudnessMeter.h"
#include "Mixer/Mixer.h"
#include "Others/StopWatch.h"
#include "Settings/KeyMapper.h"
#include "Settings/MidiMapper.h"
#include "Others/updateDialog.h"
#include "Others/SignalGenerator.h"
#include <string>
#include <iostream>
#include <memory>
#include <math.h>

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
                        public juce::MenuBarModel,
                        public juce::ApplicationCommandTarget
{
public:
    //==============================================================================
    enum CommandIDs
    {
        startTimer = 1,
        showTimer,
        showClock,
        showIndividualMeters,
        showEnveloppe,
        viewLastPlayedSound,
        showInBottomPanel,
        lanchRecord,
        goToSoundBrowser,
        goToClipEditor,
        goToClipEffect,
        goToRecorder,
        goToTextEditor,
        spaceBarPlay,
        goToFirst,
        goToNext,
        goToPrevious,
        play1,
        play2,
        play3,
        play4,
        play5,
        play6,
        play7,
        play8,
        play9,
        play10,
        play11,
        play12,
        save,
        open,
        quit,
        openSettings,
        audioSettings,
        keyboardMapping,
        midiMapping,
        launchPlaylist,
        launch8Faders,
        launchKeyMapped,
        setInMark,
        deleteInMark,
        setOutMark,
        deleteOutMark,
        documentation,
        about,
        autoCheckUpdate,
        showSignalGenerator,
        enableOSC,
        upload,
		newPlaylist,
    };

    MainComponent();
    ~MainComponent() override;

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    bool isPreparedToPlay = false;

    void paint (juce::Graphics& g) override;
    void resized() override;
    bool keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent);
    void timerCallback(int timerID);
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    
    void setOptions(juce::String FFmpegPath, juce::String convertedSoundFolder, float skewFactor);
    void deleteConvertedFiles();

    bool isPlayingOrRecording();
    void setCommandLine(juce::String commandLine);

    void MainComponent::savePlaylist();
    void MainComponent::loadPlaylist();
    bool hasBeenSaved();

    juce::AudioDeviceManager deviceManager;
    juce::Viewport playlistViewport;
    juce::Viewport playlistbisViewport;

    static bool MainComponent::exitAnswered;

    void checkNewVersion();

private:
    Settings settings;
    int numInputsChannels = 2;

    double actualSampleRate;
    int actualSamplesPerBlockExpected;

    bool showMixer = false;
    juce::OwnedArray<SoundPlayer> soundPlayers;

    void audioInitialize();
    void channelsMapping();
    void tryPreferedAudioDevice(int outputChannelsNeeded);
    void releaseResources();
    void menuBarItemSelected(int menuItemID, int topLevelMenuIndex);
    void midiInitialize();
    void handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message) override;

    void settingsButtonClicked();
    void audioSettingsButtonClicked();
    void keyMapperButtonClicked();
    void midiMapperButtonClicked();
    void signalGeneratorButtonClicked();

    void valueChanged(juce::Value& value);
    void buttonClicked(juce::Button* button) override;

    void defaultPlayersPlaylist(int playersToAdd);

    void sliderValueChanged(juce::Slider* slider);
    void launchRecord();
    void initializeBottomComponent();
    void launchSoundPlayer(SoundPlayer::Mode m);
    void stopWatchShortcuPressed();

    int checkIfSoundLoaded();
    void reloadTempPlayers();

    juce::AudioDeviceSelectorComponent audioSetupComp;
    juce::AudioDeviceManager::AudioDeviceSetup deviceSetup;
    juce::AudioDeviceManager::LevelMeter levelMeter;

    juce::DialogWindow audioSetupWindow;
    juce::DialogWindow settingsWindow;

    int menuBarHeight = 25;
    int playersStartHeightPosition = 25;
    int bottomHeight = 100;
    int playlistStartY = 30;

    juce::Label timeLabel;

    juce::StretchableLayoutManager myLayout;
    BottomComponent bottomComponent;
    std::unique_ptr<juce::StretchableLayoutResizerBar> horizontalDividerBar;

    juce::ApplicationCommandManager commandManager;
    juce::ApplicationCommandTarget* getNextCommandTarget();
    void getAllCommands(juce::Array<juce::CommandID>& commands);
    void getCommandInfo(juce::CommandID commandID, juce::ApplicationCommandInfo& result);
    bool perform(const InvocationInfo& info);
    void menuItemSelected(int menuItemID, int topLevelMenuIndex);
    juce::StringArray getMenuBarNames();
    juce::PopupMenu getMenuForIndex(int menuIndex, const juce::String& menuName);
    std::unique_ptr<juce::MenuBarComponent> menuBar;
    KeyMapper* km;
    MidiMapper midiMapper;
    bool midiMapperOpened = false;

    bool eightPlayersLaunched = false;
    bool isEightPlayerMode = false;
    bool soundboardLaunched = false;

    juce::ModifierKeys currentModifier;
    bool saved = false;

    std::unique_ptr<juce::AudioBuffer<float>> inputBuffer;
    std::unique_ptr<juce::AudioBuffer<float>> outputBuffer;
    std::unique_ptr<juce::AudioBuffer<float>> newOutputBuffer;
    std::unique_ptr<juce::AudioBuffer<float>> loudnessMeterBuffer;

    std::unique_ptr<juce::AudioSourceChannelInfo> cueAudioSource;
    std::unique_ptr<juce::AudioSourceChannelInfo> bufferAudioSource;

    std::unique_ptr<juce::AudioBuffer<float>> cueBuffer;
    std::unique_ptr<juce::AudioSourceChannelInfo> playAudioSource;
    std::unique_ptr<juce::AudioSourceChannelInfo> bottomComponentSource;
    std::unique_ptr<juce::AudioBuffer<float>> bottomComponentBuffer;

    bool initializationFocusGained = false;

    std::unique_ptr<juce::FileLogger> logger;

    juce::Array<Player::PlayerInfo> tempPlayers;

    SignalGenerator signalGenerator;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};