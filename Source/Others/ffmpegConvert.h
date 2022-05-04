/*
  ==============================================================================

    ffmpegConvert.h
    Created: 22 Dec 2021 3:50:11pm
    Author:  Lucien Lefebvre

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <stdio.h>
#include <string>
#include "Windows.h"
#include "helpers.h"
#include <regex>
#include "../Settings/Settings.h"
#include "ffmpegcpp.h"
#include <iostream>
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
        std::string USES_CONVERSION_EX;

        std::string convertedFilesPath = Settings::convertedSoundsPath.toStdString();
        //add double slash to path
        std::string newFilePath = std::regex_replace(filePath, std::regex(R"(\\)"), R"(\\)");
        std::string newConvertedFilesPath = std::regex_replace(convertedFilesPath, std::regex(R"(\\)"), R"(\\)");
        //give Output Directory
        std::size_t botDirPos = filePath.find_last_of("\\");
        std::string outputFileDirectory = filePath.substr(0, botDirPos);
        //give file name with extension
        std::string fileOutputName = filePath.substr(botDirPos, filePath.length());
        std::string newFileOutputDir = std::regex_replace(outputFileDirectory, std::regex(R"(\\)"), R"(\\)");
        size_t lastindex = fileOutputName.find_last_of(".");
        //give file name without extension and add double dash before
        std::string rawname = fileOutputName.substr(0, lastindex);
        std::string rawnamedoubleslash = std::regex_replace(rawname, std::regex(R"(\\)"), R"(\\)");
        //create entire command string
        std::string cmdstring;

        try
        {
            std::string outputFilePath = newConvertedFilesPath + rawnamedoubleslash + ".wav";
            ffmpegcpp::Muxer* muxer = new ffmpegcpp::Muxer(outputFilePath.c_str());

            ffmpegcpp::AudioCodec* codec = new ffmpegcpp::AudioCodec(AV_CODEC_ID_PCM_S16LE);
            ffmpegcpp::AudioEncoder* encoder = new ffmpegcpp::AudioEncoder(codec, muxer);

            ffmpegcpp::Demuxer* audioContainer = new ffmpegcpp::Demuxer(newFilePath.c_str());
            audioContainer->DecodeBestAudioStream(encoder);

            while (!audioContainer->IsDone())
            {
                audioContainer->Step();
                
                DBG("frame count : " << audioContainer->GetFrameCount(0));
            }
            muxer->Close();
        }
        catch (ffmpegcpp::FFmpegException e)
        {

        }

        /////////////////////return created file path
        std::string returnFilePath = std::string(newConvertedFilesPath + "\\" + rawname + ".wav");
        //DBG(returnFilePath);
        std::string returnFilePathBackslah = std::regex_replace(returnFilePath, std::regex(R"(\\)"), R"(\\)");
        returnedFile = juce::String(returnFilePath);
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
