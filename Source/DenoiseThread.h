/*
  ==============================================================================

    DenoiseThread.h
    Created: 15 Jan 2022 4:39:24pm
    Author:  DPR

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

class DenoiseThread : public juce::Thread
{
public:
    struct DenoiseParameters
    {
        std::string filePath;
        bool isPreview = false;
        double noiseReductionDB = 6.0;
        double noiseFloorDB = -30.0;
        std::string noiseType = "w";
    };

    DenoiseThread(const juce::String& threadName, size_t threadStackSize = 0) : Thread("ffmpeg Convert")
    {
        denoiseEndedBroadcaster = new juce::ChangeBroadcaster();
        previewEndedBroadcaster = new juce::ChangeBroadcaster();
    }

    ~DenoiseThread() override
    {
        delete denoiseEndedBroadcaster;
        delete previewEndedBroadcaster;
        stopThread(1000);
    }

    void run()
    {
        std::string USES_CONVERSION_EX;

        std::string ffmpegpath = juce::String(juce::File::getCurrentWorkingDirectory().getFullPathName() + "\\ffmpeg.exe").toStdString();
        //FFmpegPath = Settings::FFmpegPath.toStdString();
        std::string convertedFilesPath = Settings::convertedSoundsPath.toStdString();
        //////////*****************Create FFMPEG command Line
        //add double slash to path
        std::string newFilePath = std::regex_replace(params.filePath, std::regex(R"(\\)"), R"(\\)");
        std::string newFFmpegPath = std::regex_replace(ffmpegpath, std::regex(R"(\\)"), R"(\\)");
        std::string newConvertedFilesPath = std::regex_replace(convertedFilesPath, std::regex(R"(\\)"), R"(\\)");
        //give Output Directory
        std::size_t botDirPos = params.filePath.find_last_of("\\");
        std::string outputFileDirectory = params.filePath.substr(0, botDirPos);
        //give file name with extension
        std::string fileOutputName = params.filePath.substr(botDirPos, params.filePath.length());
        std::string newFileOutputDir = std::regex_replace(outputFileDirectory, std::regex(R"(\\)"), R"(\\)");
        size_t lastindex = fileOutputName.find_last_of(".");
        //give file name without extension and add double dash before
        std::string rawname = fileOutputName.substr(0, lastindex);
        std::string rawnamedoubleslash = std::regex_replace(rawname, std::regex(R"(\\)"), R"(\\)");
        //create entire command string
        std::string cmdstring;
        if (params.isPreview)
            cmdstring = std::string("\"" + newFFmpegPath + "\" -i \"" + newFilePath + "\" -ar 48000 -y -af \"afftdn=nr="
                + juce::String(params.noiseReductionDB).toStdString()
                + ":nf=" + juce::String(params.noiseFloorDB).toStdString()
                + ":nt=" + params.noiseType
                + ":om=o,atrim=end=10\" \"" + newConvertedFilesPath + rawnamedoubleslash + "DNS.wav\"");
        else
            cmdstring = std::string("\"" + newFFmpegPath + "\" -i \"" + newFilePath + "\" -ar 48000 -y -af \"afftdn=nr=" 
                + juce::String(params.noiseReductionDB).toStdString() 
                + ":nf=" + juce::String(params.noiseFloorDB).toStdString()
                + ":nt=" + params.noiseType 
                + ":om=o\" \"" + newConvertedFilesPath + rawnamedoubleslash + "DNS.wav\"");
        DBG(cmdstring);
        std::wstring w = (utf8_to_utf16(cmdstring));
        //LPWSTR str = const_cast<LPWSTR>(w.c_str());
        //////////////Launch FFMPEG
        //PROCESS_INFORMATION pi;
        //STARTUPINFOW si;
        //ZeroMemory(&si, sizeof(si));
        //BOOL logDone = CreateProcessW(NULL, str, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
        //if (logDone)
        //{
        //    WaitForSingleObject(pi.hProcess, INFINITE);
        //}

        juce::ChildProcess process;
        process.start(cmdstring);
        process.waitForProcessToFinish(30000);
        /////////////////////return created file path
        std::string returnFilePath = std::string(newConvertedFilesPath + "\\" + rawname + "DNS.wav");
        DBG(returnFilePath);
        std::string returnFilePathBackslah = std::regex_replace(returnFilePath, std::regex(R"(\\)"), R"(\\)");
        returnedFile = juce::String(returnFilePath);
        Settings::tempFiles.add(returnedFile);

        if (!params.isPreview)
            denoiseEndedBroadcaster->sendChangeMessage();
        else
            previewEndedBroadcaster->sendChangeMessage();
        signalThreadShouldExit();
    }

    void setFilePath(juce::String p)
    {
        params.filePath = p.toStdString();
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

    void setDenoiseParams(DenoiseParameters& p)
    {
        params = p;
    }

    bool makeProgressFile = true;
    DenoiseParameters params;
    juce::String returnedFile;
    juce::ChangeBroadcaster* denoiseEndedBroadcaster;
    juce::ChangeBroadcaster* previewEndedBroadcaster;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DenoiseThread)
};
