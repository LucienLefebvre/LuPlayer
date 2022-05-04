/*
  ==============================================================================

    R128IntegratedThread.h
    Created: 6 Jan 2022 9:25:30pm
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
//==============================================================================
/*
*/
class R128IntegratedThread : public juce::Thread
{
public:
    R128IntegratedThread(const juce::String& threadName, size_t threadStackSize = 0) : Thread("R128 Integrated")
    {
        loudnessCalculatedBroadcaster = new juce::ChangeBroadcaster();
    }

    ~R128IntegratedThread() override
    {
        delete loudnessCalculatedBroadcaster;
        stopThread(1000);
    }

    void run()
    {
        std::string USES_CONVERSION_EX;
        std::string ffmpegpath = juce::String(juce::File::getCurrentWorkingDirectory().getFullPathName() + "\\ffmpeg.exe").toStdString();
        std::string convertedFilesPath = Settings::convertedSoundsPath.toStdString();
        //////////*****************Create FFMPEG command Line
        //add double slash to path
        std::string newFilePath = std::regex_replace(filePath, std::regex(R"(\\)"), R"(\\)");
        std::string newFFmpegPath = std::regex_replace(ffmpegpath, std::regex(R"(\\)"), R"(\\)");
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
        std::string cmdstring = std::string("\"" + newFFmpegPath + "\" -nostats -i \"" + newFilePath + "\" -filter_complex ebur128=framelog=verbose -f null -");
        std::wstring w = (utf8_to_utf16(cmdstring));
        LPSTR str = const_cast<LPSTR>(cmdstring.c_str());
        ////////////Launch FFMPEG
        process.start(cmdstring);
        juce::String output = process.readAllProcessOutput();
        int nullLuIndex = output.indexOfWholeWord("I:");
        int  iLuIndex = output.indexOfAnyOf("I:", output.length() - 200);
        juce::String luString = output.substring(iLuIndex + 19, iLuIndex + 21);
        integratedLoudness = -(luString.getFloatValue());

        loudnessCalculatedBroadcaster->sendChangeMessage();
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

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(R128IntegratedThread)
};
