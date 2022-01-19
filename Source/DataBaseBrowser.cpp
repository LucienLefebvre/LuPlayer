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
DataBaseBrowser::DataBaseBrowser() : thumbnailCache(5), thumbnail(521, formatManager, thumbnailCache), resampledSource(&transport, false, 2), convertProgress(progression)
{
    auto const connection_string = NANODBC_TEXT("Driver=ODBC Driver 17 for SQL Server;Server=localhost\\NETIA;Database=ABC4;Uid=SYSADM;Pwd=SYSADM;");
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
    table.getHeader().addColumn("Date", 3, 150);
    if (showFileColumn)
        table.getHeader().addColumn("File", 4, 50, juce::TableHeaderComponent::ColumnPropertyFlags::notResizableOrSortable);

    //table.getHeader().addColumn("File", 4, 200);
    table.addMouseListener(this, true);
    //addMouseListener(this, true);

    table.addComponentListener(this);
    table.getHeader().addListener(this);

    addChildComponent(&timeLabel);
    timeLabel.setFont(juce::Font(25.0f, juce::Font::plain).withTypefaceStyle("Regular"));
    timeLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(&nameLabel);
    nameLabel.setFont(juce::Font(25.0f, juce::Font::plain).withTypefaceStyle("Regular"));
    nameLabel.setJustificationType(juce::Justification::centred);
    nameLabel.setColour(juce::Label::ColourIds::textColourId, juce::Colour(229, 149, 0));

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

    addAndMakeVisible(&todayButton);
    todayButton.setButtonText("Today");
    todayButton.setBounds(426, 1, 59, 25);
    todayButton.onClick = [this] {todayButtonClicked(); };
    todayButton.setToggleState(true, juce::NotificationType::dontSendNotification);

    addAndMakeVisible(&batchConvertButton);
    batchConvertButton.onClick = [this] { batchConvertButtonClicked(); };
    batchConvertButton.setBounds(489, 3, 64, 21);
    batchConvertButton.setButtonText("Convert");

    thumbnail.addChangeListener(this);
    addAndMakeVisible(&playHead);

    table.getHeader().setColour(juce::TableHeaderComponent::ColourIds::backgroundColourId, juce::Colour(40, 134, 189));

    fileDraggedFromDataBase = new juce::ChangeBroadcaster();
    fileDroppedFromDataBase = new juce::ChangeBroadcaster();
    cuePlay = new juce::ChangeBroadcaster();

    formatManager.registerBasicFormats();

    addChildComponent(convertProgress);
    convertProgress.setColour(juce::ProgressBar::ColourIds::foregroundColourId, juce::Colour(40, 134, 189));

    try
    {
       
    conn.connect(connection_string, 1000);
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
    table.getHeader().setColumnWidth(1, getWidth() * 5/16 - 50);
    table.getHeader().setColumnWidth(2, getWidth() / 16 - 4);
    table.getHeader().setColumnWidth(3, getWidth()* 2 / 16 - 4);
    table.getHeader().setColumnWidth(4, 45);

    thumbnailBounds.setBounds(getWidth() / 2 + 4, 30, getWidth() / 2, getHeight() - 30);
    startStopButton.setBounds(getWidth() / 2 + 4, 0, 100, 25);
    autoPlayButton.setBounds(getWidth() / 2 + 4 + 101, 0, 90, 25);
    timeLabel.setBounds(getWidth() - timeLabelWidth, 0, timeLabelWidth, 25);
    nameLabel.setBounds(autoPlayButton.getRight(), 0, getWidth() - autoPlayButton.getRight() - timeLabelWidth, 25);
    playHead.setSize(1, getHeight() - 30);

    convertProgress.setBounds(getWidth() / 2 - 204, 5, 200, 20);


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
    else if (columnId == 4 && showFileColumn == true)
    {
        std::string filePath = std::string("D:\\SONS\\ADMIN\\" + files[rowNumber].toStdString());
        juce::File sourceFile(filePath);
        std::string fileName = sourceFile.getFileNameWithoutExtension().toStdString();
        juce::String pathToTest = juce::String(Settings::convertedSoundsPath + "\\" + fileName + ".wav");
        juce::File fileToTest(pathToTest);
        if (juce::AudioFormatReader* reader = formatManager.createReaderFor(fileToTest))
        {
            g.setColour(juce::Colours::green);
            g.drawText("Yes", 2, 0, width - 4, height, juce::Justification::centredLeft, true);
            delete reader;
        }
        else
        {
            g.setColour(juce::Colours::orange);
            g.drawText("Not converted", 2, 0, width - 4, height, juce::Justification::centredLeft, true);
            delete reader;
        }
        if (isConverting)
        {
            for (int i = 0; i < myConvertObjects.size(); i++)
            {
                if (myConvertObjects[i]->getId() == rowNumber)
                {
                    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
                    g.setColour(juce::Colours::orange);
                    juce::String progression = juce::String(juce::roundToInt(myConvertObjects[i]->getProgression() * 100)) + "%";
                    g.drawText(progression, 2, 0, width - 4, height, juce::Justification::centredLeft, true);
                }
            }
        }
    }

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
        conn.connect(connection_string, 1000);

    std::string searchString;
    std::string todayString = "";
    if (todayButton.getToggleState())
        todayString = "AND CAST(ABC4.SYSADM.T_ITEM.DATE_BEG_ITEM AS DATE) LIKE CAST(GETDATE() AS DATE)";
    if (sortColum == 0)
        searchString = std::string("SELECT * FROM ABC4.SYSADM.T_ITEM WHERE ABC4.SYSADM.T_ITEM.STRING_2 LIKE '%" + search + "%' AND STATION_DELETE IS NULL " + todayString);
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
            searchString = std::string("SELECT * FROM ABC4.SYSADM.T_ITEM WHERE ABC4.SYSADM.T_ITEM.STRING_2 LIKE '%" + search + "%' AND STATION_DELETE IS NULL " + todayString + " ORDER BY " + columnToSort + " ASC");
        else if (sortDirection == 1)
            searchString = std::string("SELECT * FROM ABC4.SYSADM.T_ITEM WHERE ABC4.SYSADM.T_ITEM.STRING_2 LIKE '%" + search + "%' AND STATION_DELETE IS NULL " + todayString + " ORDER BY " + columnToSort + " DESC");
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

void DataBaseBrowser::cellClicked(int rowNumber, int columnID, const juce::MouseEvent& e)
{
    transport.setSource(nullptr);
    thumbnail.setSource(nullptr);
    fileLoaded = false;
    nameLabel.setText("", juce::NotificationType::dontSendNotification);
    timeLabel.setVisible(false);
    startStopButton.setColour(juce::TextButton::buttonColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    repaint();
    if (!files[rowNumber].isEmpty())
    {
        checkAndConvert(rowNumber);
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
            play();
    }
    if (e.getNumberOfClicks() == 2)
    {
        if (!transport.isPlaying())
            play();
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

    //delete conversion object
    for (int i = 0; i < myConvertObjects.size(); i++)
    {
        if (source == myConvertObjects[i]->finishedBroadcaster)
        {
            //load file and delete object
            if (!isBatchConverting)
                fileLoaded = loadFile(myConvertObjects[i]->getReturnedFile());
            if (wantToPlay)
            {
                startOrStop();
            }
            myConvertObjects[i]->setVisible(false);
            if (myConvertObjects[i]->getId() == table.getSelectedRow() && wantToLoadFile == true && mouseIsUp == true)
            {
                fileDroppedFromDataBase->sendChangeMessage();
                wantToLoadFile = false;
            }
            myConvertObjects.remove(i);
            //remove progress bar
            if (myConvertObjects.size() == 0)
            {
                convertProgress.setVisible(false);
                isConverting = false;
            }
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
        cuePlay->sendChangeMessage();
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

    auto elapsedTime = secondsToMMSS(transport.getCurrentPosition());
    auto remainingTime = secondsToMMSS(transport.getLengthInSeconds() - transport.getCurrentPosition());
    timeLabel.setText(elapsedTime + " // " + remainingTime, juce::NotificationType::dontSendNotification);


    if (isConverting)
        repaint();
    if (isBatchConverting)
    {
        batchConvert();
        repaint();
    }
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
    mouseIsUp = false;

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
    mouseIsUp = true;
    if (mouseDragged == true)
    {
        if (fileLoaded)
        {
            fileDroppedFromDataBase->sendChangeMessage();
            mouseDragged = false;
        }
        else 
            wantToLoadFile = true;
    }
    startDrag = false;
}

juce::File DataBaseBrowser::getSelectedFile()
{
    std::string filePath = std::string(Settings::convertedSoundsPath.toStdString() + "\\" + files[table.getSelectedRow()].trimCharactersAtEnd(".BWF").toStdString() + ".wav");
    juce::File sourceFile(filePath);
    DBG("chemin fichier" << filePath);
    if (juce::AudioFormatReader* reader = formatManager.createReaderFor(sourceFile))
    {
        DBG("fichier converti");
        delete reader;
        return filePath;
    }
    else
    {
        delete reader;
        return juce::File("D:\\SONS\\ADMIN\\" + files[table.getSelectedRow()].toStdString());
    }
}

juce::String DataBaseBrowser::getSelectedSoundName()
{
    return names[table.getSelectedRow()];
}

void DataBaseBrowser::tableSortOrderChanged(juce::TableHeaderComponent*)
{
    if (table.getHeader().getSortColumnId() != 4)
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
        fileSampleRate = reader->sampleRate;
        playSource.reset(tempSource.release());
        thumbnail.setSource(new juce::FileInputSource(file));
        repaint();
        nameLabel.setText(names[table.getSelectedRow()], juce::NotificationType::dontSendNotification);
        timeLabel.setVisible(true);
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
    Settings::tempFiles.add(returnedFile);
    return returnedFile;
}

void DataBaseBrowser::valueChanged(juce::Value& value)
{
    if (value.refersToSameSourceAs(Settings::sampleRateValue))
    {
        actualSampleRate = Settings::sampleRate;
        resampledSource.setResamplingRatio(fileSampleRate / Settings::sampleRate);
    }
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

void DataBaseBrowser::convertAllSounds()
{

}

void DataBaseBrowser::todayButtonClicked()
{
    //todayButton.setToggleState(!todayButton.getToggleState(), juce::NotificationType::dontSendNotification);
    sqlQuery(searchLabel.getText().toStdString(), table.getHeader().getSortColumnId(), !table.getHeader().isSortedForwards());
}

bool DataBaseBrowser::checkAndConvert(int rowNumber)
{
    startStopButton.setEnabled(true);
    transport.stop();
    std::string filePath = std::string("D:\\SONS\\ADMIN\\" + files[rowNumber].toStdString());
    juce::File sourceFile(filePath);
    std::string fileName = sourceFile.getFileNameWithoutExtension().toStdString();
    juce::String pathToTest = juce::String(Settings::convertedSoundsPath + "\\" + fileName + ".wav");
    juce::File fileToTest(pathToTest);
    if (juce::AudioFormatReader* reader = formatManager.createReaderFor(fileToTest))
    {
        if (!isBatchConverting)
            fileLoaded = loadFile(fileToTest.getFullPathName());
        delete reader;
        return true;
    }
    else
    {
        int soundDuration = durations[rowNumber].getIntValue() / 1000;
        myConvertObjects.add(new convertObject(filePath, soundDuration, rowNumber));
        convertObjectIndex++;
        addAndMakeVisible(myConvertObjects.getLast());
        convertProgress.setVisible(true);
        isConverting = true;
        myConvertObjects.getLast()->finishedBroadcaster->addChangeListener(this);
        return false;
    }
}

void DataBaseBrowser::batchConvertButtonClicked()
{
    if (isBatchConverting == false)
    {
        batchConvertButton.setButtonText("Stop");
        isConverting = true;
        isBatchConverting = true;
    }
    else if (isBatchConverting == true)
    {
        batchConvertButton.setButtonText("Convert");
        isBatchConverting = false;
        progression = -1.;
    }
}

void DataBaseBrowser::batchConvert()
{
    if (numRows > 0)
    {
    progression = ((double)batchConvertIndex - myConvertObjects.size() )/ (double)numRows;
    if (myConvertObjects.size() < 3)
    {
        isConverting = true;
        if (batchConvertIndex < numRows)
        {
            checkAndConvert(batchConvertIndex);
            batchConvertIndex++;
        }
    }
    if (batchConvertIndex == numRows)
    {
        isConverting = false;
        isBatchConverting = false;
        batchConvertIndex = 0;
        progression = -1.;
        batchConvertButton.setButtonText("Convert");
    }
    }
}

bool DataBaseBrowser::keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent)
{
    if (isVisible())
    {
        if (key == juce::KeyPress::spaceKey)
        {
            play();
            return false;
        }
    }
}

void DataBaseBrowser::play()
{
    if (fileLoaded)
    {
        startOrStop();
        wantToPlay = false;
    }
    else
    {
        wantToPlay = true;
    }
}
