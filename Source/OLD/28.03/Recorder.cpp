/*
  ==============================================================================

    Recorder.cpp
    Created: 16 Mar 2021 10:05:40am
    Author:  Lucien

  ==============================================================================
*/
#define JUCE_USE_LAME_AUDIO_FORMAT 1
#include <JuceHeader.h>
#include "Recorder.h"
#include "Settings.h"
//==============================================================================
Recorder::Recorder() : editingThumbnailCache(5), editingThumbnail(521, editingFormatManager, thumbnailCache)

{
    backgroundThread.startThread();
    juce::Timer::startTimer(50);
    editingFormatManager.registerBasicFormats();
    setWantsKeyboardFocus(false);
    
    mouseDragInRecorder = new juce::ChangeBroadcaster();
    spaceBarKeyPressed = new juce::ChangeBroadcaster();
    recordingBroadcaster = new juce::ChangeBroadcaster();

    addChildComponent(&recordButton);
    thumbnail.addChangeListener(this);

    addChildComponent(&enableMonitoring);
    enableMonitoring.setToggleState(false, juce::NotificationType::dontSendNotification);
    enableMonitoring.setButtonText("Monitor Recorder");

    addChildComponent(&saveButton);
    saveButton.onClick = [this] {saveButtonClicked(); };

    addChildComponent(&formatBox);
    formatBox.addItem("WAV", 1);
    formatBox.addItem("MP3", 2);
    formatBox.setSelectedId(1, juce::NotificationType::dontSendNotification);

    if (Settings::audioOutputMode == 2
        || Settings::audioOutputMode == 3)
        numChannels = 2;
    else if (Settings::audioOutputMode == 1)
        numChannels = 1;

    Settings::audioOutputModeValue.addListener(this);

    addChildComponent(&micSlider);
    micSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    micSlider.setRange(-100, +12);
    micSlider.setValue(0);
    micSlider.setSkewFactor(3.5, false);
    micSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 15);
    micSlider.setWantsKeyboardFocus(false);
    micSlider.setNumDecimalPlacesToDisplay(0);
    micSlider.setEnabled(false);
    micSlider.setDoubleClickReturnValue(true, 0);
    micSlider.setColour(juce::Slider::ColourIds::textBoxOutlineColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    addChildComponent(&soundSlider);
    soundSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    soundSlider.setRange(-100, +12);
    soundSlider.setValue(0);
    soundSlider.setSkewFactor(3.5, false);
    soundSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 15);
    soundSlider.setWantsKeyboardFocus(false);
    soundSlider.setNumDecimalPlacesToDisplay(0);
    soundSlider.setEnabled(false);
    soundSlider.setDoubleClickReturnValue(true, 0);
    soundSlider.setColour(juce::Slider::ColourIds::textBoxOutlineColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    addChildComponent(&micLabel);
    micLabel.setText("Mic Level", juce::NotificationType::dontSendNotification);
    micLabel.setJustificationType(juce::Justification::centred);

    addChildComponent(&soundLabel);
    soundLabel.setText("Sound Level", juce::NotificationType::dontSendNotification);
    soundLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(&enabledButton);
    enabledButton.onClick = [this] { enableButtonClicked(); };
    enabledButton.setToggleState(false, juce::NotificationType::dontSendNotification);
    enabledButton.setButtonText("Enable Recorder");

    addChildComponent(&cueButton);
    cueButton.onClick = [this] {cueButtonClicked(); };
    cueButton.setEnabled(false);
    cueButton.setButtonText("Play");

    cueTransport.addChangeListener(this);

    recordButton.setEnabled(false);
    recordButton.onClick = [this]
    {
        recordButtonClicked();
    };




    addChildComponent(&timeLabel);
    timeLabel.setText(TRANS("0"), juce::NotificationType::dontSendNotification);
    timeLabel.setJustificationType(juce::Justification::centred);
    timeLabel.setFont(juce::Font(35.00f, juce::Font::plain).withTypefaceStyle("Regular"));
    timeLabel.setWantsKeyboardFocus(false);

    //juce::AudioThumbnail& getAudioThumbnail() { return thumbnail; }

    addChildComponent(meter);
    lnf.setColour(foleys::LevelMeter::lmBackgroundColour, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    lnf.setColour(foleys::LevelMeter::lmMeterGradientLowColour, juce::Colours::green);
    lnf.setColour(foleys::LevelMeter::lmMeterGradientMaxColour, juce::Colours::red);
    lnf.setColour(foleys::LevelMeter::lmMeterGradientMidColour, juce::Colours::orange);

    addAndMakeVisible(marksLabel);
    marksLabel.setSize(100, 25);
    marksLabel.setFont(juce::Font(20.0f, juce::Font::bold).withTypefaceStyle("Regular"));
    marksLabel.setColour(juce::Label::ColourIds::textColourId, juce::Colours::white);
    marksLabel.setColour(juce::Label::ColourIds::backgroundColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    marksLabel.setAlpha(0.7);
}

Recorder::~Recorder()
{
    lastRecording.deleteFile();
    cueTransport.setSource(nullptr);
    delete mouseDragInRecorder;
    delete spaceBarKeyPressed;
    delete recordingBroadcaster;
}

void Recorder::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background


    if (isEnabled())
    {
        g.setColour(juce::Colour(40, 134, 189));
        g.fillRect(leftControlWidth - 4, 0, 4, getHeight());



        if (!fileRecorded)////RECORDING THUMBNAIL
        {
            g.fillRect(thumbnailBounds.getWidth() + leftControlWidth, 0, 4, getHeight());
            //DRAW THUMBNAIL
            thumbnail.drawChannels(g, thumbnailBounds, std::max(thumbnail.getTotalLength() - 10, (double)0), thumbnail.getTotalLength(), 1);


            //DRAW LU METER
            if (Settings::audioOutputMode == 2 || Settings::audioOutputMode == 3)
                shortTermLoudness = loudnessMeter.getMomentaryLoudness();
            else if (Settings::audioOutputMode == 1)
            {
                shortTermLoudness = loudnessMeter.getMomentaryLoudnessForIndividualChannels()[0];
            }
            if (shortTermLoudness > 1)
                shortTermLoudness = 1;
            juce::NormalisableRange<float>loudnessMeterRange(-300, 1, 0.001, 3, false);
            float loudnessNormalized = loudnessMeterRange.convertTo0to1(shortTermLoudness);
            int loudnessMeterWidth = loudnessNormalized * (meter.getWidth() - 67 - 15);
            if (shortTermLoudness >= -23.)
                g.setColour(juce::Colours::orange);
            else
                g.setColour(juce::Colour(40, 134, 189));

            g.fillRoundedRectangle(meter.getBounds().getTopLeft().getX() + 67, meter.getBounds().getBottomLeft().getY() + 10, loudnessMeterWidth, 25, 5);
            if (shortTermLoudness >= -23.)
            {
                DBG("draw");
                float minusTwentyThreeDbLoudness = 0.779363;
                int loudnessMeterWidthMinusEighteen = minusTwentyThreeDbLoudness * ( meter.getWidth() - 67 - 15);
                g.setColour(juce::Colour(40, 134, 189));
                g.fillRoundedRectangle(meter.getBounds().getTopLeft().getX() + 67, meter.getBounds().getBottomLeft().getY() + 10, loudnessMeterWidthMinusEighteen, 25, 5);
            }
        }
        else
        {
            //EDITING THUMBNAIL
            editingThumbnail.drawChannels(g,
                editingThumbnailBounds,
                0,                                    // start time
                (float)editingThumbnail.getTotalLength(),             // end time
                1);

            g.setColour(juce::Colours::black);
            auto cueaudioPosition = (float)cueTransport.getCurrentPosition();
            auto cuecurrentPosition = cueTransport.getLengthInSeconds();
            auto cuedrawPosition = ((cueaudioPosition / cuecurrentPosition) * (float)editingThumbnailBounds.getWidth())
                + (float)editingThumbnailBounds.getX();
            if (cuedrawPosition > 340)
                g.drawLine(cuedrawPosition, (float)editingThumbnailBounds.getY(), cuedrawPosition,
                    (float)editingThumbnailBounds.getBottom(), 2.0f);


            if (startTimeSet)
            {
                g.setColour(juce::Colour(0, 196, 255));
                auto startDrawPosition = (startTime * (float)editingThumbnailBounds.getWidth() / cueTransport.getLengthInSeconds()) + (float)editingThumbnailBounds.getX();
                if (startDrawPosition > waveformThumbnailXStart)
                    g.drawLine(startDrawPosition, (float)editingThumbnailBounds.getY(), startDrawPosition,
                        (float)editingThumbnailBounds.getBottom(), 2.0f);
            }
            if (stopTimeSet)
            {
                g.setColour(juce::Colour(238, 255, 0));
                auto startDrawPosition = (stopTime * (float)editingThumbnailBounds.getWidth() / cueTransport.getLengthInSeconds()) + (float)editingThumbnailBounds.getX();
                if (startDrawPosition > waveformThumbnailXStart && startDrawPosition)
                    g.drawLine(startDrawPosition, (float)editingThumbnailBounds.getY(), startDrawPosition,
                        (float)editingThumbnailBounds.getBottom(), 2.0f);
            }
        }
        g.setColour(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        g.fillRect(0, 0, leftControlWidth - 4, getHeight());
    }
    else
        g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));   // clear the background
}

