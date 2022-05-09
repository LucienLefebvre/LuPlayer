/*
  ==============================================================================

    ffmpegConvert.h
    Created: 22 Dec 2021 3:50:11pm
    Author:  Lucien Lefebvre

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "../Settings/Settings.h"
#include "ffmpegcpp.h"
//==============================================================================

class ffmpegConvert  : public juce::Thread
{
public:
    ffmpegConvert(const juce::String& threadName, size_t threadStackSize = 0) : Thread("ffmpeg Convert")
    {
        conversionEndedBroadcaster = new juce::ChangeBroadcaster();
    }

    ~ffmpegConvert() override
    {
        delete conversionEndedBroadcaster;
        stopThread(1000);
    }

    void run()
    {
        hasBeenKilled = false;

        juce::File inputFile(filePath);
        juce::String outputFileName = inputFile.getFileNameWithoutExtension() + ".wav";
        juce::File outputFile(juce::File(Settings::convertedSoundsPath).getChildFile(outputFileName));

        try
        {
            ffmpegcpp::Muxer* muxer = new ffmpegcpp::Muxer(outputFile.getFullPathName().toStdString().c_str());

            ffmpegcpp::AudioCodec* codec = new ffmpegcpp::AudioCodec(AV_CODEC_ID_PCM_S16LE);
            ffmpegcpp::AudioEncoder* encoder = new ffmpegcpp::AudioEncoder(codec, muxer);

            ffmpegcpp::Demuxer* audioContainer = new ffmpegcpp::Demuxer(filePath.c_str());
            audioContainer->DecodeBestAudioStream(encoder);

            while (!audioContainer->IsDone())
            {
                audioContainer->Step();
            }
            muxer->Close();
        }
        catch (ffmpegcpp::FFmpegException e)
        {

        }

        returnedFile = outputFile.getFullPathName()
            ;
        Settings::tempFiles.add(returnedFile);

        if (!hasBeenKilled)
        {
            conversionEndedBroadcaster->sendChangeMessage();
        }      
        signalThreadShouldExit();
    }

    void setFilePath(juce::String p)
    {
        filePath = p.toStdString();
    }

    juce::String getFile()
    {
        return returnedFile;
    }

    void shouldMakeProgressFile(bool makeFile)
    {
        makeProgressFile = makeFile;
    }

    void killThread()
    {
        hasBeenKilled = true;
        process.kill();
    }

    bool hasBeenKilled = false;
    bool makeProgressFile = true;
    std::string filePath;
    juce::String returnedFile;
    juce::ChangeBroadcaster* conversionEndedBroadcaster;
    juce::ChildProcess process;
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ffmpegConvert)
};
