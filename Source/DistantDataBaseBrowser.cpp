/*
  ==============================================================================

    DistantDistantDataBaseBrowser.cpp
    Created: 20 Dec 2021 3:26:29pm
    Author:  DPR

  ==============================================================================
*/

#include <JuceHeader.h>
#include "DistantDataBaseBrowser.h"
#include <stdio.h>
#include <string>
#include "nanodbc/nanodbc.h"
#include "Windows.h"
#include "Settings.h"

using namespace std;
using namespace nanodbc;

//==============================================================================
DistantDataBaseBrowser::DistantDataBaseBrowser() : thumbnailCache(5), thumbnail(521, formatManager, thumbnailCache), resampledSource(&transport, false, 2)
{
    //auto const connection_string = NANODBC_TEXT("Driver=ODBC Driver 17 for SQL Server;Server=192.168.8.107;Database=ABC4;Uid=SYSADM;Pwd=SYSADM;");
    Settings::sampleRateValue.addListener(this);
    juce::Timer::startTimer(40);

    addAndMakeVisible(&searchLabel);
    searchLabel.setBounds(100, 1, 300, 25);
    searchLabel.setEditable(true, true, false);
    searchLabel.setText("", juce::NotificationType::dontSendNotification);
    searchLabel.setColour(juce::Label::outlineColourId, juce::Colours::lightgrey);
    searchLabel.addListener(this);

    addAndMakeVisible(&refreshButton);
    refreshButton.setBounds(5, 1, 94, 25);
    refreshButton.setButtonText("Resfresh");
    refreshButton.onClick = [this] {sqlQuery(searchLabel.getText().toStdString(), table.getHeader().getSortColumnId(), !table.getHeader().isSortedForwards()); };

    table.setModel(this);
    addAndMakeVisible(&table);
    table.setBounds(0, 30, 100, getHeight() - 30);
    table.getHeader().addColumn("Name", 1, 400);
    table.getHeader().addColumn("Duration", 2, 100);
    table.getHeader().addColumn("Date", 3, 200);


    table.addMouseListener(this, true);

    table.addComponentListener(this);
    table.getHeader().addListener(this);

    addAndMakeVisible(&timeLabel);
    timeLabel.setFont(juce::Font(20.00f, juce::Font::plain).withTypefaceStyle("Regular"));
    timeLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(startStopButton);
    startStopButton.setColour(juce::TextButton::buttonColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    startStopButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    startStopButton.onClick = [this] { startOrStop(); };
    startStopButton.setEnabled(false);

    addAndMakeVisible(&autoPlayButton);
    autoPlayButton.setToggleState(false, juce::NotificationType::dontSendNotification);

    addAndMakeVisible(&clearSearchButton);
    clearSearchButton.setButtonText("X");
    clearSearchButton.setBounds(401, 1, 24, 25);
    clearSearchButton.onClick = [this] {searchLabel.setText("", juce::NotificationType::sendNotification); };

    addAndMakeVisible(&hostLabel);
    hostLabel.setBounds(0, getHeight() - 27, 150, 25);
    hostLabel.setText("IP or host name", juce::NotificationType::dontSendNotification);
    hostLabel.setColour(juce::Label::outlineColourId, juce::Colours::lightgrey);
    hostLabel.setEditable(true, true, false);
    hostLabel.addListener(this);

    addAndMakeVisible(&lastServerBox);
    updateLastServersBox();
    lastServerBox.addListener(this);

    addAndMakeVisible(&connectButton);
    connectButton.setButtonText("Connect");
    connectButton.onClick = [this] { connectToDB(); };

    thumbnail.addChangeListener(this);
    addAndMakeVisible(&playHead);

    table.getHeader().setColour(juce::TableHeaderComponent::ColourIds::backgroundColourId, juce::Colour(40, 134, 189));

    fileDraggedFromDataBase = new juce::ChangeBroadcaster();
    fileDroppedFromDataBase = new juce::ChangeBroadcaster();
    cuePlay = new juce::ChangeBroadcaster();

    formatManager.registerBasicFormats();

    addChildComponent(&connectionLabel);
    connectionLabel.setSize(400, 50);
    connectionLabel.setFont(juce::Font(50.));
    connectionLabel.setText("Trying to connect...", juce::NotificationType::dontSendNotification);

    thread.addListener(this);

    //try
    //{

    //    conn.connect(connection_string, 1000);
    //    //nanodbc::connection conn(connection_string);
    //    conn.dbms_name();
    //    initialize();
    //}
    //catch (std::runtime_error const& e)
    //{
    //    std::cerr << e.what() << std::endl;
    //}



}

DistantDataBaseBrowser::~DistantDataBaseBrowser()
{
    Settings::sampleRateValue.removeListener(this);
    transport.setSource(nullptr);
    juce::Timer::stopTimer();
    thumbnail.setSource(nullptr);
    delete fileDraggedFromDataBase;
    delete fileDroppedFromDataBase;
    delete cuePlay;
    table.removeComponentListener(this);
    transport.removeAllChangeListeners();
    //transport.releaseResources();
    //resampledSource.releaseResources();
    conn.disconnect();
    thread.removeListener(this);
    //UNMAP DRIVE
    std::string cmdstring = std::string("net use z: /delete");
    //std::string cmdstring = std::string("net use z: \\\\Laptop-67vrd21a\\sons$");
    std::wstring w = (utf8_to_utf16(cmdstring));
    LPWSTR str = const_cast<LPWSTR>(w.c_str());
    DBG(str);
    ////////////Launch FFMPEG
    PROCESS_INFORMATION pi;
    STARTUPINFOW si;
    ZeroMemory(&si, sizeof(si));
    BOOL logDone = CreateProcessW(NULL, str, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    if (logDone)
    {
        WaitForSingleObject(pi.hProcess, INFINITE);
    }

}

void DistantDataBaseBrowser::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));   // clear the background
    g.setColour(juce::Colour(40, 134, 189));
    thumbnail.drawChannels(g,
        thumbnailBounds,
        0,                                    // start time
        (float)thumbnail.getTotalLength(),             // end time
        2.0);                                  // vertical zoom
    g.fillRect(getWidth() / 2, 0, 4, getHeight());
    //g.fillRect(0, 28, getWidth(), 2);
}

