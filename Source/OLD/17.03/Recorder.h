/*
  ==============================================================================

    Recorder.h
    Created: 16 Mar 2021 10:05:40am
    Author:  Lucien

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Player.h"

//==============================================================================
/*
*/
class Recorder : public juce::Component,
    public juce::ChangeListener,
    public juce::Timer,
    public juce::MouseListener
    
{
public:
    Recorder();
    ~Recorder() override;

    void Recorder::prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void Recorder::recordAudioBuffer(juce::AudioBuffer<float>* soundBuffer, juce::AudioBuffer<float>* micBuffer, int channels, double sampleRate, int samplesPerBlockExpected);

    void Recorder::paint (juce::Graphics&) override;
    void Recorder::resized() override;
    bool Recorder::isEnabled();
    juce::AudioTransportSource cueTransport;

private:
    juce::ToggleButton enabledButton;
    bool isRecorderEnabled = false;
    void Recorder::startRecording();
    void Recorder::stopRecording();
    void Recorder::start(const juce::File& file);
    void Recorder::stop();
    bool Recorder::isRecording();
    void Recorder::changeListenerCallback(juce::ChangeBroadcaster* source);
    void Recorder::enableButtonClicked();
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
    void Recorder::setStart();
    void Recorder::setStartTime(float time);
    void Recorder::setStopTime(float time);
    void Recorder::setStop();
    bool Recorder::loadFile(const juce::String& path);

    foleys::LevelMeterLookAndFeel lnf;
    foleys::LevelMeter meter{ foleys::LevelMeter::Horizontal }; // See foleys::LevelMeter::MeterFlags for options
    foleys::LevelMeterSource meterSource;

    double actualSampleRate;
    int actualSamplesPerBlockExpected;
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
    bool startTimeSet;
    bool stopTimeSet;

    bool fileRecorded = false;

    juce::TextButton cueButton;
    juce::AudioThumbnail editingThumbnail;
    juce::AudioThumbnailCache editingThumbnailCache;
    juce::AudioFormatManager editingFormatManager;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Recorder)
};
