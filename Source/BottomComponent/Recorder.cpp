/*
  ==============================================================================

    Recorder.cpp
    Created: 16 Mar 2021 10:05:40am
    Author:  Lucien Lefebvre

  ==============================================================================
*/
#define JUCE_USE_LAME_AUDIO_FORMAT 1
#include <JuceHeader.h>
#include "Recorder.h"

using namespace std;

//==============================================================================
Recorder::Recorder() : editingThumbnailCache(5), editingThumbnail(521, editingFormatManager, editingThumbnailCache)

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


    editingThumbnail.addChangeListener(this);

    addChildComponent(&timeLabel);
    timeLabel.setText(TRANS("0"), juce::NotificationType::dontSendNotification);
    timeLabel.setJustificationType(juce::Justification::centred);
    timeLabel.setFont(juce::Font(35.00f, juce::Font::plain).withTypefaceStyle("Regular"));
    timeLabel.setWantsKeyboardFocus(false);

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
    backgroundThread.stopThread(1000);
    thumbnail.clear();
    editingThumbnail.clear();
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
            thumbnail.drawChannels(g, thumbnailBounds, max(thumbnail.getTotalLength() - 10, (double)0), thumbnail.getTotalLength(), 1);

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
                2);

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
    const juce::ScopedLock sl(writerLock);

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


    //copy the buffer comming from the soundplayer
    recordingSoundBuffer.copyFrom(0, 0, *soundBuffer, 0, 0, actualSamplesPerBlockExpected);
    if (numChannels == 2)
    recordingSoundBuffer.copyFrom(1, 0, *soundBuffer, 1, 0, actualSamplesPerBlockExpected);

    //add them to the recording buffer and apply gain
    juce::AudioBuffer<float> recordingBuffer(2, actualSamplesPerBlockExpected);
    recordingBuffer.clear();
    recordingBuffer.addFrom(0, 0, recordingMicBuffer, 0, 0, actualSamplesPerBlockExpected, juce::Decibels::decibelsToGain(micSlider.getValue()));
    if (numChannels == 2)
    recordingBuffer.addFrom(1, 0, recordingMicBuffer, 1, 0, actualSamplesPerBlockExpected, juce::Decibels::decibelsToGain(micSlider.getValue()));
    recordingBuffer.addFrom(0, 0, recordingSoundBuffer, 0, 0, actualSamplesPerBlockExpected, juce::Decibels::decibelsToGain(soundSlider.getValue()));
    if (numChannels == 2)
    recordingBuffer.addFrom(1, 0, recordingSoundBuffer, 1, 0, actualSamplesPerBlockExpected, juce::Decibels::decibelsToGain(soundSlider.getValue()));

    if (activeWriter.load() != nullptr)//write
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
    cueTransport.setSource(nullptr);
    thumbnail.clear();
    editingThumbnail.clear();
    editingThumbnail.setSource(nullptr);
    editingThumbnailCache.clear();

    recordButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::red);

    if (actualSampleRate > 0)
    {   
        file.deleteFile();

        if (auto fileStream = std::unique_ptr<juce::FileOutputStream>(file.createOutputStream()))
        {
            juce::WavAudioFormat wavFormat;

            if (auto writer = wavFormat.createWriterFor(fileStream.get(), actualSampleRate, numChannels, 24, {}, 0))
            {
                fileStream.release(); 

                threadedWriter.reset(new juce::AudioFormatWriter::ThreadedWriter(writer, backgroundThread, 32768));

                thumbnail.reset(writer->getNumChannels(), writer->getSampleRate());
                nextSampleNum = 0;

                const juce::ScopedLock sl(writerLock);
                activeWriter = threadedWriter.get();
            }
        }
    }

}