void DistantDataBaseBrowser::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    actualSampleRate = sampleRate;
    actualSamplesPerBlockExpected = samplesPerBlockExpected;

    transport.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void DistantDataBaseBrowser::resized()
{

    table.setBounds(5, 30, getWidth() / 2 - 5, getHeight() - 30 - 30);
    table.getHeader().setColumnWidth(1, getWidth() * 5 / 16 - 5);
    table.getHeader().setColumnWidth(2, getWidth() / 16 - 4);
    table.getHeader().setColumnWidth(3, getWidth() * 2 / 16 - 4);
    thumbnailBounds.setBounds(getWidth() / 2 + 4, 30, getWidth() / 2, getHeight() - 30);
    startStopButton.setBounds(getWidth() / 2 + 4, 0, 100, 25);
    autoPlayButton.setBounds(getWidth() / 2 + 4 + 101, 0, 100, 25);
    timeLabel.setBounds(autoPlayButton.getRight(), 0, getWidth() - autoPlayButton.getRight(), 25);
    playHead.setSize(1, getHeight() - 30);
    hostLabel.setBounds(150, getHeight() - 27, 150, 25);
    lastServerBox.setBounds(0, getHeight() - 27, 150, 25);
    connectButton.setBounds(300, getHeight() - 27, 100, 25);
    connectionLabel.setCentrePosition(getWidth() / 2, getHeight() / 2);
}

int DistantDataBaseBrowser::getNumRows()
{
    return numRows;
}

void DistantDataBaseBrowser::paintRowBackground(juce::Graphics& g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected)
{
    auto alternateColour = getLookAndFeel().findColour(juce::ListBox::backgroundColourId)
        .interpolatedWith(getLookAndFeel().findColour(juce::ListBox::textColourId), 0.03f);
    if (rowIsSelected)
        g.fillAll(juce::Colour(40, 134, 189));
    else if (rowNumber % 2)
        g.fillAll(alternateColour);
}