void Recorder::resized()
{
    enabledButton.setBounds(0, 0, 150, 25);
    recordButton.setBounds(0, 60, 125, 25);
    saveButton.setBounds(0, 120, 55, 25);
    formatBox.setBounds(57, 120, 68, 25);
    meter.setBounds((getWidth() - leftControlWidth)/2 + leftControlWidth + 4, getHeight()/2 - 60, - 4 + (getWidth() - leftControlWidth) / 2, 80);
    thumbnailBounds.setBounds(leftControlWidth, 0, (getWidth() - leftControlWidth) / 2, getHeight());
    editingThumbnailBounds.setBounds(leftControlWidth, 0, getWidth() - leftControlWidth, getHeight());
    micSlider.setBounds(150, 0, 50, getHeight() - 25);
    soundSlider.setBounds(250, 0, 50, getHeight() - 25);
    micLabel.setSize(200, 25);
    micLabel.setCentrePosition(175, getHeight() - 12);
    soundLabel.setSize(200, 25);
    soundLabel.setCentrePosition(275, getHeight() - 12);
    cueButton.setBounds(0, 90, 125, 25);
    timeLabel.setBounds(0, 150, 125, 40);
    enableMonitoring.setBounds(0, 30, 125, 25);
    marksLabel.setBounds(leftControlWidth, getHeight() - 25, 400, 25);

    waveformThumbnailXSize = getWidth() - leftControlWidth;
    waveformThumbnailXEnd = getWidth();
}


