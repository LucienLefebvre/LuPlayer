/*
  ==============================================================================

    DataBaseBrowser.cpp
    Created: 24 Mar 2021 7:16:03pm
    Author:  DPR

  ==============================================================================
*/
//faire thumbnail dans component à part
#include <JuceHeader.h>
#include "DataBaseBrowser.h"
#include <stdio.h>
#include <string>
#include "nanodbc/nanodbc.h"
#include "Windows.h"
#include "Settings.h"

using namespace std;
using namespace nanodbc;

//==============================================================================
DataBaseBrowser::DataBaseBrowser() : thumbnailCache(5), thumbnail(521, formatManager, thumbnailCache), resampledSource(&transport, false, 2)
{
    auto const connection_string = NANODBC_TEXT("Driver=ODBC Driver 17 for SQL Server;Server=localhost\\NETIA;Database=ABC4;Uid=SYSADM;Pwd=SYSADM;");

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
    refreshButton.onClick = [this] {sqlQuery(searchLabel.getText().toStdString()); };

    table.setModel(this);
    addAndMakeVisible(&table);
    table.setBounds(0, 30, 100, getHeight() - 30);
    table.getHeader().addColumn("Name", 1, 400);
    table.getHeader().addColumn("Duration", 2, 100);
    table.getHeader().addColumn("Date", 3, 200);

    //table.getHeader().addColumn("File", 4, 200);
    table.addMouseListener(this, true);
    //addMouseListener(this, true);

    table.addComponentListener(this);
    table.getHeader().addListener(this);

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

    thumbnail.addChangeListener(this);
    addAndMakeVisible(&playHead);

    table.getHeader().setColour(juce::TableHeaderComponent::ColourIds::backgroundColourId, juce::Colour(40, 134, 189));

    fileDraggedFromDataBase = new juce::ChangeBroadcaster();
    fileDroppedFromDataBase = new juce::ChangeBroadcaster();

    formatManager.registerBasicFormats();

    try
    {
       
    conn.connect(connection_string);
        //nanodbc::connection conn(connection_string);
        conn.dbms_name();
        initialize();
    }
    catch (std::runtime_error const& e)
    {
        std::cerr << e.what() << std::endl;
    }
    


}

DataBaseBrowser::~DataBaseBrowser()
{
    transport.setSource(nullptr);
    juce::Timer::stopTimer();
    thumbnail.setSource(nullptr);
    delete fileDraggedFromDataBase;
    delete fileDroppedFromDataBase;
    table.removeComponentListener(this);
    transport.removeAllChangeListeners();
    //transport.releaseResources();
    //resampledSource.releaseResources();
    conn.disconnect();
}

void DataBaseBrowser::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
    g.setColour(juce::Colour(40, 134, 189));
    thumbnail.drawChannels(g,
        thumbnailBounds,
        0,                                    // start time
        (float)thumbnail.getTotalLength(),             // end time
        2.0);                                  // vertical zoom
    g.fillRect(getWidth() / 2, 0, 4, getHeight());
    //g.fillRect(0, 28, getWidth(), 2);
}

void DataBaseBrowser::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    actualSampleRate = sampleRate;
    actualSamplesPerBlockExpected = samplesPerBlockExpected;

    transport.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void DataBaseBrowser::resized()
{
    table.setBounds(5, 30, getWidth() / 2 - 5, getHeight() - 30);
    table.getHeader().setColumnWidth(1, getWidth() * 5/16 - 5);
    table.getHeader().setColumnWidth(2, getWidth() / 16 - 4);
    table.getHeader().setColumnWidth(3, getWidth()* 2 / 16 - 4);
    thumbnailBounds.setBounds(getWidth() / 2 + 4, 30, getWidth() / 2, getHeight() - 30);
    startStopButton.setBounds(getWidth() / 2 + 4, 0, 100, 25);
    autoPlayButton.setBounds(getWidth() / 2 + 4 + 101, 0, 100, 25);
    playHead.setSize(1, getHeight() - 30);
}

int DataBaseBrowser::getNumRows()
{
    return numRows;
}

void DataBaseBrowser::paintRowBackground(juce::Graphics& g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected)
{
    auto alternateColour = getLookAndFeel().findColour(juce::ListBox::backgroundColourId)
        .interpolatedWith(getLookAndFeel().findColour(juce::ListBox::textColourId), 0.03f);
    if (rowIsSelected)
        g.fillAll(juce::Colour(40, 134, 189));
    else if (rowNumber % 2)
        g.fillAll(alternateColour);
}

