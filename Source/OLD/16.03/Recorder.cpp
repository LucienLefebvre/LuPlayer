/*
  ==============================================================================

    Recorder.cpp
    Created: 16 Mar 2021 10:05:40am
    Author:  Lucien

  ==============================================================================
*/

#include <JuceHeader.h>
#include "Recorder.h"

//==============================================================================
Recorder::Recorder() : thumbnail(521, formatManager, thumbnailCache)
{

    backgroundThread.startThread();
    addAndMakeVisible(recordButton);
    thumbnail.addChangeListener(this);
    recordButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffff5c5c));
    recordButton.setColour(juce::TextButton::textColourOnId, juce::Colours::black);

    addAndMakeVisible(&micSlider);
    micSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    micSlider.setRange(0., 1.);
    micSlider.setValue(1.0);
    micSlider.setSkewFactor(0.5, false);
    micSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 50, 15);
    micSlider.setWantsKeyboardFocus(false);
    micSlider.setDoubleClickReturnValue(true, 1.);

    recordButton.onClick = [this]
    {
        if (isRecording())
            stopRecording();
        else
            startRecording();
    };


    //juce::AudioThumbnail& getAudioThumbnail() { return thumbnail; }

    addAndMakeVisible(meter);
    lnf.setColour(foleys::LevelMeter::lmBackgroundColour, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    lnf.setColour(foleys::LevelMeter::lmMeterGradientLowColour, juce::Colours::green);
    lnf.setColour(foleys::LevelMeter::lmMeterGradientMaxColour, juce::Colours::red);
    lnf.setColour(foleys::LevelMeter::lmMeterGradientMidColour, juce::Colours::orange);
}

Recorder::~Recorder()
{
}

void Recorder::paint (juce::Graphics& g)
{


    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background



    g.setColour(juce::Colour(40, 134, 189));
   if (thumbnail.getTotalLength() > 0.0)
   {
           thumbnail.drawChannels(g, thumbnailBounds, 0, thumbnail.getTotalLength(), 1);
   }
}

void Recorder::resized()
{
    recordButton.setBounds(0, 0, 100, 25);
    meter.setBounds(getWidth()*2/3, 0, getWidth() / 3, 80);
    thumbnailBounds.setBounds(getWidth() / 3, 0, getWidth() / 3, getHeight());
    micSlider.setBounds(120, 0, 50, getHeight());
}

void Recorder::recordAudioBuffer(juce::AudioBuffer<float>* soundBuffer, juce::AudioBuffer<float>* micBuffer, int channels, double sampleRate, int samplesPerBlockExpected)
{

    //int numSamples = bufferToRecord->getNumSamples();
    //int numSamples = sourceToRecord. getNumSamples();
    const juce::ScopedLock sl(writerLock);

    //    if (activeWriter.load() != nullptr && channels >= thumbnail.getNumChannels())
    if (activeWriter.load() != nullptr)
    {

        //create two audio buffers
        juce::AudioBuffer<float> recordingMicBuffer(2, actualSamplesPerBlockExpected);
        juce::AudioBuffer<float> recordingSoundBuffer(2, actualSamplesPerBlockExpected);
        recordingSoundBuffer.clear();
        recordingMicBuffer.clear();
        //Copy from the original buffer
        auto* inBufferL = micBuffer->getReadPointer(0);
        auto* inBufferR = micBuffer->getReadPointer(1);
        recordingMicBuffer.copyFrom(0, 0, inBufferL, actualSamplesPerBlockExpected);
        recordingMicBuffer.copyFrom(1, 0, inBufferR, actualSamplesPerBlockExpected);
        //recordingMicBuffer.copyFrom(1, 0, *micBuffer, 1, 0, actualSamplesPerBlockExpected);



        recordingSoundBuffer.copyFrom(0, 0, *soundBuffer, 0, 0, actualSamplesPerBlockExpected);
        recordingSoundBuffer.copyFrom(1, 0, *soundBuffer, 1, 0, actualSamplesPerBlockExpected);

        //add them to the recording buffer
        juce::AudioBuffer<float> recordingBuffer(2, actualSamplesPerBlockExpected);
        recordingBuffer.clear();
        recordingBuffer.addFrom(0, 0, recordingMicBuffer, 0, 0, actualSamplesPerBlockExpected, micSlider.getValue());
        recordingBuffer.addFrom(1, 0, recordingMicBuffer, 1, 0, actualSamplesPerBlockExpected, micSlider.getValue());
        recordingBuffer.addFrom(0, 0, recordingSoundBuffer, 0, 0, actualSamplesPerBlockExpected, 1.0);
        recordingBuffer.addFrom(1, 0, recordingSoundBuffer, 1, 0, actualSamplesPerBlockExpected, 1.0);

        activeWriter.load()->write(recordingBuffer.getArrayOfReadPointers(), recordingBuffer.getNumSamples());
        
        // Create an AudioBuffer to wrap our incoming data, note that this does no allocations or copies, it simply references our input data
        juce::AudioBuffer<float> buffer(const_cast<float**> (recordingBuffer.getArrayOfReadPointers()), thumbnail.getNumChannels(), recordingBuffer.getNumSamples());
        thumbnail.addBlock(nextSampleNum, buffer, 0, recordingBuffer.getNumSamples());
        nextSampleNum += recordingBuffer.getNumSamples();

        meterSource.measureBlock(recordingBuffer);
    }


}

