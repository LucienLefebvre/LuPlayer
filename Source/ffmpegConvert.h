/*
  ==============================================================================

    ffmpegConvert.h
    Created: 22 Dec 2021 3:50:11pm
    Author:  lucie

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <stdio.h>
#include <string>
#include "Windows.h"
#include "helpers.h"
#include <regex>
#include "Settings.h"
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

        std::string ffmpegpath = juce::String(juce::File::getCurrentWorkingDirectory().getFullPathName() + "\\ffmpeg.exe").toStdString();
        //FFmpegPath = Settings::FFmpegPath.toStdString();
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
        std::string cmdstring;
        if (makeProgressFile)
            cmdstring = std::string("\"" + newFFmpegPath + "\" -i \"" + newFilePath + "\" -ar 48000 -y \"" + newConvertedFilesPath + rawnamedoubleslash + ".wav\" -progress " + newConvertedFilesPath + rawnamedoubleslash + ".txt\"");
        else
            cmdstring = std::string("\"" + newFFmpegPath + "\" -i \"" + newFilePath + "\" -ar 48000 -y \"" + newConvertedFilesPath + rawnamedoubleslash + ".wav\"");
        std::wstring w = (utf8_to_utf16(cmdstring));
        LPWSTR str = const_cast<LPWSTR>(w.c_str());
        ////////////Launch FFMPEG
        process.start(cmdstring);
        bool result = process.waitForProcessToFinish(30000);
        /*PROCESS_INFORMATION pi;
        STARTUPINFOW si;
        ZeroMemory(&si, sizeof(si));
        BOOL logDone = CreateProcessW(NULL, str, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
        if (logDone)
        {
            WaitForSingleObject(pi.hProcess, INFINITE);
        }*/



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

        int i = 3;
        signalThreadShouldExit();
    }

    void setFilePath(juce::String p)
    {
        filePath = p.toStdString();
        //filePath = std::regex_replace(p.toStdString(), std::regex(R"(\\)"), R"(\\)");;
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
        //stopThread(0);
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
