/*
  ==============================================================================

    R128IntegratedThread.h
    Created: 6 Jan 2022 9:25:30pm
    Author:  Lucien Lefebvre

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

#include "../Settings/Settings.h"
#include "../Externals/LUFSMeter/Ebu128LoudnessMeter.h"
//==============================================================================
/*
*/
class R128IntegratedThread : public juce::Thread
{
public:
    R128IntegratedThread(const juce::String& threadName, size_t threadStackSize = 0) : Thread("R128 Integrated")
    {
        loudnessCalculatedBroadcaster = new juce::ChangeBroadcaster();
        formatManager.registerBasicFormats();
    }

    ~R128IntegratedThread() override
    {
        delete loudnessCalculatedBroadcaster;
        stopThread(1000);
    }

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate)
    {
        actualSamplesPerBlocksExpected = samplesPerBlockExpected;
        actualSampleRate = sampleRate;
        loudnessMeter.prepareToPlay(sampleRate, 2, samplesPerBlockExpected, 20);
    }

    void run()
    {
        if (juce::AudioFormatReader* reader = formatManager.createReaderFor(juce::File(filePath)))
        {
            readerSource.reset(new juce::AudioFormatReaderSource(reader, true));
            readerBuffer.reset(new juce::AudioBuffer<float>(reader->numChannels, actualSamplesPerBlocksExpected));
            sourceChannelInfo.reset(new juce::AudioSourceChannelInfo(*readerBuffer.get()));
            loudnessMeter.prepareToPlay(actualSampleRate, reader->numChannels, actualSamplesPerBlocksExpected, 20);
            juce::int64 numSamples = reader->lengthInSamples;
            juce::int64 remainingSamples = numSamples;
            while (remainingSamples > 0)
            {
                readerSource->getNextAudioBlock(*sourceChannelInfo.get());
                loudnessMeter.processBlock(*readerBuffer.get());
                remainingSamples -= actualSamplesPerBlocksExpected;
                progress = 1 - ((double)remainingSamples / (double)numSamples);
            }
            integratedLoudness = loudnessMeter.getIntegratedLoudness();
            if (reader->numChannels == 1)
                integratedLoudness += 3;
        }
        progress = -1;
        loudnessCalculatedBroadcaster->sendChangeMessage();
        loudnessMeter.reset();
    }

    void setFilePath(juce::String p)
    {
        filePath = p.toStdString();
        fileName = juce::File(p).getFileNameWithoutExtension().toStdString();
    }

    juce::String getFile()
    {
        return returnedFile;
    }

    double getILU()
    {
        return integratedLoudness;
    }

    void deleteProgressFile()
    {
        juce::File progressFile(progressFilePath);
        if (progressFile.exists())
            progressFile.deleteFile();
    }

    void killThread()
    {
        process.kill();
    }

    juce::ChildProcess process;
    std::string progressFilePath;
    std::string filePath;
    std::string fileName;
    juce::String returnedFile;
    juce::ChangeBroadcaster* loudnessCalculatedBroadcaster;
    double integratedLoudness = 0;

    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    std::unique_ptr<juce::AudioBuffer<float>> readerBuffer;
    std::unique_ptr<juce::AudioSourceChannelInfo> sourceChannelInfo;

    juce::AudioFormatManager formatManager;
    Ebu128LoudnessMeter loudnessMeter;

    int actualSamplesPerBlocksExpected = 480;
    double actualSampleRate = 48000;

    double progress = 0;
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(R128IntegratedThread)
};