void Recorder::stop()
{
    recordButton.setColour(juce::TextButton::ColourIds::buttonColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    {
        const juce::ScopedLock sl(writerLock);
        activeWriter = nullptr;
    }

    threadedWriter.reset();
    saveButton.setEnabled(true);
    enabledButton.setEnabled(true);
    fileRecorded = true;
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

    if (cueTransport.isPlaying())
    {
        cueTransport.stop();
        cueButton.setButtonText("Play");
        cueButton.setColour(juce::TextButton::ColourIds::buttonColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    }

    start(lastRecording);
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

    recordButton.setButtonText("Record");
}

void Recorder::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &editingThumbnail)
    {
        repaint();
    }
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
    juce::String micAutorisation = juce::WindowsRegistry::getValue("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\CapabilityAccessManager\\ConsentStore\\microphone\\VALUE");
    if (micAutorisation.equalsIgnoreCase("Deny"))
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, "Mic access needed",
            "Windows doesn't allow access to the microphone. "
            "To fix that, go to Windows settings, search \"Microphone privacy settings\", "
            "and turn on \"Allow apps to access your microphone\" (restart needed");
    }

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
    if (!fileRecorded)
    meter.setVisible(enabledButton.getToggleState());
    marksLabel.setVisible(enabledButton.getToggleState());
    repaint();
}

void Recorder::timerCallback()
{

    juce::String markIn = secondsToMMSS((int)startTime);
    juce::String markOut = secondsToMMSS((int)stopTime);
    juce::String lenght = secondsToMMSS(stopTime - startTime);
    timeLabel.setText(secondsToMMSS(thumbnail.getTotalLength()), juce::NotificationType::dontSendNotification);

    if (startTimeSet || stopTimeSet)
    {
        if (!stopTimeSet)
            stopTime = (float)thumbnail.getTotalLength();
        marksLabel.setText(juce::String("In Mark : " + markIn + " // Out Mark : " + markOut + " // Lenght : " + lenght), juce::NotificationType::dontSendNotification);
    }
        else if (cueTransport.isPlaying())
    {
        DBG(cueTransport.getCurrentPosition());
        if (cueTransport.getCurrentPosition() > stopTime)
            cueTransport.stop();
    }
    if (fileRecorded)
        timeLabel.setText(secondsToMMSS(cueTransport.getCurrentPosition()), juce::NotificationType::dontSendNotification);

    repaint();
}


