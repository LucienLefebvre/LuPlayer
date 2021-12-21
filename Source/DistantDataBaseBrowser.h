/*
  ==============================================================================

    DistantDistantDataBaseBrowser.h
    Created: 20 Dec 2021 3:26:29pm
    Author:  DPR

  ==============================================================================
*/
#pragma once

#include <JuceHeader.h>
#include "nanodbc/nanodbc.h"
#include <iostream>
#include "PlayHead.h"
#include <Ebu128LoudnessMeter.h>
#include "LoudnessBar.h"
#include <stdio.h>
#include <string>
#include "Windows.h"
//==============================================================================
/*
*/
class DistantDataBaseBrowser : public juce::Component,
    public juce::Label::Listener,
    public juce::TableListBoxModel,
    public juce::ComponentListener,
    public juce::ChangeListener,
    public juce::Timer,
    public juce::TableHeaderComponent::Listener,
    public juce::ChangeBroadcaster,
    public juce::Value::Listener,
    public juce::Thread::Listener,
    public juce::ComboBox::Listener
{
public:
    DistantDataBaseBrowser();
    ~DistantDataBaseBrowser() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    int DistantDataBaseBrowser::getNumRows() override;
    void paintRowBackground(juce::Graphics& g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected) override;
    void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool /*rowIsSelected*/) override;
    void DistantDataBaseBrowser::prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    juce::File DistantDataBaseBrowser::getSelectedFile();
    juce::String DistantDataBaseBrowser::getSelectedSoundName();
    juce::AudioTransportSource transport;
    juce::ResamplingAudioSource resampledSource;
    juce::ChangeBroadcaster* fileDraggedFromDataBase;
    juce::ChangeBroadcaster* fileDroppedFromDataBase;
    juce::ChangeBroadcaster* cuePlay;