void DistantDataBaseBrowser::paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool /*rowIsSelected*/)
{
    g.setColour(getLookAndFeel().findColour(juce::ListBox::textColourId));
    if (rowNumber == table.getSelectedRow())
        g.setColour(juce::Colours::black);
    if (files[rowNumber].isEmpty())
        g.setColour(juce::Colours::red);
    if (columnId == 1)
    {
        g.drawText(names[rowNumber], 2, 0, width - 4, height, juce::Justification::centredLeft, true);
    }
    else if (columnId == 2)
    {
        g.drawText(secondsToMMSS(durations[rowNumber].getIntValue() / 1000), 2, 0, width - 4, height, juce::Justification::centredLeft, true);
    }
    else if (columnId == 3)
    {
        g.drawText(dates[rowNumber], 2, 0, width - 4, height, juce::Justification::centredLeft, true);
    }
    /*else if (columnId == 4)
    {
        g.drawText(files[rowNumber], 2, 0, width - 4, height, juce::Justification::centredLeft, true);
    }*/

    if (rowNumber == table.getSelectedRow())
    {
        g.setColour(juce::Colour(40, 134, 189));
        g.fillRect(width - 1, 0, 1, height);
    }
    else
    {
        g.setColour(getLookAndFeel().findColour(juce::ListBox::backgroundColourId));
        g.fillRect(width - 1, 0, 1, height);
    }
}

void DistantDataBaseBrowser::initialize()
{
    //sqlQuery("", 3, 1);
    table.getHeader().setSortColumnId(3, false);
}

void DistantDataBaseBrowser::sqlQuery(std::string search, int sortColum, int sortDirection)
{
    clearListBox();
    //auto const connection_string = NANODBC_TEXT("Driver=ODBC Driver 17 for SQL Server;Server=192.168.8.107;Database=ABC4;Uid=SYSADM;Pwd=SYSADM;");
    //if (!conn.connected())
        //conn.connect(connection_string, 1000);

    std::string searchString;
    if (sortColum == 0)
        searchString = std::string("SELECT * FROM ABC4.SYSADM.T_ITEM WHERE ABC4.SYSADM.T_ITEM.STRING_2 LIKE '%" + search + "%' AND STATION_DELETE IS NULL");
    else
    {
        std::string columnToSort;
        if (sortColum == 1)
        {
            columnToSort = "STRING_2";
        }
        else if (sortColum == 2)
        {
            columnToSort = "ITEM_DURATION";
        }
        else if (sortColum == 3)
        {
            columnToSort = "DATE_BEG_ITEM";
        }
        std::string direction;
        if (sortDirection == 0)
            searchString = std::string("SELECT * FROM ABC4.SYSADM.T_ITEM WHERE ABC4.SYSADM.T_ITEM.STRING_2 LIKE '%" + search + "%' AND STATION_DELETE IS NULL ORDER BY " + columnToSort + " ASC");
        else if (sortDirection == 1)
            searchString = std::string("SELECT * FROM ABC4.SYSADM.T_ITEM WHERE ABC4.SYSADM.T_ITEM.STRING_2 LIKE '%" + search + "%' AND STATION_DELETE IS NULL ORDER BY " + columnToSort + " DESC");


    }


    nanodbc::result row = execute(
        conn,
        searchString);


    for (int i = 1; row.next(); ++i)
    {
        //numRows = i;
        try
        {
            names.set(i, row.get<nanodbc::string>(45));
        }
        catch (std::runtime_error const& e)
        {
            names.set(i, "");
            std::cerr << e.what() << std::endl;
        }
        try
        {
            durations.set(i, row.get<nanodbc::string>(11));
        }
        catch (std::runtime_error const& e)
        {
            durations.set(i, "");
            std::cerr << e.what() << std::endl;
        }
        try
        {
            dates.set(i, row.get<nanodbc::string>(2));
        }
        catch (std::runtime_error const& e)
        {
            dates.set(i, "");
            std::cerr << e.what() << std::endl;
        }
        try
        {
            files.set(i, row.get<nanodbc::string>(29));
        }
        catch (std::runtime_error const& e)
        {
            files.set(i, "");
            std::cerr << e.what() << std::endl;
        }

    }
    for (int i = 0; i < files.size(); ++i)
    {
        if (files[i].isEmpty())
        {
            names.remove(i);
            dates.remove(i);
            durations.remove(i);
            files.remove(i);
            i--;
        }
        else
            dates.set(i, dates[i].dropLastCharacters(6));
        numRows = names.size();
    }
    table.updateContent();

}