void Recorder::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{

    actualSampleRate = sampleRate;
    actualSamplesPerBlockExpected = samplesPerBlockExpected;
    meterSource.resize(2, sampleRate * 0.1 / samplesPerBlockExpected);
    meter.setLookAndFeel(&lnf);
    meter.setMeterSource(&meterSource);
}

void Recorder::start(const juce::File& file)
{

    stop();

    if (actualSampleRate > 0)
    {
        // Create an OutputStream to write to our destination file...
        file.deleteFile();

        if (auto fileStream = std::unique_ptr<juce::FileOutputStream>(file.createOutputStream()))
        {
            // Now create a WAV writer object that writes to our output stream...
            juce::WavAudioFormat wavFormat;

            if (auto writer = wavFormat.createWriterFor(fileStream.get(), 48000, 2, 16, {}, 0))
            {
                fileStream.release(); // (passes responsibility for deleting the stream to the writer object that is now using it)

                // Now we'll create one of these helper objects which will act as a FIFO buffer, and will
                // write the data to disk on our background thread.
                threadedWriter.reset(new juce::AudioFormatWriter::ThreadedWriter(writer, backgroundThread, 32768));

                // Reset our recording thumbnail
                thumbnail.reset(writer->getNumChannels(), writer->getSampleRate());
                nextSampleNum = 0;

                // And now, swap over our active writer pointer so that the audio callback will start using it..
                const juce::ScopedLock sl(writerLock);
                activeWriter = threadedWriter.get();
            }
        }
    }

}

void Recorder::stop()
{
    // First, clear this pointer to stop the audio callback from using our writer object..
    {
        const juce::ScopedLock sl(writerLock);
        activeWriter = nullptr;
    }

    // Now we can delete the writer object. It's done in this order because the deletion could
    // take a little time while remaining data gets flushed to disk, so it's best to avoid blocking
    // the audio callback while this happens.
    threadedWriter.reset();
}

bool Recorder::isRecording()
{
    return activeWriter.load() != nullptr;
}

void Recorder::startRecording()
{
    if (!juce::RuntimePermissions::isGranted(juce::RuntimePermissions::writeExternalStorage))
    {
        SafePointer<Recorder> safeThis(this);

        juce::RuntimePermissions::request(juce::RuntimePermissions::writeExternalStorage,
            [safeThis](bool granted) mutable
            {
                if (granted)
                    safeThis->startRecording();
            });
        return;
    }


    auto parentDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);

    lastRecording = parentDir.getNonexistentChildFile("JUCE Demo Audio Recording", ".wav");

    start(lastRecording);

    recordButton.setButtonText("Stop");
    //recordingThumbnail.setDisplayFullThumbnail(false);
}

void Recorder::stopRecording()
{
    stop();

#if JUCE_CONTENT_SHARING
    SafePointer<AudioRecordingDemo> safeThis(this);
    File fileToShare = lastRecording;

    ContentSharer::getInstance()->shareFiles(Array<URL>({ URL(fileToShare) }),
        [safeThis, fileToShare](bool success, const String& error)
        {
            if (fileToShare.existsAsFile())
                fileToShare.deleteFile();

            if (!success && error.isNotEmpty())
            {
                NativeMessageBox::showMessageBoxAsync(AlertWindow::WarningIcon,
                    "Sharing Error",
                    error);
            }
        });
#endif

    lastRecording = juce::File();
    recordButton.setButtonText("Record");
    //recordingThumbnail.setDisplayFullThumbnail(true);
}

void Recorder::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &thumbnail)
        repaint();
}