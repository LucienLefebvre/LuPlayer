/*
  ==============================================================================

    DataBaseImport.cpp
    Created: 26 Mar 2021 8:14:16pm
    Author:  DPR

  ==============================================================================
*/

#include <JuceHeader.h>
#include "DataBaseImport.h"
using namespace std;
using namespace nanodbc;
//==============================================================================
DataBaseImport::DataBaseImport() : thumbnailCache(5), thumbnail(521, formatManager, thumbnailCache), resampledSource(&transport, false, 2)
{
    juce::Timer::startTimer(40);

    addAndMakeVisible(&table);
    table.setModel(this);
    table.getHeader().addColumn("Name", 1, 400);
    table.getHeader().addColumn("Duration", 2, 100);
    table.getHeader().addColumn("File", 3, 200);
    table.getHeader().setColour(juce::TableHeaderComponent::ColourIds::backgroundColourId, juce::Colour(40, 134, 189));
    table.addComponentListener(this);
    table.addMouseListener(this, true);

    addAndMakeVisible(&importButton);
    importButton.onClick = [this] {feedDB(); };
    importButton.setButtonText("Import");

    addAndMakeVisible(&clearButton);
    clearButton.onClick = [this] {clear(); };
    clearButton.setButtonText("Clear");

    addAndMakeVisible(startStopButton);
    startStopButton.setColour(juce::TextButton::buttonColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    startStopButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    startStopButton.onClick = [this] { startOrStop(); };
    startStopButton.setEnabled(false);

    addAndMakeVisible(&autoPlayButton);
    autoPlayButton.setToggleState(false, juce::NotificationType::dontSendNotification);

    addAndMakeVisible(&playHead);

    thumbnail.addChangeListener(this);
    transport.addChangeListener(this);
    formatManager.registerBasicFormats();

    DBG(juce::SystemStats::getComputerName());
}

DataBaseImport::~DataBaseImport()
{
    transport.removeAllChangeListeners();
}

void DataBaseImport::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
    g.setColour(juce::Colour(40, 134, 189));
    thumbnail.drawChannels(g,
        thumbnailBounds,
        0,                                    // start time
        (float)thumbnail.getTotalLength(),             // end time
        2.0);                                  // vertical zoom
    g.fillRect(getWidth() / 2, 0, 4, getHeight());
}

void DataBaseImport::resized()
{
    table.setBounds(5, 30, getWidth() / 2 - 5, getHeight() - 30);
    importButton.setBounds(5, 0, 100, 25);
    clearButton.setBounds(getWidth() / 2 - 100, 0, 100, 25);
    table.getHeader().setColumnWidth(1, getWidth() * 4 / 16 - 5);
    table.getHeader().setColumnWidth(2, getWidth() * 1  / 16 - 4);
    table.getHeader().setColumnWidth(3, getWidth() * 3 / 16 - 4);
    thumbnailBounds.setBounds(getWidth() / 2 + 4, 30, getWidth() / 2, getHeight() - 30);
    startStopButton.setBounds(getWidth() / 2 + 4, 0, 100, 25);
    autoPlayButton.setBounds(getWidth() / 2 + 4 + 101, 0, 100, 25);
    playHead.setSize(1, getHeight() - 30);
}

bool DataBaseImport::isInterestedInFileDrag(const juce::StringArray& files)
{
    for (auto file : files)
        if ((file.contains(".wav")) || (file.contains(".WAV"))
            || (file.contains(".bwf")) || (file.contains(".BWF"))
            || (file.contains(".aiff")) || (file.contains(".AIFF"))
            || (file.contains(".aif")) || (file.contains(".AIF"))
            || (file.contains(".flac")) || (file.contains(".FLAC"))
            || (file.contains(".opus")) || (file.contains(".OPUS"))
            || (file.contains(".mp3")) || (file.contains(".MP3")))
        {
            return true;
        }
    return false;
}