void DistantDataBaseBrowser::labelTextChanged(juce::Label* labelThatHasChanged)
{
    if (labelThatHasChanged == &searchLabel)
    {
        clearListBox();
        sqlQuery(searchLabel.getText().toStdString());
    }
    else if (labelThatHasChanged == &hostLabel)
    {
        connectToDB();
    }
}

void DistantDataBaseBrowser::clearListBox()
{
    names.clear();
    namesbis.clear();
    durations.clear();
    dates.clear();
    files.clear();
    numRows = 0;
    table.updateContent();

}

void DistantDataBaseBrowser::cellClicked(int rowNumber, int columnID, const juce::MouseEvent& e)
{
    if (conn.connected())
    {
        if (!files[rowNumber].isEmpty())
        {
            startStopButton.setEnabled(true);
            transport.stop();
            std::string filePath = std::string("Z:\\" + files[table.getSelectedRow()].toStdString());
            juce::File sourceFile(filePath);
            std::string fileName = sourceFile.getFileNameWithoutExtension().toStdString();
            juce::String pathToTest = juce::String(Settings::convertedSoundsPath + "\\" + fileName + ".wav");
            juce::File fileToTest(pathToTest);
            if (sourceFile.exists())
            {
                if (juce::AudioFormatReader* reader = formatManager.createReaderFor(fileToTest))
                {
                    loadFile(fileToTest.getFullPathName());
                    delete reader;
                }
                else
                    loadFile(juce::String(startFFmpeg(filePath)));
            }
            else
                resetThumbnail();
        }
        else
        {
            resetThumbnail();
        }
        if (autoPlayButton.getToggleState())
        {
            if (!transport.isPlaying())
                startOrStop();
        }
        if (e.getNumberOfClicks() == 2)
        {
            if (!transport.isPlaying())
                startOrStop();
        }
    }
}

void DistantDataBaseBrowser::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &thumbnail)
        repaint();
    else if (source == &transport)
    {
        if (!transport.isPlaying())
        {
            transport.stop();
            transport.setPosition(0);
            startStopButton.setColour(juce::TextButton::buttonColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        }
    }
}

