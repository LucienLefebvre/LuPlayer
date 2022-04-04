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
#include "Settings.h"
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
        std::string cmdstring = std::string("\"" + newFFmpegPath + "\" -nostats -i \"" + newFilePath + "\" -filter_complex ebur128=framelog=verbose -f null -");
        std::wstring w = (utf8_to_utf16(cmdstring));
        LPSTR str = const_cast<LPSTR>(cmdstring.c_str());
        //DBG(cmdstring);
        //LPSTR s = const_cast<char*>(w.c_str());
        ////////////Launch FFMPEG
        process.start(cmdstring);
        juce::String output = process.readAllProcessOutput();
        int nullLuIndex = output.indexOfWholeWord("I:");
        int  iLuIndex = output.indexOfAnyOf("I:", output.length() - 200);
        juce::String luString = output.substring(iLuIndex + 19, iLuIndex + 21);
        integratedLoudness = -(luString.getFloatValue());

        //SECURITY_ATTRIBUTES sa;
        //sa.nLength = sizeof(sa);
        //sa.lpSecurityDescriptor = NULL;
        //sa.bInheritHandle = TRUE;

        //std::string outFileString = fileName + ".txt";
        //LPCSTR outFile = outFileString.c_str();
        //HANDLE h = CreateFile((outFile),
        //    FILE_APPEND_DATA,
        //    FILE_SHARE_WRITE | FILE_SHARE_READ,
        //    &sa,
        //    OPEN_ALWAYS,
        //    FILE_ATTRIBUTE_NORMAL,
        //    NULL);

        //PROCESS_INFORMATION pi;
        //STARTUPINFO si;
        //BOOL ret = FALSE;
        //DWORD flags = CREATE_NO_WINDOW;

        //ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
        //ZeroMemory(&si, sizeof(STARTUPINFO));
        //si.cb = sizeof(STARTUPINFO);
        //si.dwFlags |= STARTF_USESTDHANDLES;
        //si.hStdInput = NULL;
        //si.hStdError = h;
        //si.hStdOutput = h;

        //TCHAR cmd[] = TEXT("Test.exe 30");
        //ret = CreateProcess(NULL, str, NULL, NULL, TRUE, flags, NULL, NULL, &si, &pi);
        //WaitForSingleObject(pi.hProcess, INFINITE);
        //if (ret)
        //{
        //    CloseHandle(pi.hProcess);
        //    CloseHandle(pi.hThread);
        //    CloseHandle(h);
        //}

        //progressFilePath = (juce::File::getCurrentWorkingDirectory().getFullPathName() + "\\" + fileName + ".txt").toStdString();
        //juce::File progressFile(progressFilePath);
        //if (progressFile.exists())
        //{

        //    juce::StringArray progressInfo;
        //    progressFile.readLines(progressInfo);
        //    int currentProgressLine = progressInfo.size() - 9; //récupère la ligne out_time_ms
        //    juce::String progressLine = progressInfo[currentProgressLine].trimCharactersAtStart("    I:         ").trimCharactersAtEnd(" LUFS");//enlève le début
        //    integratedLoudness = progressLine.getDoubleValue();
        //    double loudnessDifference = -23. - integratedLoudness;

        //}
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