private:
    void DistantDataBaseBrowser::initialize();
    void DistantDataBaseBrowser::labelTextChanged(juce::Label* labelThatHasChanged);
    void DistantDataBaseBrowser::clearListBox();
    void DistantDataBaseBrowser::cellClicked(int rowNumber, int columnID, const juce::MouseEvent& e);
    void DistantDataBaseBrowser::sqlQuery(std::string search, int sortColum = 0, int sortDirection = 0);
    void DistantDataBaseBrowser::changeListenerCallback(juce::ChangeBroadcaster* source);
    void DistantDataBaseBrowser::startOrStop();
    void DistantDataBaseBrowser::timerCallback();
    void DistantDataBaseBrowser::mouseDown(const juce::MouseEvent& e) override;
    void DistantDataBaseBrowser::mouseDrag(const juce::MouseEvent& e) override;
    void DistantDataBaseBrowser::mouseUp(const juce::MouseEvent& e) override;
    void DistantDataBaseBrowser::tableSortOrderChanged(juce::TableHeaderComponent*) override;
    void DistantDataBaseBrowser::tableColumnsChanged(juce::TableHeaderComponent* tableHeader) override;
    void DistantDataBaseBrowser::tableColumnsResized(juce::TableHeaderComponent* tableHeader) override;
    void DistantDataBaseBrowser::tableColumnDraggingChanged(juce::TableHeaderComponent* tableHeader, int columnIdNowBeingDragged) override;
    void DistantDataBaseBrowser::valueChanged(juce::Value& value);
    void DistantDataBaseBrowser::connectToDB();
    void DistantDataBaseBrowser::exitSignalSent();

    void DistantDataBaseBrowser::updateLastServersBox();

    bool checkIfHostExistInList(juce::String host);

    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged);

    void resetThumbnail();

    bool mouseDragged = false;
    bool startDrag = false;
    nanodbc::connection conn;

    int numRows = 0;

    juce::TableListBox table;

    juce::StringArray names;
    juce::StringArray namesbis;
    juce::StringArray durations;
    juce::StringArray dates;
    juce::StringArray files;

    juce::Label searchLabel;
    juce::TextButton refreshButton;
    juce::TextButton startStopButton{ "Play/Stop" };
    juce::TextButton clearSearchButton;
    juce::ToggleButton autoPlayButton{ "Autoplay" };

    juce::Label hostLabel;
    juce::TextButton connectButton;
    juce::ComboBox lastServerBox;

    juce::Label timeLabel;

    juce::Label connectionLabel;

    juce::File file;
    double fileSampleRate = 48000;
    Ebu128LoudnessMeter loudnessMeter;
    LoudnessBar loudnessBarComponent;

    //juce::WindowsRegistry registry;


    PlayHead playHead;
    juce::Rectangle<int> thumbnailBounds;
    double actualSampleRate;
    int actualSamplesPerBlockExpected;
    juce::AudioFormatManager formatManager;
    juce::AudioThumbnailCache thumbnailCache;
    juce::AudioThumbnail thumbnail;
    std::unique_ptr<juce::AudioFormatReaderSource> playSource;
    bool DistantDataBaseBrowser::loadFile(const juce::String& path);
    std::wstring DistantDataBaseBrowser::utf8_to_utf16(const std::string& utf8);
    juce::String DistantDataBaseBrowser::secondsToMMSS(int seconds);
    const juce::String DistantDataBaseBrowser::startFFmpeg(std::string filePath);



    class connectThread : public juce::ThreadWithProgressWindow
    {
    public:
        connectThread::connectThread() : juce::ThreadWithProgressWindow("Distant Database", true, true)
        {
            setStatusMessage("Connecting to database......");
        }

        connectThread::~connectThread()
        {

        }

        void connectThread::run()
        {
            auto const connection_string = NANODBC_TEXT(connectionString.toStdString());
            try
            {
                connection.connect(connection_string, 2);

                //nanodbc::connection conn(connection_string);
                //conn.dbms_name();
            }
            catch (std::runtime_error const& e)
            {
                std::cerr << e.what() << std::endl;
            }

            if (connection.connected())
            {


                setStatusMessage("Mapping distant drive......");
                setProgress(0.5);
                //DISK MAPPING
                std::string USES_CONVERSION_EX;
                std::string cmdstring = std::string("net use z: \\\\" + hostAdress.toStdString() + "\\sons$");
                std::wstring w = (utf8_to_utf16(cmdstring));
                LPWSTR str = const_cast<LPWSTR>(w.c_str());
                DBG(str);
                PROCESS_INFORMATION pi;
                STARTUPINFOW si;
                si.wShowWindow = SW_SHOW;
                si.dwFlags = STARTF_USESHOWWINDOW;
                ZeroMemory(&si, sizeof(si));
                BOOL logDone = CreateProcessW(NULL, str, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
                if (logDone)
                {
                    WaitForSingleObject(pi.hProcess, INFINITE);
                }
                isConnected = true;
                signalThreadShouldExit();
                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
                    "Distant Database Found",
                    "");
            }
            else
            {
                isConnected = false;
                signalThreadShouldExit();
                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                    "No Database Found",
                    "");
            }
            if (threadShouldExit())
                return;
        }

        void connectThread::setString(juce::String s)
        {
            connectionString = s;
        }

        void connectThread::setHost(juce::String h)
        {
            hostAdress = h;
        }

        void connectThread::setConnection(nanodbc::connection& c)
        {
            connection = c;
        }
        juce::String connectionString;
        juce::String hostAdress;
        nanodbc::connection connection;
        bool isConnected;



        std::wstring connectThread::utf8_to_utf16(const std::string& utf8)
        {
            std::vector<unsigned long> unicode;
            size_t i = 0;
            while (i < utf8.size())
            {
                unsigned long uni;
                size_t todo;
                bool error = false;
                unsigned char ch = utf8[i++];
                if (ch <= 0x7F)
                {
                    uni = ch;
                    todo = 0;
                }
                else if (ch <= 0xBF)
                {
                    throw std::logic_error("not a UTF-8 string");
                }
                else if (ch <= 0xDF)
                {
                    uni = ch & 0x1F;
                    todo = 1;
                }
                else if (ch <= 0xEF)
                {
                    uni = ch & 0x0F;
                    todo = 2;
                }
                else if (ch <= 0xF7)
                {
                    uni = ch & 0x07;
                    todo = 3;
                }
                else
                {
                    throw std::logic_error("not a UTF-8 string");
                }
                for (size_t j = 0; j < todo; ++j)
                {
                    if (i == utf8.size())
                        throw std::logic_error("not a UTF-8 string");
                    unsigned char ch = utf8[i++];
                    if (ch < 0x80 || ch > 0xBF)
                        throw std::logic_error("not a UTF-8 string");
                    uni <<= 6;
                    uni += ch & 0x3F;
                }
                if (uni >= 0xD800 && uni <= 0xDFFF)
                    throw std::logic_error("not a UTF-8 string");
                if (uni > 0x10FFFF)
                    throw std::logic_error("not a UTF-8 string");
                unicode.push_back(uni);
            }
            std::wstring utf16;
            for (size_t i = 0; i < unicode.size(); ++i)
            {
                unsigned long uni = unicode[i];
                if (uni <= 0xFFFF)
                {
                    utf16 += (wchar_t)uni;
                }
                else
                {
                    uni -= 0x10000;
                    utf16 += (wchar_t)((uni >> 10) + 0xD800);
                    utf16 += (wchar_t)((uni & 0x3FF) + 0xDC00);
                }
            }
            return utf16;
        }
    };

    connectThread thread;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DistantDataBaseBrowser)
};