void Recorder::saveButtonClicked()
{
    if (formatBox.getSelectedId() == 1)
        chooser.reset(new juce::FileChooser("Choose an audio File", juce::File::getSpecialLocation(juce::File::userDesktopDirectory), "*.wav"));
    else if (formatBox.getSelectedId() == 2)
        chooser.reset(new juce::FileChooser("Choose an audio File", juce::File::getSpecialLocation(juce::File::userDesktopDirectory), "*.mp3"));
    if (chooser->browseForFileToSave(true))
    {
            juce::File myFile;
            myFile = chooser->getResult();

            if (outputformatManager.getNumKnownFormats() == 0)
                outputformatManager.registerBasicFormats();
            std::unique_ptr<juce::AudioFormatReader> reader;
            reader.reset(outputformatManager.createReaderFor(fileToSave));
            reader.get()->numChannels = numChannels;
            int startSample = startTime * actualSampleRate;
            int endSample = stopTime * actualSampleRate;
            int numSamples = endSample - startSample;

            juce::AudioSampleBuffer* buffer = new juce::AudioSampleBuffer(numChannels, numSamples);

            buffer->clear();

            if (numChannels == 2)
            {
                reader->read(buffer, 0, numSamples, startSample, true, true);
            }
            else if (numChannels == 1)
            {
                reader->read(buffer, 0, numSamples, startSample, true, false);
            }
            juce::WavAudioFormat audioFormat;

            auto parentDir = juce::File(Settings::convertedSoundsPath);
            const juce::File tempFile(parentDir.getNonexistentChildFile("LuPlayerRecording", ".wav"));

            juce::LAMEEncoderAudioFormat compressedAudioFormat(juce::String(juce::File::getCurrentWorkingDirectory().getFullPathName() + "\\lame.exe"));
            auto fileStream = std::unique_ptr<juce::FileOutputStream>(myFile.createOutputStream());
            std::unique_ptr<juce::AudioFormatWriter> outputWriter;


            if (formatBox.getSelectedId() == 1)
            {

                if (numChannels == 1)
                    outputWriter.reset(audioFormat.createWriterFor(fileStream.get(), reader->sampleRate, 1, (int)reader->bitsPerSample, juce::StringPairArray(), 0));
                else if (numChannels == 2)
                    outputWriter.reset(audioFormat.createWriterFor(fileStream.get(), reader->sampleRate, 2, (int)reader->bitsPerSample, juce::StringPairArray(), 0));
                fileStream.release();
                if (outputWriter->writeFromAudioSampleBuffer(*buffer, 0, numSamples))
                {
                    std::unique_ptr<juce::AlertWindow> exportWindow;
                    exportWindow->showMessageBox(juce::AlertWindow::AlertIconType::InfoIcon, "File exported", "");
                }
                else
                {
                    std::unique_ptr<juce::AlertWindow> exportWindow;
                    exportWindow->showMessageBox(juce::AlertWindow::AlertIconType::WarningIcon, "", "Error exporting file");
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
                    std::unique_ptr<juce::AlertWindow> exportWindow;
                    exportWindow->showMessageBox(juce::AlertWindow::AlertIconType::InfoIcon, "File exported", "");
                    outputWriter->flush();
                }
                else
                {
                    std::unique_ptr<juce::AlertWindow> exportWindow;
                    exportWindow->showMessageBox(juce::AlertWindow::AlertIconType::WarningIcon, "", "Error exporting file");
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
        std::unique_ptr<juce::AudioFormatReaderSource> cuetempSource(new juce::AudioFormatReaderSource(cuereader, true));
        cueTransport.setSource(cuetempSource.get());
        cueTransport.addChangeListener(this);
        cueTransport.setGain(1.0);
        cueButton.setEnabled(true);
        cueTransport.setPosition(0);
        readerSource.reset(cuetempSource.release());
        fileRecorded = true;
        thumbnail.setSource(nullptr);
        editingThumbnail.setSource(new juce::FileInputSource(file));
        if (!stopTimeSet)
        stopTime = (float)cueTransport.getLengthInSeconds();
        DBG(stopTime);
        repaint();
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

void Recorder::setOrDeleteStart(bool setOrDelete)
{
    if (setOrDelete)
        setStart();
    else
        deleteStart();
}

void Recorder::setStart()
{
    if (isVisible())
    {
        if (!isRecording())
        {
            if ((float)cueTransport.getCurrentPosition() < stopTime && (float)cueTransport.getCurrentPosition() > 0)
            {
                startTime = (float)cueTransport.getCurrentPosition();
                startTimeSet = true;
            }
        }
        else
        {
            startTime = thumbnail.getTotalLength();
            startTimeSet = true;
        }
    }

}

void Recorder::setOrDeleteStop(bool setOrDelete)
{
    if (setOrDelete)
        setStop();
    else
        deleteStop();
}

void Recorder::setStop()
{
    if (isVisible())
    {
        if (!isRecording())
        {
            if ((float)cueTransport.getCurrentPosition() > startTime && (float)cueTransport.getCurrentPosition() > 0)
            {
                stopTime = (float)cueTransport.getCurrentPosition();
                stopTimeSet = true;
            }
        }
        else
        {
            stopTime = thumbnail.getTotalLength();
            stopTimeSet = true;
        }
    }
}

void Recorder::deleteStart()
{
    startTime = 0;
    startTimeSet = false;
}

void Recorder::deleteStop()
{
    stopTime = (float)cueTransport.getTotalLength();
    stopTimeSet = false;
}

bool Recorder::keyPressed(const juce::KeyPress& key, KeyMapper* keyMapper)
{
    int keyCode = key.getKeyCode();
    if (keyMapper->getKeyMapping(7) == keyCode)
    {
        setStart();
        return true;
    }
    else if (keyMapper->getKeyMapping(8) == keyCode)
    {
        deleteStart();
        return true;
    }
    else if (keyMapper->getKeyMapping(9) == keyCode)
    {
        setStop();
        return true;
    }
    else if (keyMapper->getKeyMapping(10) == keyCode)
    {
        deleteStop();
        return true;
    }
    else if (keyMapper->getKeyMapping(4) == keyCode)
    {
        cueButtonClicked();
        return true;
    }
    else
        return false;
}

void Recorder::cueButtonClicked()
{
    if (!isRecording())
    {
        if (cueTransport.isPlaying())
        {
            cueTransport.stop();
            cueButton.setButtonText("Play");
            cueButton.setColour(juce::TextButton::ColourIds::buttonColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        }
        else
        {
            cueTransport.start();
            cueButton.setButtonText("Stop");
            cueButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::green);
        }
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