void Recorder::recordButtonClicked()
{
    if (isRecording())
        stopRecording();
    else
        startRecording();
}


void Recorder::recordAudioBuffer(juce::AudioBuffer<float>* soundBuffer, juce::AudioBuffer<float>* micBuffer, juce::AudioBuffer<float>* outputBuffer, int channels, double sampleRate, int samplesPerBlockExpected)
{

    //int numSamples = bufferToRecord->getNumSamples();
    //int numSamples = sourceToRecord. getNumSamples();
    const juce::ScopedLock sl(writerLock);

    //    if (activeWriter.load() != nullptr && channels >= thumbnail.getNumChannels())


        //create two audio buffers
        juce::AudioBuffer<float> recordingMicBuffer(numChannels, actualSamplesPerBlockExpected);
        juce::AudioBuffer<float> recordingSoundBuffer(numChannels, actualSamplesPerBlockExpected);
        recordingSoundBuffer.clear();
        recordingMicBuffer.clear();
        //Copy from the original buffer
        auto* inBufferL = micBuffer->getReadPointer(0);
        auto* inBufferR = micBuffer->getReadPointer(1);
        recordingMicBuffer.copyFrom(0, 0, inBufferL, actualSamplesPerBlockExpected);
        if (numChannels == 2)
        recordingMicBuffer.copyFrom(1, 0, inBufferR, actualSamplesPerBlockExpected);
        //recordingMicBuffer.copyFrom(1, 0, *micBuffer, 1, 0, actualSamplesPerBlockExpected);



        recordingSoundBuffer.copyFrom(0, 0, *soundBuffer, 0, 0, actualSamplesPerBlockExpected);
        if (numChannels == 2)
        recordingSoundBuffer.copyFrom(1, 0, *soundBuffer, 1, 0, actualSamplesPerBlockExpected);

        //add them to the recording buffer
        juce::AudioBuffer<float> recordingBuffer(2, actualSamplesPerBlockExpected);
        recordingBuffer.clear();
        recordingBuffer.addFrom(0, 0, recordingMicBuffer, 0, 0, actualSamplesPerBlockExpected, juce::Decibels::decibelsToGain(micSlider.getValue()));
        if (numChannels == 2)
        recordingBuffer.addFrom(1, 0, recordingMicBuffer, 1, 0, actualSamplesPerBlockExpected, juce::Decibels::decibelsToGain(micSlider.getValue()));
        recordingBuffer.addFrom(0, 0, recordingSoundBuffer, 0, 0, actualSamplesPerBlockExpected, juce::Decibels::decibelsToGain(soundSlider.getValue()));
        if (numChannels == 2)
        recordingBuffer.addFrom(1, 0, recordingSoundBuffer, 1, 0, actualSamplesPerBlockExpected, juce::Decibels::decibelsToGain(soundSlider.getValue()));

        if (activeWriter.load() != nullptr)
        {
            activeWriter.load()->write(recordingBuffer.getArrayOfReadPointers(), recordingBuffer.getNumSamples());

            // Create an AudioBuffer to wrap our incoming data, note that this does no allocations or copies, it simply references our input data
            juce::AudioBuffer<float> buffer(const_cast<float**> (recordingBuffer.getArrayOfReadPointers()), thumbnail.getNumChannels(), recordingBuffer.getNumSamples());
            thumbnail.addBlock(nextSampleNum, buffer, 0, recordingBuffer.getNumSamples());
            nextSampleNum += recordingBuffer.getNumSamples();
        }

        meterSource.measureBlock(recordingBuffer);
        loudnessMeter.processBlock(recordingBuffer);

        outputBuffer->copyFrom(0, 0, recordingBuffer, 0, 0, actualSamplesPerBlockExpected);
        if (numChannels == 2)
            outputBuffer->copyFrom(1, 0, recordingBuffer, 1, 0, actualSamplesPerBlockExpected);




}