void DataBaseBrowser::paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool /*rowIsSelected*/)
{
    /*g.setColour(getLookAndFeel().findColour(juce::ListBox::textColourId));
    g.setFont(font);

    if (auto* rowElement = dataList->getChildElement(rowNumber))
    {
        auto text = rowElement->getStringAttribute(getAttributeNameForColumnId(columnId));

        g.drawText(text, 2, 0, width - 4, height, Justification::centredLeft, true);
    }*/
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
        g.drawText(secondsToMMSS(durations[rowNumber].getIntValue()/1000), 2, 0, width - 4, height, juce::Justification::centredLeft, true);
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

void DataBaseBrowser::initialize()
{
    //sqlQuery("", 3, 1);
    table.getHeader().setSortColumnId(3, false);
}

void DataBaseBrowser::sqlQuery(std::string search, int sortColum, int sortDirection)
{
    clearListBox();
    auto const connection_string = NANODBC_TEXT("Driver=ODBC Driver 17 for SQL Server;Server=localhost\\NETIA;Database=ABC4;Uid=SYSADM;Pwd=SYSADM;");
    if (!conn.connected())
        conn.connect(connection_string);

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

void DataBaseBrowser::labelTextChanged(juce::Label* labelThatHasChanged)
{
    clearListBox();
    sqlQuery(searchLabel.getText().toStdString());
}

void DataBaseBrowser::clearListBox()
{
    names.clear();
    namesbis.clear();
    durations.clear();
    dates.clear();
    files.clear();
    numRows = 0;
    table.updateContent();

}

void DataBaseBrowser::cellClicked(int rowNumber, int columnID, const juce::MouseEvent&)
{
    if (!files[rowNumber].isEmpty())
    {
        startStopButton.setEnabled(true);
        transport.stop();
        std::string filePath = std::string("D:\\SONS\\ADMIN\\" + files[table.getSelectedRow()].toStdString());
        juce::File sourceFile(filePath);
        std::string fileName = sourceFile.getFileNameWithoutExtension().toStdString();
        juce::String pathToTest = juce::String(Settings::convertedSoundsPath + "\\" + fileName + ".wav");
        juce::File fileToTest(pathToTest);
        if (juce::AudioFormatReader* reader = formatManager.createReaderFor(fileToTest))
        {
            loadFile(fileToTest.getFullPathName());
            delete reader;
        }
        else
            loadFile(juce::String(startFFmpeg(filePath)));
    }
    else
    {
        thumbnail.setSource(nullptr);
        transport.setSource(nullptr);
        startStopButton.setEnabled(false);
        startStopButton.setColour(juce::TextButton::buttonColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    }
    if (autoPlayButton.getToggleState())
    {
        if (!transport.isPlaying())
            startOrStop();
    }
}

void DataBaseBrowser::changeListenerCallback(juce::ChangeBroadcaster* source)
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

void DataBaseBrowser::startOrStop()
{
    if (transport.isPlaying())
    {
        transport.stop();
        transport.setPosition(0);
        startStopButton.setColour(juce::TextButton::buttonColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    }
    else
    {
        transport.start();
        startStopButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
    }
}


void DataBaseBrowser::timerCallback()
{
    playHead.setVisible(true);

    auto audioPosition = (float)transport.getCurrentPosition();
    auto currentPosition = transport.getLengthInSeconds();
    auto drawPosition = ((audioPosition / currentPosition) * (float)thumbnailBounds.getWidth())
        + (float)thumbnailBounds.getX();
    playHead.setTopLeftPosition(drawPosition, 30);
}

void DataBaseBrowser::mouseDown(const juce::MouseEvent& e)
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

void DataBaseBrowser::mouseDrag(const juce::MouseEvent& e)
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
void DataBaseBrowser::mouseUp(const juce::MouseEvent& e)
{
    if (mouseDragged == true)
    {
        fileDroppedFromDataBase->sendChangeMessage();
        mouseDragged = false;
    }
    startDrag = false;
}

juce::File DataBaseBrowser::getSelectedFile()
{
    return file;
}

juce::String DataBaseBrowser::getSelectedSoundName()
{
    return names[table.getSelectedRow()];
}

void DataBaseBrowser::tableSortOrderChanged(juce::TableHeaderComponent*)
{
    //if (table.getHeader().getSortColumnId() != 4)
    sqlQuery(searchLabel.getText().toStdString(), table.getHeader().getSortColumnId(), !table.getHeader().isSortedForwards());
}
void DataBaseBrowser::tableColumnsChanged(juce::TableHeaderComponent* tableHeader)
{

}
void DataBaseBrowser::tableColumnsResized(juce::TableHeaderComponent* tableHeader)
{

}
void DataBaseBrowser::tableColumnDraggingChanged(juce::TableHeaderComponent* tableHeader, int columnIdNowBeingDragged)
{

}

bool DataBaseBrowser::loadFile(const juce::String& path)
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
        double fileSampleRate = reader->sampleRate;
        playSource.reset(tempSource.release());
        thumbnail.setSource(new juce::FileInputSource(file));
        repaint();
        return true;
    }
    else
        return false;
}


const juce::String DataBaseBrowser::startFFmpeg(std::string filePath)
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
    BOOL logDone = CreateProcessW(NULL, str, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    if (logDone)
    {
        WaitForSingleObject(pi.hProcess, INFINITE);
    }



    /////////////////////return created file path
    std::string returnFilePath = std::string(newConvertedFilesPath + "\\" + rawname + ".wav");
    //DBG(returnFilePath);
    std::string returnFilePathBackslah = std::regex_replace(returnFilePath, std::regex(R"(\\)"), R"(\\)");
    juce::String returnedFile = juce::String(returnFilePath);

    return returnedFile;
}
juce::String DataBaseBrowser::secondsToMMSS(int seconds)
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
std::wstring DataBaseBrowser::utf8_to_utf16(const std::string& utf8)
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