void DataBaseImport::filesDropped(const juce::StringArray& files, int x, int y)
{
    for (auto file : files)
    {
        filesToImport.add(file);
        juce::File fileToImport(file);
        names.add(fileToImport.getFileNameWithoutExtension());
        loadFile(file, false);
        std::string duration = std::string(juce::String((float)transport.getLengthInSeconds()*1000).toStdString());
        durations.add(duration);
        durationsMMSS.add(secondsToMMSS(transport.getLengthInSeconds()));
    }
    numRows = filesToImport.size();
    table.updateContent();
}
int DataBaseImport::getNumRows()
{
    return numRows;
}
void DataBaseImport::fileDragMove(const juce::StringArray& files, int x, int y)
{

}
void DataBaseImport::fileDragEnter(const juce::StringArray& files, int x, int y)
{

}
void DataBaseImport::fileDragExit(const juce::StringArray& files)
{

}
void DataBaseImport::paintRowBackground(juce::Graphics& g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected)
{
    auto alternateColour = getLookAndFeel().findColour(juce::ListBox::backgroundColourId)
        .interpolatedWith(getLookAndFeel().findColour(juce::ListBox::textColourId), 0.03f);
    if (rowIsSelected)
        g.fillAll(juce::Colour(40, 134, 189));
    else if (rowNumber % 2)
        g.fillAll(alternateColour);
}
void DataBaseImport::paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool /*rowIsSelected*/)
{
    g.setColour(getLookAndFeel().findColour(juce::ListBox::textColourId));
    if (rowNumber == table.getSelectedRow())
        g.setColour(juce::Colours::black);
    if (columnId == 1)
    {
        g.drawText(names[rowNumber], 2, 0, width - 4, height, juce::Justification::centredLeft, true);
    }
    else if (columnId == 2)
    {
        g.drawText(durationsMMSS[rowNumber], 2, 0, width - 4, height, juce::Justification::centredLeft, true);
    }
    if (columnId == 3)
    {
        g.drawText(filesToImport[rowNumber], 2, 0, width - 4, height, juce::Justification::centredLeft, true);
    }
}
juce::String DataBaseImport::getText(const int columnNumber, const int rowNumber) const
{
    return names[rowNumber];
}

void DataBaseImport::setText(const int columnNumber, const int rowNumber, const juce::String& newText)
{
    names.set(rowNumber, newText);
}
juce::Component* DataBaseImport::refreshComponentForCell(int rowNumber, int columnId, bool /*isRowSelected*/, juce::Component* existingComponentToUpdate)
{
    if (columnId == 1)
    {
        auto* textLabel = static_cast<EditableTextCustomComponent*> (existingComponentToUpdate);

        if (textLabel == nullptr)
            textLabel = new EditableTextCustomComponent(*this);

        textLabel->setRowAndColumn(rowNumber, columnId);
        return textLabel;
    }
    else
        return nullptr;
}

void DataBaseImport::clear()
{
    transport.setSource(nullptr);
    filesToImport.clear();
    names.clear();
    durations.clear();
    durationsMMSS.clear();
    numRows = 0;
    table.updateContent();
}

void DataBaseImport::returnKeyPressed(int lastRowSelected)
{
    filesToImport.remove(lastRowSelected);
    names.remove(lastRowSelected);
    durations.remove(lastRowSelected);
    durationsMMSS.remove(lastRowSelected);
    numRows = names.size();
    table.updateContent();
}

void DataBaseImport::deleteKeyPressed(int lastRowSelected)
{
    returnKeyPressed(lastRowSelected);
}

void DataBaseImport::cellClicked(int rowNumber, int columnID, const juce::MouseEvent&)
{
    DBG("clicked");
    loadFile(filesToImport[rowNumber], true);
    if (autoPlayButton.getToggleState())
    {
        if (!transport.isPlaying())
            startOrStop();
    }
}