void DistantDataBaseBrowser::startOrStop()
{
    if (transport.isPlaying())
    {
        transport.stop();
        transport.setPosition(0);
        startStopButton.setColour(juce::TextButton::buttonColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    }
    else
    {
        cuePlay->sendChangeMessage();
        transport.start();
        startStopButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
    }
}


void DistantDataBaseBrowser::timerCallback()
{
    playHead.setVisible(true);

    auto audioPosition = (float)transport.getCurrentPosition();
    auto currentPosition = transport.getLengthInSeconds();
    auto drawPosition = ((audioPosition / currentPosition) * (float)thumbnailBounds.getWidth())
        + (float)thumbnailBounds.getX();
    playHead.setTopLeftPosition(drawPosition, 30);

    auto elapsedTime = secondsToMMSS(transport.getCurrentPosition());
    auto remainingTime = secondsToMMSS(transport.getLengthInSeconds() - transport.getCurrentPosition());
    timeLabel.setText(elapsedTime + " // " + remainingTime, juce::NotificationType::dontSendNotification);
}

void DistantDataBaseBrowser::mouseDown(const juce::MouseEvent& e)
{
    if (e.x < thumbnailBounds.getX())
    {
        mouseDrag(e);
        startDrag = true;
    }
    else if (e.x > thumbnailBounds.getX())
    {
        mouseDrag(e);
        startDrag = false;
    }
}

void DistantDataBaseBrowser::mouseDrag(const juce::MouseEvent& e)
{
    if (startDrag == true)
    {
        fileDraggedFromDataBase->sendChangeMessage();
    }
    else if (e.x > thumbnailBounds.getX() && e.y > thumbnailBounds.getY())
    {
        double mouseDragRelativeXPosition = getMouseXYRelative().getX() - thumbnailBounds.getPosition().getX();
        double mouseDragInSeconds = (((float)mouseDragRelativeXPosition * (float)transport.getLengthInSeconds()) / (float)thumbnailBounds.getWidth());
        transport.setPosition(mouseDragInSeconds);
    }
    mouseDragged = true;
}
void DistantDataBaseBrowser::mouseUp(const juce::MouseEvent& e)
{
    if (mouseDragged == true)
    {
        fileDroppedFromDataBase->sendChangeMessage();
        mouseDragged = false;
    }
    startDrag = false;
}

juce::File DistantDataBaseBrowser::getSelectedFile()
{
    return file;
}

juce::String DistantDataBaseBrowser::getSelectedSoundName()
{
    return names[table.getSelectedRow()];
}

void DistantDataBaseBrowser::tableSortOrderChanged(juce::TableHeaderComponent*)
{
    //if (table.getHeader().getSortColumnId() != 4)
    sqlQuery(searchLabel.getText().toStdString(), table.getHeader().getSortColumnId(), !table.getHeader().isSortedForwards());
}
void DistantDataBaseBrowser::tableColumnsChanged(juce::TableHeaderComponent* tableHeader)
{

}
void DistantDataBaseBrowser::tableColumnsResized(juce::TableHeaderComponent* tableHeader)
{

}
void DistantDataBaseBrowser::tableColumnDraggingChanged(juce::TableHeaderComponent* tableHeader, int columnIdNowBeingDragged)
{

}

bool DistantDataBaseBrowser::loadFile(const juce::String& path)
{

    file = juce::File(path);
    //deleteFile();
    if (juce::AudioFormatReader* reader = formatManager.createReaderFor(file))
    {

        //transport
        std::unique_ptr<juce::AudioFormatReaderSource> tempSource(new juce::AudioFormatReaderSource(reader, true));
        transport.setSource(tempSource.get());
        transport.addChangeListener(this);
        transport.setGain(1.0);
        resampledSource.setResamplingRatio(reader->sampleRate / Settings::sampleRate);
        fileSampleRate = reader->sampleRate;
        playSource.reset(tempSource.release());
        thumbnail.setSource(new juce::FileInputSource(file));
        repaint();
        return true;
    }
    else
        return false;
}


const juce::String DistantDataBaseBrowser::startFFmpeg(std::string filePath)
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
    std::string cmdstring = std::string("\"" + newFFmpegPath + "\" -i \"" + newFilePath + "\" -ar 48000 -y \"" + newConvertedFilesPath + rawnamedoubleslash + ".wav\"");
    std::wstring w = (utf8_to_utf16(cmdstring));
    LPWSTR str = const_cast<LPWSTR>(w.c_str());
    ////////////Launch FFMPEG
    PROCESS_INFORMATION pi;
    STARTUPINFOW si;
    ZeroMemory(&si, sizeof(si));
    BOOL logDone = CreateProcessW(NULL, str, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
    if (logDone)
    {
        WaitForSingleObject(pi.hProcess, INFINITE);
    }



    /////////////////////return created file path
    std::string returnFilePath = std::string(newConvertedFilesPath + "\\" + rawname + ".wav");
    //DBG(returnFilePath);
    std::string returnFilePathBackslah = std::regex_replace(returnFilePath, std::regex(R"(\\)"), R"(\\)");
    juce::String returnedFile = juce::String(returnFilePath);
    Settings::tempFiles.add(returnedFile);
    return returnedFile;
}

void DistantDataBaseBrowser::valueChanged(juce::Value& value)
{
    if (value.refersToSameSourceAs(Settings::sampleRateValue))
    {
        actualSampleRate = Settings::sampleRate;
        resampledSource.setResamplingRatio(fileSampleRate / Settings::sampleRate);
    }
}

juce::String DistantDataBaseBrowser::secondsToMMSS(int seconds)
{
    int timeSeconds = seconds % 60;
    int timeMinuts = trunc(seconds / 60);
    juce::String timeString;
    if (timeSeconds < 10)
        timeString = juce::String(timeMinuts) + ":0" + juce::String(timeSeconds);
    else
        timeString = juce::String(timeMinuts) + ":" + juce::String(timeSeconds);
    return timeString;
}


std::wstring DistantDataBaseBrowser::utf8_to_utf16(const std::string& utf8)
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

void DistantDataBaseBrowser::connectToDB()
{
    /*juce::WindowsRegistry::setValue(juce::String("HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\ODBC\\ODBC.INI\\DistantDB\\Database"), "ABC4");
    juce::WindowsRegistry::setValue(juce::String("HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\ODBC\\ODBC.INI\\DistantDB\\Driver"), "C:\\Windows\\system32\\SQLSRV32.dll");
    juce::WindowsRegistry::setValue(juce::String("HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\ODBC\\ODBC.INI\\DistantDB\\LastUser"), "SYSADM");
    juce::WindowsRegistry::setValue(juce::String("HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\ODBC\\ODBC.INI\\DistantDB\\Server"), hostLabel.getText());*/
    //connectButton.setButtonText("Connecting...");

    //ADD current adress to file
    clearListBox();

    juce::String serverFilePath = juce::File::getCurrentWorkingDirectory().getFullPathName() + "\\LastsServers.txt";
    juce::File serverFile(serverFilePath);
    if (!serverFile.exists())
    {
       serverFile.create();
    }
    if (!checkIfHostExistInList(hostLabel.getText()))
    {
        serverFile.appendText(hostLabel.getText() + "\n");
    }

    //serverFile.appendText("\n");

    updateLastServersBox();


    juce::String connectionString = "Driver=ODBC Driver 17 for SQL Server;Server=" + hostLabel.getText() + ";Database=ABC4;Uid=SYSADM;Pwd=SYSADM;";

    thread.setString(connectionString);
    thread.setHost(hostLabel.getText());
    thread.setConnection(conn);
    thread.launchThread();

    if (thread.isConnected == true)
    {
        initialize();
    }
    //auto const connection_string = NANODBC_TEXT(connectionString.toStdString());
    //try
    //{
    //    conn.connect(connection_string, 2);
    //    //nanodbc::connection conn(connection_string);
    //    //conn.dbms_name();
    //}
    //catch (std::runtime_error const& e)
    //{
    //    std::cerr << e.what() << std::endl;
    //}

    //if (conn.connected())
    //{
    //    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
    //        "Distant Database Found",
    //        "");

    //    //DISK MAPPING
    //    std::string cmdstring = std::string("net use z: \\\\" + hostLabel.getText().toStdString() + "\\sons$");
    //    std::wstring w = (utf8_to_utf16(cmdstring));
    //    LPWSTR str = const_cast<LPWSTR>(w.c_str());
    //    DBG(str);
    //    PROCESS_INFORMATION pi;
    //    STARTUPINFOW si;
    //    si.wShowWindow = SW_SHOW;
    //    si.dwFlags = STARTF_USESHOWWINDOW;
    //    ZeroMemory(&si, sizeof(si));
    //    BOOL logDone = CreateProcessW(NULL, str, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
    //    if (logDone)
    //    {
    //        WaitForSingleObject(pi.hProcess, INFINITE);
    //    }


        /*initialize();
    }
    else
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
            "Connection Error",
            "");
    }
    connectionLabel.setVisible(false);*/
}

void DistantDataBaseBrowser::exitSignalSent()
{
    DBG("thread fini");
    const juce::MessageManagerLock mmLock;
    if (thread.isConnected)
    {
        initialize();
    }
}

void DistantDataBaseBrowser::updateLastServersBox()
{
    lastServerBox.clear(juce::NotificationType::dontSendNotification);
    juce::String serverFilePath = juce::File::getCurrentWorkingDirectory().getFullPathName() + "\\LastsServers.txt";
    juce::File serverFile(serverFilePath);
    if (serverFile.exists())
    {
        juce::StringArray lastServers;
        serverFile.readLines(lastServers);
        int numServers = lastServers.size();
        lastServerBox.addItemList(lastServers, 1);
    }
}

bool DistantDataBaseBrowser::checkIfHostExistInList(juce::String host)
{
    juce::String serverFilePath = juce::File::getCurrentWorkingDirectory().getFullPathName() + "\\LastsServers.txt";
    juce::File serverFile(serverFilePath);
    if (serverFile.exists())
    {
        juce::StringArray lastServers;
        serverFile.readLines(lastServers);
        if (lastServers.contains(host))
            return true;
        else
            return false;
    }
    else
        return false;
}

void DistantDataBaseBrowser::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &lastServerBox)
    {
        hostLabel.setText(lastServerBox.getText(), juce::NotificationType::sendNotification);
    }
}

void DistantDataBaseBrowser::resetThumbnail()
{
    thumbnail.setSource(nullptr);
    transport.setSource(nullptr);
    startStopButton.setEnabled(false);
    startStopButton.setColour(juce::TextButton::buttonColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}