void Recorder::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    actualSampleRate = sampleRate;
    actualSamplesPerBlockExpected = samplesPerBlockExpected;
   
    meterSource.resize(2, sampleRate * 0.1 / samplesPerBlockExpected);
    meter.setLookAndFeel(&lnf);
    meter.setMeterSource(&meterSource);
    cueTransport.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void Recorder::start(const juce::File& file)
{

    stop();
    deleteStart();
    deleteStop();
    marksLabel.setText("", juce::NotificationType::dontSendNotification);
    recordingBroadcaster->sendChangeMessage();
    saveButton.setEnabled(false);
    cueButton.setEnabled(false);
    enabledButton.setEnabled(false);
    meter.setVisible(true);
    fileRecorded = false;
    lastRecording.deleteFile();
    recordButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::red);
    if (actualSampleRate > 0)
    {
        // Create an OutputStream to write to our destination file...
        file.deleteFile();

        if (auto fileStream = std::unique_ptr<juce::FileOutputStream>(file.createOutputStream()))
        {
            // Now create a WAV writer object that writes to our output stream...
            juce::WavAudioFormat wavFormat;

            if (auto writer = wavFormat.createWriterFor(fileStream.get(), actualSampleRate, numChannels, 24, {}, 0))
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
    recordButton.setColour(juce::TextButton::ColourIds::buttonColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    {
        const juce::ScopedLock sl(writerLock);
        activeWriter = nullptr;
    }

    // Now we can delete the writer object. It's done in this order because the deletion could
    // take a little time while remaining data gets flushed to disk, so it's best to avoid blocking
    // the audio callback while this happens.
    threadedWriter.reset();
    saveButton.setEnabled(true);
    enabledButton.setEnabled(true);
    recordingBroadcaster->sendChangeMessage();
    meter.setVisible(false);
}

bool Recorder::isRecording()
{
    return activeWriter.load() != nullptr;
}

void Recorder::startRecording()
{
    lastRecording.deleteFile();
    if (fileRecorded)

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


    auto parentDir = juce::File(Settings::convertedSoundsPath);

    lastRecording = parentDir.getNonexistentChildFile("Temp Recording File", ".wav");

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
    fileToSave = lastRecording;
    loadFile(fileToSave.getFullPathName());
    //lastRecording = juce::File();

    recordButton.setButtonText("Record");
    //recordingThumbnail.setDisplayFullThumbnail(true);
}

void Recorder::changeListenerCallback(juce::ChangeBroadcaster* source)
{
   /* if (source == &thumbnail)
        repaint();*/
    if (source == &cueTransport)
    {
        if (cueTransport.isPlaying())
        {

        }
        else
        {
            cueTransport.setPosition(0);
            cueButton.setButtonText("Play");
            cueButton.setColour(juce::TextButton::ColourIds::buttonColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        }
    }
}

bool Recorder::isEnabled()
{
    return isRecorderEnabled;
}

void Recorder::enableButtonClicked()
{
    isRecorderEnabled = enabledButton.getToggleState();
    enableMonitoring.setVisible(enabledButton.getToggleState());
    micSlider.setVisible(enabledButton.getToggleState());
    soundSlider.setVisible(enabledButton.getToggleState());
    recordButton.setVisible(enabledButton.getToggleState());
    saveButton.setVisible(enabledButton.getToggleState());
    micLabel.setVisible(enabledButton.getToggleState());
    soundLabel.setVisible(enabledButton.getToggleState());
    timeLabel.setVisible(enabledButton.getToggleState());
    cueButton.setVisible(enabledButton.getToggleState());
    recordButton.setEnabled(enabledButton.getToggleState());
    saveButton.setEnabled(false);
    formatBox.setVisible(enabledButton.getToggleState());
    micSlider.setEnabled(enabledButton.getToggleState());
    soundSlider.setEnabled(enabledButton.getToggleState());
    meter.setVisible(enabledButton.getToggleState());
    marksLabel.setVisible(enabledButton.getToggleState());
    repaint();
}

void Recorder::timerCallback()
{
    timeLabel.setText(secondsToMMSS(thumbnail.getTotalLength()), juce::NotificationType::dontSendNotification);
    juce::String markIn = secondsToMMSS((int)startTime);
    juce::String markOut = secondsToMMSS((int)stopTime);
    juce::String lenght = secondsToMMSS(stopTime - startTime);
    if (isRecording() || fileRecorded)
    {
        if (startTimeSet || stopTimeSet)
        {
            if (!stopTimeSet)
                stopTime = (float)thumbnail.getTotalLength();
            marksLabel.setText(juce::String("In Mark : " + markIn + " // Out Mark : " + markOut + " // Lenght : " + lenght), juce::NotificationType::dontSendNotification);
        }
    }

    repaint();
}


void Recorder::saveButtonClicked()
{

    //juce::FileChooser chooser("Choose an audio File", juce::File::getSpecialLocation(juce::File::userDesktopDirectory), "*.wav");
    if (formatBox.getSelectedId() == 1)
        chooser.reset(new juce::FileChooser("Choose an audio File", juce::File::getSpecialLocation(juce::File::userDesktopDirectory), "*.wav"));
    else if (formatBox.getSelectedId() == 2)
        chooser.reset(new juce::FileChooser("Choose an audio File", juce::File::getSpecialLocation(juce::File::userDesktopDirectory), "*.mp3"));
    if (chooser->browseForFileToSave(true))
    {
        juce::File myFile;
        myFile = chooser->getResult();
        //fileToSave.copyFileTo(myFile);

        if (outputformatManager.getNumKnownFormats() == 0)
            outputformatManager.registerBasicFormats();
        std::unique_ptr<juce::AudioFormatReader> reader;
        reader.reset(outputformatManager.createReaderFor(fileToSave));
        reader.get()->numChannels = numChannels;
        int startSample = startTime * actualSampleRate;
        int endSample = stopTime * actualSampleRate;
        int numSamples = endSample - startSample;
        //int numChannels = 2;

        DBG(numChannels);
        juce::AudioSampleBuffer* buffer = new juce::AudioSampleBuffer(numChannels, numSamples);

        buffer->clear();

        if (numChannels == 2)
        {
            reader->read(buffer, 0, numSamples, startSample, true, false);
            reader->read(buffer, 0, numSamples, startSample, false, true);
        }
        else if (numChannels == 1)
        {
            reader->read(buffer, 0, numSamples, startSample, true, false);
        }
        juce::WavAudioFormat audioFormat;

        auto parentDir = juce::File(Settings::convertedSoundsPath);
        const juce::File tempFile(parentDir.getNonexistentChildFile("MultiPlayerRecording", ".wav"));
        //tempFile.deleteFile();

        juce::LAMEEncoderAudioFormat compressedAudioFormat(juce::String(juce::File::getCurrentWorkingDirectory().getFullPathName() + "\\lame.exe"));
        auto fileStream = std::unique_ptr<juce::FileOutputStream>(myFile.createOutputStream());
        std::unique_ptr<juce::AudioFormatWriter> outputWriter;


        for (auto bitrate : compressedAudioFormat.getQualityOptions())
            DBG(bitrate);
        for (auto samplesrate : compressedAudioFormat.getPossibleSampleRates())
            DBG(samplesrate);
        for (auto getFileExtensions : compressedAudioFormat.getFileExtensions())
            DBG(getFileExtensions);
        
        if (formatBox.getSelectedId() == 1)
        {

            if (numChannels == 1)
                outputWriter.reset(audioFormat.createWriterFor(fileStream.get(), reader->sampleRate, 1, (int)reader->bitsPerSample, juce::StringPairArray(), 0));
            else if (numChannels == 2)
                outputWriter.reset(audioFormat.createWriterFor(fileStream.get(), reader->sampleRate, 2, (int)reader->bitsPerSample, juce::StringPairArray(), 0));
            fileStream.release();
            if (outputWriter->writeFromAudioSampleBuffer(*buffer, 0, numSamples))
                {
                 DBG("file writed");
                }
        }
        else if (formatBox.getSelectedId() == 2)
        {
            if (numChannels == 1)
                outputWriter.reset(compressedAudioFormat.createWriterFor(fileStream.get(), reader->sampleRate, 1, (int)reader->bitsPerSample, juce::StringPairArray(), 23));
            else if (numChannels == 2)
                outputWriter.reset(compressedAudioFormat.createWriterFor(fileStream.get(), reader->sampleRate, 2, (int)reader->bitsPerSample, juce::StringPairArray(), 23));
            fileStream.release();
            if (outputWriter->writeFromAudioSampleBuffer(*buffer, 0, numSamples))
            {
                DBG("file writed mp3");
                outputWriter->flush();
            }  

        }
        delete buffer;
    }
}


juce::String Recorder::secondsToMMSS(int seconds)
{
    int timeSeconds = seconds % 60;
    int timeMinuts = trunc(seconds / 60);
    juce::String timeString;
    if (timeSeconds < 10)
        timeString = juce::String(timeMinuts) + ":0" + juce::String(timeSeconds);
    else
        timeString = juce::String(timeMinuts) + ":" + juce::String(timeSeconds);
    return timeString;
}

bool Recorder::loadFile(const juce::String& path)
{
    std::string loadedFilePath = path.toStdString();

    juce::File file(path);

    if (juce::AudioFormatReader* cuereader = editingFormatManager.createReaderFor(file))
    {
        //std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
        std::unique_ptr<juce::AudioFormatReaderSource> cuetempSource(new juce::AudioFormatReaderSource(cuereader, true));
        cueTransport.setSource(cuetempSource.get());
        cueTransport.addChangeListener(this);
        cueTransport.setGain(1.0);
        cueButton.setEnabled(true);
        cueTransport.setPosition(0);
        readerSource.reset(cuetempSource.release());
        fileRecorded = true;
        editingThumbnail.setSource(new juce::FileInputSource(file));
        if (!stopTimeSet)
        stopTime = (float)cueTransport.getLengthInSeconds();
        return true;
    }
    else
    {
        return false;
    }

}



void Recorder::mouseDown(const juce::MouseEvent& event)
{
    if (event.getNumberOfClicks() == 2)
    {
        thumbnailHorizontalZoom = 1;
        oldThumbnailOffset = 0;
        thumbnailOffset = 0;
    }
    thumbnailDragStart = getMouseXYRelative().getX();
    if (editingThumbnail.getNumChannels() != 0)
    {
        mouseDragXPosition = getMouseXYRelative().getX();
        if (mouseDragXPosition >= waveformThumbnailXStart && mouseDragXPosition <= waveformThumbnailXEnd)
        {
            mouseDragRelativeXPosition = getMouseXYRelative().getX() - editingThumbnailBounds.getPosition().getX();
            mouseDragInSeconds = (((float)mouseDragRelativeXPosition * cueTransport.getLengthInSeconds()) / (float)editingThumbnailBounds.getWidth());
            cueTransport.setPosition(mouseDragInSeconds);

        }
    }
    thumbnailMiddle = waveformThumbnailXSize / 2;
    thumbnailDrawStart = thumbnailMiddle - (thumbnailMiddle * thumbnailHorizontalZoom);
    thumbnailDrawEnd = thumbnailMiddle + (thumbnailMiddle * thumbnailHorizontalZoom);
    thumbnailDrawSize = thumbnailDrawEnd - thumbnailDrawStart;
    repaintThumbnail();
    mouseDragInRecorder->sendChangeMessage();
}

void Recorder::mouseDrag(const juce::MouseEvent& event)
{

    if (editingThumbnail.getNumChannels() != 0
        && !event.mods.isCtrlDown())
    {
        mouseDragXPosition = getMouseXYRelative().getX();
        if (mouseDragXPosition >= waveformThumbnailXStart && mouseDragXPosition <= waveformThumbnailXEnd)
        {
            mouseDragRelativeXPosition = getMouseXYRelative().getX() - editingThumbnailBounds.getPosition().getX();
            mouseDragInSeconds = (((float)mouseDragRelativeXPosition * cueTransport.getLengthInSeconds()) / (float)editingThumbnailBounds.getWidth());
            cueTransport.setPosition(mouseDragInSeconds);
        }
    }
    if (event.mods.isCtrlDown())
    {
        setMouseCursor(juce::MouseCursor::DraggingHandCursor);
        thumbnailMiddle = waveformThumbnailXSize / 2;
        thumbnailDrawStart = thumbnailMiddle - (thumbnailMiddle * thumbnailHorizontalZoom);
        thumbnailDrawEnd = thumbnailMiddle + (thumbnailMiddle * thumbnailHorizontalZoom);
        thumbnailDrawSize = thumbnailDrawEnd - thumbnailDrawStart;
        thumbnailOffset = oldThumbnailOffset + thumbnailDragStart - getMouseXYRelative().getX();
        repaintThumbnail();
    }
}

void Recorder::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
    if (event.mods.isCtrlDown())
    {
        thumbnailHorizontalZoom = thumbnailHorizontalZoom * (5 + wheel.deltaY) / 5;
        if (thumbnailHorizontalZoom < 1)
            thumbnailHorizontalZoom = 1;
        calculThumbnailBounds();
    }
}

void Recorder::mouseDoubleClick(const juce::MouseEvent& event)
{

}

void Recorder::calculThumbnailBounds()
{
    thumbnailMiddle = waveformThumbnailXSize / 2;
    thumbnailDrawStart = thumbnailMiddle - (thumbnailMiddle * thumbnailHorizontalZoom);
    thumbnailDrawEnd = thumbnailMiddle + (thumbnailMiddle * thumbnailHorizontalZoom);
    thumbnailDrawSize = thumbnailDrawEnd - thumbnailDrawStart;
    repaintThumbnail();
}

void Recorder::repaintThumbnail()
{
    editingThumbnailBounds.setBounds(leftControlWidth + thumbnailDrawStart - thumbnailOffset, 0, thumbnailDrawSize, getHeight());
    repaint();
}

void Recorder::mouseUp(const juce::MouseEvent& event)
{

    oldThumbnailOffset = thumbnailOffset;
    thumbnailDragStart = 0;
    setMouseCursor(juce::MouseCursor::NormalCursor);

}

void Recorder::setStart()
{
    if (!isRecording())
    {
        if ((float)cueTransport.getCurrentPosition() < stopTime && (float)cueTransport.getCurrentPosition() > 0)
        {
            startTime = (float)cueTransport.getCurrentPosition();
            startTimeSet = true;
            // startTimeButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colour(0, 196, 255));
        }
    }
    else
    {
        startTime = thumbnail.getTotalLength();
        startTimeSet = true;
    }

}

void Recorder::setStop()
{
    if (!isRecording())
    {
        if ((float)cueTransport.getCurrentPosition() > startTime && (float)cueTransport.getCurrentPosition() > 0)
        {
            stopTime = (float)cueTransport.getCurrentPosition();
            stopTimeSet = true;
            // stopTimeButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colour(238, 255, 0));
        }
    }
    else
    {
        stopTime = thumbnail.getTotalLength();
        stopTimeSet = true;
    }
}

void Recorder::deleteStart()
{
    startTime = 0;
    startTimeSet = false;
    //startTimeButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colour(0, 115, 150));
}

void Recorder::deleteStop()
{
    stopTime = (float)cueTransport.getTotalLength();
    stopTimeSet = false;
    //stopTimeButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colour(141, 150, 0));
}

bool Recorder::keyPressed(const juce::KeyPress& key)
{
    DBG("keypressed");
    if (key == 73)
    {
        setStart();
        return true;
    }
    else if (key == 75)
    {
        deleteStart();
        return true;
    }
    else if (key == 79)
    {
        setStop();
        return true;
    }
    else if (key == 76)
    {
        deleteStop();
        return true;
    }
    else if (key == 67)
    {
        cueButtonClicked();
        return true;
    }
    //else if (key == juce::KeyPress::spaceKey)
    //    spaceBarKeyPressed->sendChangeMessage();
    else
        return false;
}

void Recorder::cueButtonClicked()
{
    if (cueTransport.isPlaying())
    {
        cueTransport.stop();
        cueButton.setButtonText("Play");
        //enableMonitoring.setToggleState(true, juce::NotificationType::dontSendNotification);
        cueButton.setColour(juce::TextButton::ColourIds::buttonColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    }
    else
    {
        cueTransport.start();
        cueButton.setButtonText("Stop");
        //enableMonitoring.setToggleState(false, juce::NotificationType::dontSendNotification);
        cueButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::green);
    }
}

void Recorder::valueChanged(juce::Value& value)
{
    if (value.refersToSameSourceAs(Settings::audioOutputModeValue))
    {
        if (Settings::audioOutputMode == 2
            || Settings::audioOutputMode == 3)
        {
            numChannels = 2;

        }
        else if (Settings::audioOutputMode == 1)
        {
            numChannels = 1;
        }
    }
}

