/*
  ==============================================================================

    DataBaseImport.h
    Created: 26 Mar 2021 8:14:16pm
    Author:  DPR

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "nanodbc.h"
#include <iostream>
#include "Windows.h"
#include <string>
#include <stdio.h>
#include <regex>
#include "PlayHead.h"
#include "Settings.h"

//==============================================================================
/*
*/
class DataBaseImport : public juce::Component,
    public juce::FileDragAndDropTarget,
    public juce::TableListBoxModel,
    public juce::ChangeListener,
    public juce::ComponentListener,
    public juce::Timer,
    public juce::Value::Listener
{
public:
    DataBaseImport();
    ~DataBaseImport() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void DataBaseImport::prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    //const juce::String DataBaseImport::startFFmpeg(std::string filePath);

    juce::AudioTransportSource transport;
    juce::ResamplingAudioSource resampledSource;

private:
    void DataBaseImport::feedDB();
    bool DataBaseImport::isInterestedInFileDrag(const juce::StringArray& files);
    void DataBaseImport::filesDropped(const juce::StringArray& files, int x, int y);
    void DataBaseImport::fileDragMove(const juce::StringArray& files, int x, int y);
    void DataBaseImport::fileDragEnter(const juce::StringArray& files, int x, int y);
    void DataBaseImport::fileDragExit(const juce::StringArray& files);
    int DataBaseImport::getNumRows() override;
    void DataBaseImport::paintRowBackground(juce::Graphics& g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected) override;
    void DataBaseImport::paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool /*rowIsSelected*/) override;
    juce::String DataBaseImport::getText(const int columnNumber, const int rowNumber) const;
    void DataBaseImport::setText(const int columnNumber, const int rowNumber, const juce::String& newText);
    juce::Component* DataBaseImport::refreshComponentForCell(int rowNumber, int columnId, bool /*isRowSelected*/, juce::Component* existingComponentToUpdate) override;
    bool DataBaseImport::loadFile(const juce::String& path, bool dispalyThumbnail);
    void DataBaseImport::changeListenerCallback(juce::ChangeBroadcaster* source);
    juce::String DataBaseImport::secondsToMMSS(int seconds);
    void DataBaseImport::clear();
    void DataBaseImport::returnKeyPressed(int lastRowSelected);
    void DataBaseImport::deleteKeyPressed(int lastRowSelected);
    void DataBaseImport::startOrStop();
    void DataBaseImport::cellClicked(int rowNumber, int columnID, const juce::MouseEvent&);
    void DataBaseImport::mouseDown(const juce::MouseEvent& e);
    void DataBaseImport::mouseDrag(const juce::MouseEvent& e);
    void DataBaseImport::timerCallback();
    void DataBaseImport::sortFiles(int columnToSort);
    void DataBaseImport::selectedRowsChanged(int lastRowSelected);
    void DataBaseImport::valueChanged(juce::Value& value);
    void DataBaseImport::openButtonClicked();
    void DataBaseImport::addFiles(const juce::StringArray files);
    double fileSampleRate = 48000;
    juce::TableListBox table;
    juce::StringArray filesToImport;
    juce::StringArray sortedFilesToImport;
    juce::StringArray names;
    juce::StringArray sortedNames;
    juce::StringArray durations;
    juce::StringArray sortedDurations;
    juce::StringArray durationsMMSS;
    juce::StringArray sortedDurationsMMSS;

    juce::TextButton importButton;
    juce::TextButton clearButton;
    juce::TextButton startStopButton{ "Play/Stop" };
    juce::ToggleButton autoPlayButton{ "Autoplay" };
    juce::TextButton openButton{ "Open" };

    //FeedDbThread* importThread;

    int numRows = 0;

    PlayHead playHead;
    juce::Rectangle<int> thumbnailBounds;
    double actualSampleRate;
    int actualSamplesPerBlockExpected;
    juce::AudioFormatManager formatManager;
    juce::AudioThumbnailCache thumbnailCache;
    juce::AudioThumbnail thumbnail;
    std::unique_ptr<juce::AudioFormatReaderSource> playSource;


    class EditableTextCustomComponent : public juce::Label
    {
    public:
        EditableTextCustomComponent(DataBaseImport& td) : owner(td)
        {
            // double click to edit the label text; single click handled below
            setEditable(false, true, false);
        }

        void mouseDown(const juce::MouseEvent& event) override
        {
            // single click on the label should simply select the row
            owner.table.selectRowsBasedOnModifierKeys(row, event.mods, false);

            Label::mouseDown(event);
        }

        void textWasEdited() override
        {
            owner.setText(columnId, row, getText());
        }

        // Our demo code will call this when we may need to update our contents
        void setRowAndColumn(const int newRow, const int newColumn)
        {
            row = newRow;
            columnId = newColumn;
            setText(owner.getText(columnId, row), juce::NotificationType::dontSendNotification);
        }

        void paint(juce::Graphics& g) override
        {
            auto& lf = getLookAndFeel();
            if (!dynamic_cast<juce::LookAndFeel_V4*> (&lf))
                lf.setColour(textColourId, juce::Colours::black);

            Label::paint(g);
        }

    private:
        DataBaseImport& owner;
        int row, columnId;
        juce::Colour textColour;
    };


    class FeedDbThread : public juce::ThreadWithProgressWindow
    {
    public:
        FeedDbThread::FeedDbThread() : juce::ThreadWithProgressWindow("Importing files", true, true)
        {
            setStatusMessage("Importing......");
        }

        FeedDbThread::~FeedDbThread()
        {

        }

        void FeedDbThread::run()
        {
            setProgress(-1.0); // setting a value beyond the range 0 -> 1 will show a spinning bar..
            int thingsToDo = filesToImport.size();
            std::string computerName = juce::SystemStats::getComputerName().toStdString();

            for (auto i = 0; i < filesToImport.size(); i++)
            {
                nanodbc::connection conn;
                auto const connection_string = NANODBC_TEXT("Driver=ODBC Driver 17 for SQL Server;Server=localhost\\NETIA;Database=ABC4;Uid=SYSADM;Pwd=SYSADM;");
                if (!conn.connected())
                    conn.connect(connection_string);

                std::string idRequestString = "SELECT MAX(ID_ITEM) FROM ABC4.SYSADM.T_ITEM";
                nanodbc::result row = execute(
                    conn,
                    idRequestString);
                row.next();
                int nextIdInt;
                std::string nextIdString;
                try
                {
                    nextIdInt = juce::String(row.get<nanodbc::string>(0)).getIntValue() + 1;
                }
                catch (std::runtime_error const& e)
                {

                    std::cerr << e.what() << std::endl;
                }

                if (nextIdInt < 100000)
                {
                    nextIdInt = nextIdInt + 100000;

                }

                nextIdString = juce::String(nextIdInt).toStdString();



                std::string addRowStream = "DECLARE @myid uniqueidentifier "
                    "SET @myid = NEWID() "
                    "INSERT INTO ABC4.SYSADM.T_ITEM(GUID_ITEM, ID_ITEM) "
                    "VALUES(@myid, " + nextIdString + ")";
                execute( conn, addRowStream);

                std::string getGuidStream = "SELECT GUID_ITEM FROM ABC4.SYSADM.T_ITEM WHERE ID_ITEM = " + nextIdString;
                row = execute( conn, getGuidStream);
                row.next();
                std::string convertedFileName = juce::String(row.get<nanodbc::string>(0)).toLowerCase().toStdString();

                juce::File createdFile(startFFmpeg(filesToImport[i].toStdString(), convertedFileName).toStdString());


                std::string fileNameWithoutExtension = createdFile.getFileNameWithoutExtension().toStdString();
                juce::File destinationFile = juce::File(std::string(createdFile.getParentDirectory().getFullPathName().toStdString() + "\\" + fileNameWithoutExtension + "\.BWF"));
                createdFile.moveFileTo(destinationFile);
                std::string injectionString = "UPDATE ABC4.SYSADM.T_ITEM "
                    "SET DATE_BEG_ITEM = GETDATE(), DATE_END_ITEM = GETDATE(), "
                    "STRING_2 = '" + names[i].toStdString() + "', ITEM_DURATION = " + durations[i].toStdString() + ", STATION_REC = '" + computerName + "', "
                    "TYPE_ITEM = 1, TYPE1_ITEM = 10000, MODE = 0, ALGO = 9, FORMAT = 8, FREQUENCY = 48, [PATH] = '\\\\" + computerName + "\\SONS$\\ADMIN\\', [FILE] = '" + destinationFile.getFileName().toStdString() + "', RECORDED = 1, STATE = 2, VIRTUAL = 7, USER_REC = 'DPR', "
                    "LOCK = 0, RTA = 0, RTB = 0, BROADCASTABLE = 0, ARCHIVABLE = 0 "
                    "WHERE ID_ITEM = " + nextIdString;
                DBG(injectionString);
                execute(conn, injectionString);

                if (threadShouldExit())
                    return;


                setProgress(i / (double)thingsToDo);

                setStatusMessage(juce::String(thingsToDo) + " files left to import...");
            }

            setProgress(-1.0); // setting a value beyond the range 0 -> 1 will show a spinning bar..
        }

        // This method gets called on the message thread once our thread has finished..
        void FeedDbThread::threadComplete(bool userPressedCancel)
        {
            if (userPressedCancel)
            {
                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                    "Import Cancelled",
                    "");
            }
            else
            {
                // thread finished normally..
                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
                    "Files Imported",
                    "");
            }

            // ..and clean up by deleting our thread object..
            //delete this;
        }

        void FeedDbThread::setFiles(juce::StringArray f, juce::StringArray n, juce::StringArray d)
        {
            filesToImport = f;
            names = n;
            durations = d;
        }

        const juce::String FeedDbThread::startFFmpeg(std::string filePath, std::string fileName)
        {
            std::string USES_CONVERSION_EX;

            std::string ffmpegpath = juce::String(juce::File::getCurrentWorkingDirectory().getFullPathName() + "\\ffmpeg.exe").toStdString();
            //FFmpegPath = Settings::FFmpegPath.toStdString();
            std::string convertedFilesPath = "D:\\SONS\\ADMIN\\";
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
            std::string cmdstring = std::string("\"" + newFFmpegPath + "\" -i \"" + newFilePath + "\" -acodec mp2 -ar 48000 -ab 256k -ac 2 -af \"volume = -9dB\" -y \"" + newConvertedFilesPath + fileName + ".wav\"");
            //std::string cmdstring = std::string("\"" + newFFmpegPath + "\" -i \"" + newFilePath + "\" -acodec mp2 -ar 48000 -ab 256k -ac 2 -af \"volume = -9dB\" -y \"" + newConvertedFilesPath + rawnamedoubleslash + ".wav\"");
            DBG(cmdstring);
            std::wstring w = (utf8_to_utf16(cmdstring));
            LPWSTR str = const_cast<LPWSTR>(w.c_str());
            ////////////Launch FFMPEG
            PROCESS_INFORMATION pi;
            STARTUPINFOW si;
            ZeroMemory(&si, sizeof(si));
            BOOL logDone = CreateProcessW(NULL, str, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
            if (logDone)
            {
                WaitForSingleObject(pi.hProcess, INFINITE);
            }



            /////////////////////return created file path
            std::string returnFilePath = std::string(newConvertedFilesPath + "\\" + fileName + ".wav");
            //DBG(returnFilePath);
            std::string returnFilePathBackslah = std::regex_replace(returnFilePath, std::regex(R"(\\)"), R"(\\)");
            juce::String returnedFile = juce::String(returnFilePath);

            return returnedFile;
        }
        std::wstring FeedDbThread::utf8_to_utf16(const std::string& utf8)
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
        juce::StringArray filesToImport;
        juce::StringArray names;
        juce::StringArray durations;
    };
    FeedDbThread importThread;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DataBaseImport)
};