void DataBaseImport::feedDB()
{
    for (auto i = 0; i < filesToImport.size(); i++)
    {

        //startFFmpeg(filesToImport[i].toStdString());
        juce::File createdFile(startFFmpeg(filesToImport[i].toStdString()).toStdString());
        std::string fileName = createdFile.getFileName().toStdString();


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

        nextIdString = juce::String(nextIdInt).toStdString();
        DBG(nextIdString);
        std::string injectionString = "DECLARE @myid uniqueidentifier "
            "SET @myid = NEWID() "
            "INSERT INTO ABC4.SYSADM.T_ITEM(GUID_ITEM, ID_ITEM, STRING_2, DATE_BEG_ITEM, DATE_END_ITEM, TYPE_ITEM, TYPE1_ITEM, MODE, ALGO, [FORMAT], FREQUENCY, [PATH], [FILE], RECORDED, [STATE], VIRTUAL, FILE_LENGTH, STATION_REC, USER_REC, LOCK, RTA, RTB, BROADCASTABLE, ARCHIVABLE, PROTECTED, STRING_3, STRING_4, STRING_5, STRING_6, STRING_9, STRING_10, VALUE_1, ITEM_DURATION) "
            "VALUES(@myid, " + nextIdString + ", '" + names[i].toStdString() + "', GETDATE(), GETDATE(), 1, 10000, 0, 9, 8, 48, '\\\\LAPTOP-67VRD21A\\SONS$\\ADMIN\\', '" + fileName + "', 1, 2, 7, 10801450, 'LAPTOP - 67VRD21A', 'DPR', 0, 0, 0, 0, 0, 0, '', '', '', '', '', '', 0," + durations[i].toStdString() + ")";
        DBG(injectionString);

        execute(conn, injectionString);
    }
}

const juce::String DataBaseImport::startFFmpeg(std::string filePath)
{
    std::string USES_CONVERSION_EX;

    std::string ffmpegpath = juce::String(juce::File::getCurrentWorkingDirectory().getFullPathName() + "\\ffmpeg.exe").toStdString();
    //FFmpegPath = Settings::FFmpegPath.toStdString();
    std::string convertedFilesPath = "D:\\SONS\\ADMIN";
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
    std::string cmdstring = std::string("\"" + newFFmpegPath + "\" -i \"" + newFilePath + "\" -acodec mp2 -ar 48000 -ab 256k -ac 2 -af \"volume = -9dB\" -y \"" + newConvertedFilesPath + rawnamedoubleslash + ".wav\"");
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
    std::string returnFilePath = std::string(newConvertedFilesPath + "\\" + rawname + ".wav");
    //DBG(returnFilePath);
    std::string returnFilePathBackslah = std::regex_replace(returnFilePath, std::regex(R"(\\)"), R"(\\)");
    juce::String returnedFile = juce::String(returnFilePath);

    return returnedFile;
}

bool DataBaseImport::loadFile(const juce::String& path, bool dispalyThumbnail)
{

    juce::File file = juce::File(path);
    //deleteFile();
    if (juce::AudioFormatReader* reader = formatManager.createReaderFor(file))
    {
        DBG("loadFile");
        //transport
        std::unique_ptr<juce::AudioFormatReaderSource> tempSource(new juce::AudioFormatReaderSource(reader, true));
        transport.setSource(tempSource.get());
        transport.addChangeListener(this);
        transport.setGain(1.0);
        resampledSource.setResamplingRatio(reader->sampleRate / Settings::sampleRate);
        double fileSampleRate = reader->sampleRate;
        playSource.reset(tempSource.release());
        if (dispalyThumbnail)
        {
            thumbnail.setSource(new juce::FileInputSource(file));
            repaint();
        }
        startStopButton.setEnabled(true);
        return true;
    }
    else
        return false;
}

void DataBaseImport::startOrStop()
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

void DataBaseImport::mouseDown(const juce::MouseEvent& e)
{

    if (e.x > thumbnailBounds.getX())
    {
        mouseDrag(e);
    }
}

void DataBaseImport::mouseDrag(const juce::MouseEvent& e)
{
    if (e.x > thumbnailBounds.getX() && e.y > thumbnailBounds.getY())
    {
        double mouseDragRelativeXPosition = getMouseXYRelative().getX() - thumbnailBounds.getPosition().getX();
        double mouseDragInSeconds = (((float)mouseDragRelativeXPosition * (float)transport.getLengthInSeconds()) / (float)thumbnailBounds.getWidth());
        transport.setPosition(mouseDragInSeconds);
    }

}

void DataBaseImport::timerCallback()
{
    auto audioPosition = (float)transport.getCurrentPosition();
    auto currentPosition = transport.getLengthInSeconds();
    auto drawPosition = ((audioPosition / currentPosition) * (float)thumbnailBounds.getWidth())
        + (float)thumbnailBounds.getX();
    playHead.setTopLeftPosition(drawPosition, 30);
}

void DataBaseImport::changeListenerCallback(juce::ChangeBroadcaster* source)
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

juce::String DataBaseImport::secondsToMMSS(int seconds)
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