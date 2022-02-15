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
#include <regex>
#include "convertObject.h"
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

    bool checkAndConvert(int rowNumber);

    void batchConvert();

    void batchConvertButtonClicked();

    void todayButtonClicked();

    void connectButtonClicked();

    void updateButtonStates();

    void unmapDistantDrive();

    //void keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent);

    bool mouseDragged = false;
    bool startDrag = false;
    std::unique_ptr<nanodbc::connection> conn;

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

    juce::TextEditor hostLabel;
    juce::TextButton connectButton;
    juce::ComboBox lastServerBox;

    juce::TextEditor userLabel;
    juce::TextEditor passwordLabel{"Password", (juce::juce_wchar) 0x25cf };

    juce::Label timeLabel;

    juce::Label connectionLabel;

    juce::ToggleButton todayButton;

    juce::TextButton batchConvertButton;
    
    juce::File file;
    double fileSampleRate = 48000;
    Ebu128LoudnessMeter loudnessMeter;
    LoudnessBar loudnessBarComponent;

    bool isConnecting = false;

    //juce::WindowsRegistry registry;
    juce::OwnedArray<convertObject> myConvertObjects;
    int convertObjectIndex = 0;
    juce::ProgressBar convertProgress;
    double progression = -1.;
    bool isConverting = false;
    bool isBatchConverting = false;
    int batchConvertIndex = 0;

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

    juce::ChildProcess process;

    class connectThread : public juce::Thread
    {
    public:
        connectThread::connectThread() : juce::Thread("Distant Database")
        {
            //setStatusMessage("Connecting to database......");
            connectionTriedBroadcaster = new juce::ChangeBroadcaster();
        }

        connectThread::~connectThread()
        {
            isConnected = false;
            delete connectionTriedBroadcaster;
        }

        void connectThread::run()
        {
            auto const connection_string = NANODBC_TEXT(connectionString.toStdString());
            try
            {
                DBG(hostAdress);
                DBG(user);
                connection->connect(connection_string, 5L);

                //nanodbc::connection conn(connection_string);
                //conn.dbms_name();
            }
            catch (std::runtime_error const& e)
            {
                std::cerr << e.what() << std::endl;
            }

            if (connection->connected())
            {
                DBG("mapping...");
                std::string USES_CONVERSION_EX;
                std::string cmdstring = std::string("net use z: \\\\" 
                                        + hostAdress.toStdString() 
                                        + "\\sons$ " 
                                        + password.toStdString() 
                                        + " /user:" 
                                        + user.toStdString());

                process.start(cmdstring);
                if (process.waitForProcessToFinish(5000))
                    DBG("isconnected");
                isConnected = true;

                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
                    "Distant Database Found",
                    "");
                signalThreadShouldExit();
            }
            else
            {
                isConnected = false;

                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                    "No Database Found",
                    "");
                signalThreadShouldExit();

            }
            //if (threadShouldExit())
            //{
                return;
            //}
        }

        void connectThread::setString(juce::String s)
        {
            connectionString = s;
        }

        void connectThread::setHost(juce::String h)
        {
            hostAdress = h;
        }

        void connectThread::setUser(juce::String u)
        {
            user = u;
        }

        void connectThread::setPassword(juce::String p)
        {
            password = p;
        }

        void connectThread::setConnection(nanodbc::connection& c)
        {
            connection = &c;
        }
        /*bool connectThread::getIsConnected()
        {
            return connection->connected();
        }*/

        juce::ChangeBroadcaster* connectionTriedBroadcaster;

        juce::ChildProcess process;

        juce::String connectionString;
        juce::String hostAdress;
        juce::String user;
        juce::String password;
        nanodbc::connection* connection;
        bool isConnected = false;

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

    std::unique_ptr<connectThread> thread;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DistantDataBaseBrowser)
};
