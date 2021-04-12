/*
  ==============================================================================

    Recorder.h
    Created: 16 Mar 2021 10:05:40am
    Author:  Lucien

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class Recorder : public juce::Component,
    public juce::ChangeListener
    
{
public:
    Recorder();
    ~Recorder() override;

    void Recorder::prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void Recorder::recordAudioBuffer(juce::AudioBuffer<float>* soundBuffer, juce::AudioBuffer<float>* micBuffer, int channels, double sampleRate, int samplesPerBlockExpected);

    void paint (juce::Graphics&) override;
    void resized() override;


private:
    void Recorder::startRecording();
    void Recorder::stopRecording();
    void Recorder::start(const juce::File& file);
    void Recorder::stop();
    bool Recorder::isRecording();
    void Recorder::changeListenerCallback(juce::ChangeBroadcaster* source);
    foleys::LevelMeterLookAndFeel lnf;
    foleys::LevelMeter meter{ foleys::LevelMeter::Horizontal }; // See foleys::LevelMeter::MeterFlags for options
    foleys::LevelMeterSource meterSource;

    double actualSampleRate;
    int actualSamplesPerBlockExpected;
    bool displayFullThumb = false;
    juce::Rectangle<int> thumbnailBounds;

    juce::TextButton recordButton{ "Record" };

    juce::File lastRecording;

    juce::AudioFormatManager formatManager;
    juce::AudioThumbnailCache thumbnailCache{ 10 };
    juce::AudioThumbnail thumbnail{ 512, formatManager, thumbnailCache };

    juce::TimeSliceThread backgroundThread{ "Audio Recorder Thread" }; // the thread that will write our audio data to disk
    std::unique_ptr<juce::AudioFormatWriter::ThreadedWriter> threadedWriter; // the FIFO used to buffer the incoming data
    //std::unique_ptr<juce::AudioFormatWriter>* writer; // the FIFO used to buffer the incoming data
    juce::int64 nextSampleNum = 0;

    juce::CriticalSection writerLock;
    std::atomic<juce::AudioFormatWriter::ThreadedWriter*> activeWriter{ nullptr };
    juce::AudioFormatWriter* writer;


    juce::Slider micSlider;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Recorder)
};
