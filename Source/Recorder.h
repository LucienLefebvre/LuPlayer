/*
  ==============================================================================

    Recorder.h
    Created: 16 Mar 2021 10:05:40am
    Author:  Lucien

  ==============================================================================
*/

#pragma once
#define JUCE_USE_LAME_AUDIO_FORMAT 1
#include <JuceHeader.h>
#include "Player.h"
#include <algorithm>
#include <Ebu128LoudnessMeter.h>
#include "Settings/KeyMapper.h"
//==============================================================================
/*
*/
class Recorder : public juce::Component,
    public juce::ChangeListener,
    public juce::Timer,
    public juce::MouseListener,
    public juce::ChangeBroadcaster,
    public juce::Value::Listener
    
{
public:
    Recorder();
    ~Recorder() override;

    void Recorder::prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void Recorder::recordAudioBuffer(juce::AudioBuffer<float>* soundBuffer, juce::AudioBuffer<float>* micBuffer, juce::AudioBuffer<float>* outputBuffer, int channels, double sampleRate, int samplesPerBlockExpected);

    void Recorder::paint (juce::Graphics&) override;
    void Recorder::resized() override;
    bool Recorder::isEnabled();
    bool Recorder::keyPressed(const juce::KeyPress& key, KeyMapper* keyMapper);
    void Recorder::enableButtonClicked();
    void Recorder::recordButtonClicked();
    bool Recorder::isRecording();
    void Recorder::setStart();
    void Recorder::setStop();

    juce::ToggleButton enabledButton;
    juce::ToggleButton enableMonitoring;
    bool isRecorderEnabled = false;
    juce::AudioTransportSource cueTransport;
    juce::ChangeBroadcaster* mouseDragInRecorder;
    juce::ChangeBroadcaster* spaceBarKeyPressed;;
    juce::ChangeBroadcaster* recordingBroadcaster;

private:


    void Recorder::startRecording();
    void Recorder::stopRecording();
    void Recorder::start(const juce::File& file);
    void Recorder::stop();

    void Recorder::changeListenerCallback(juce::ChangeBroadcaster* source);
    void Recorder::timerCallback();
    juce::String Recorder::secondsToMMSS(int seconds);
    void Recorder::saveButtonClicked();
    void Recorder::cropRecordedFile(juce::File& fileToCrop);
    void Recorder::mouseDrag(const juce::MouseEvent& event);
    void Recorder::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel);
    void Recorder::mouseDoubleClick(const juce::MouseEvent& event);
    void Recorder::calculThumbnailBounds();
    void Recorder::repaintThumbnail();
    void Recorder::mouseDown(const juce::MouseEvent& event);
    void Recorder::mouseUp(const juce::MouseEvent& event);
    bool Recorder::loadFile(const juce::String& path);
    void Recorder::deleteStart();
    void Recorder::deleteStop();
    void Recorder::cueButtonClicked();
    void Recorder::valueChanged(juce::Value& value);


    foleys::LevelMeterLookAndFeel lnf;
    foleys::LevelMeter meter{ foleys::LevelMeter::Horizontal }; // See foleys::LevelMeter::MeterFlags for options
    foleys::LevelMeterSource meterSource;

    Ebu128LoudnessMeter loudnessMeter;
    float shortTermLoudness;

    double actualSampleRate;
    int actualSamplesPerBlockExpected;
    int numChannels;
    bool displayFullThumb = false;
    juce::Rectangle<int> thumbnailBounds;
    juce::Rectangle<int> editingThumbnailBounds;

    juce::TextButton recordButton{ "Record" };
    juce::TextButton saveButton{ " Save" };

    juce::File lastRecording;
    juce::File fileToSave;

    juce::AudioFormatManager formatManager;
    juce::AudioThumbnailCache thumbnailCache{ 10 };
    juce::AudioThumbnail thumbnail{ 512, formatManager, thumbnailCache };

    juce::TimeSliceThread backgroundThread{ "Audio Recorder Thread" };
    std::unique_ptr<juce::AudioFormatWriter::ThreadedWriter> threadedWriter;

    juce::int64 nextSampleNum = 0;

    juce::CriticalSection writerLock;
    std::atomic<juce::AudioFormatWriter::ThreadedWriter*> activeWriter{ nullptr };
    juce::AudioFormatWriter* writer;


    //BUTTONS
    juce::Slider micSlider;
    juce::Label micLabel;
    juce::Slider soundSlider;
    juce::Label soundLabel;
    juce::Label timeLabel;


    //FILE SAVE
    juce::AudioFormatManager outputformatManager;


    //WAVEFORM DRAG
    int mouseDragXPosition;
    int waveformThumbnailXStart = 314;
    int waveformThumbnailXSize;
    int waveformThumbnailXEnd;
    int mouseDragRelativeXPosition;
    float mouseDragInSeconds;
    int thumbnailMiddle;
    int thumbnailDrawStart;
    float thumbnailHorizontalZoom = 1;
    int thumbnailDrawEnd;
    int thumbnailDrawSize;
    float thumbnailOffset = 0;
    float oldThumbnailOffset = 0;
    int thumbnailDragStart = 0;
    int cursorPosition;

    float startTime = 0;
    float stopTime;
    bool startTimeSet = false;
    bool stopTimeSet = false;


    bool fileRecorded = false;

    juce::TextButton cueButton;
    juce::AudioThumbnail editingThumbnail;
    juce::AudioThumbnailCache editingThumbnailCache;
    juce::AudioFormatManager editingFormatManager;
    int leftControlWidth = 340;

    juce::ComboBox formatBox;
    std::unique_ptr<juce::FileChooser> chooser;
    std::unique_ptr<juce::LAMEEncoderAudioFormat> encoder;
    //juce::AudioFormatReaderSource cuetempSource;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;

    juce::Label marksLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Recorder)
};
