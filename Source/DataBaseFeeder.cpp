/*
  ==============================================================================

    DataBaseFeeder.cpp
    Created: 26 Mar 2021 8:14:16pm
    Author:  DPR

  ==============================================================================
*/
//Album Flynt : 35s Vs 2mn30s
//Journée interception : 37s vs 2mn52s
#include <JuceHeader.h>
#include "DataBaseFeeder.h"
using namespace std;
using namespace nanodbc;
//==============================================================================
DataBaseFeeder::DataBaseFeeder() : thumbnailCache(5), thumbnail(521, formatManager, thumbnailCache), resampledSource(&transport, false, 2)
{

    juce::Timer::startTimer(40);
    Settings::sampleRateValue.addListener(this);
    addAndMakeVisible(&table);
    table.setModel(this);
    table.getHeader().addColumn("Name", 1, 400);
    table.getHeader().addColumn("Description", 2, 400);
    table.getHeader().addColumn("Duration", 3, 100, 30, -1, juce::TableHeaderComponent::ColumnPropertyFlags::notSortable);
    table.getHeader().addColumn("Type", 4, 100, 30, -1, juce::TableHeaderComponent::ColumnPropertyFlags::notSortable);
    table.getHeader().addColumn("File", 5, 200, 30, -1, juce::TableHeaderComponent::ColumnPropertyFlags::notSortable);
    table.getHeader().setColour(juce::TableHeaderComponent::ColourIds::backgroundColourId, juce::Colour(40, 134, 189));
    table.addComponentListener(this);
    table.addMouseListener(this, false);

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

    addAndMakeVisible(&openButton);
    openButton.setBounds(5, 0, 100, 25);
    openButton.onClick = [this] { openButtonClicked(); };

    thumbnail.addChangeListener(this);
    transport.addChangeListener(this);
    formatManager.registerBasicFormats();

    feedDbThread.importFinishedBroadcaster->addChangeListener(this);
}

DataBaseFeeder::~DataBaseFeeder()
{
    Settings::sampleRateValue.removeListener(this);
    transport.removeAllChangeListeners();
}

void DataBaseFeeder::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    actualSampleRate = sampleRate;
    actualSamplesPerBlockExpected = samplesPerBlockExpected;

    transport.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void DataBaseFeeder::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));   // clear the background
    g.setColour(juce::Colour(40, 134, 189));
    thumbnail.drawChannels(g,
        thumbnailBounds,
        0,                                    // start time
        (float)thumbnail.getTotalLength(),             // end time
        1.0);                                  // vertical zoom
    g.fillRect(getWidth() / 2, 0, 4, getHeight());
}

void DataBaseFeeder::resized()
{
    table.setBounds(5, 30, getWidth() / 2 - 5, getHeight() - 60);
    importButton.setBounds(getWidth() / 4 - 50, getHeight() - 28, 100, 25);
    clearButton.setBounds(getWidth() / 2 - 100, 0, 100, 25);
    table.getHeader().setColumnWidth(1, getWidth() * 4 / 16 - 5);
    table.getHeader().setColumnWidth(2, getWidth() * 1 / 16 - 4);
    table.getHeader().setColumnWidth(3, getWidth() * 3 / 16 - 4);
    thumbnailBounds.setBounds(getWidth() / 2 + 4, 30, getWidth() / 2, getHeight() - 30);
    startStopButton.setBounds(getWidth() / 2 + 4, 0, 100, 25);
    autoPlayButton.setBounds(getWidth() / 2 + 4 + 101, 0, 100, 25);
    playHead.setSize(1, getHeight() - 30);
}

bool DataBaseFeeder::isInterestedInFileDrag(const juce::StringArray& files)
{
    for (auto file : files)
        if ((file.contains(".wav")) || (file.contains(".WAV"))
            || (file.contains(".bwf")) || (file.contains(".BWF"))
            || (file.contains(".aiff")) || (file.contains(".AIFF"))
            || (file.contains(".aif")) || (file.contains(".AIF"))
            || (file.contains(".flac")) || (file.contains(".FLAC"))
            || (file.contains(".opus")) || (file.contains(".OPUS"))
            || (file.contains(".mp3")) || (file.contains(".MP3"))
            || (file.contains(".mp4")) || (file.contains(".MP4"))
            || (file.contains(".mkv")) || (file.contains(".MKV"))
            || (file.contains(".avi")) || (file.contains(".avi")))
        {
            return true;
        }
    return false;
}

void DataBaseFeeder::filesDropped(const juce::StringArray& files, int x, int y)
{
    addFiles(files);
}

void DataBaseFeeder::sortFiles(int columnToSort)
{
    /*for (auto f = 0; f < sortedFilesToImport.size(); f++)
    {
        sortedNames.set(f, "");
        sortedDurations.set(f, "");
        sortedDurationsMMSS.set(f, "");
    }
    sortedFilesToImport.sort(true);
    for (auto f = 0; f < sortedFilesToImport.size(); f++)
    {
        for (auto i = 0; i < sortedFilesToImport.size(); i++)
        {
            if (filesToImport[i].equalsIgnoreCase(sortedFilesToImport[f]))
            {
                DBG("true");
                sortedNames.set(f, names[i]);
                sortedDurations.set(f, durations[i]);
                sortedDurationsMMSS.set(f, durationsMMSS[i]);
            }
            DBG(f);
        }

    }


    numRows = sortedFilesToImport.size();
    table.updateContent();*/

}
int DataBaseFeeder::getNumRows()
{
    return filesToImport.size();
}
void DataBaseFeeder::fileDragMove(const juce::StringArray& files, int x, int y)
{

}
void DataBaseFeeder::fileDragEnter(const juce::StringArray& files, int x, int y)
{

}
void DataBaseFeeder::fileDragExit(const juce::StringArray& files)
{

}
void DataBaseFeeder::paintRowBackground(juce::Graphics& g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected)
{
    auto alternateColour = getLookAndFeel().findColour(juce::ListBox::backgroundColourId)
        .interpolatedWith(getLookAndFeel().findColour(juce::ListBox::textColourId), 0.03f);
    if (rowIsSelected)
        g.fillAll(juce::Colour(40, 134, 189));
    else if (rowNumber % 2)
        g.fillAll(alternateColour);
}
void DataBaseFeeder::paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool /*rowIsSelected*/)
{
    g.setColour(getLookAndFeel().findColour(juce::ListBox::textColourId));
    if (rowNumber == table.getSelectedRow())
        g.setColour(juce::Colours::black);
    if (columnId == 1)
    {
        g.drawText(filesToImport[rowNumber].name, 2, 0, width - 4, height, juce::Justification::centredLeft, true);
    }
    else if (columnId == 2)
    {
        g.drawText(filesToImport[rowNumber].name2, 2, 0, width - 4, height, juce::Justification::centredLeft, true);
    }
    else if (columnId == 3)
    {
        g.drawText(juce::String(filesToImport[rowNumber].duration), 2, 0, width - 4, height, juce::Justification::centredLeft, true);
    }
    else if (columnId == 4)
    {

    }    
    else if (columnId == 5)
    {
        g.drawText(filesToImport[rowNumber].path, 2, 0, width - 4, height, juce::Justification::centredLeft, true);
    }
}
juce::String DataBaseFeeder::getText(const int columnNumber, const int rowNumber) const
{
    if (columnNumber == 1)
        return filesToImport[rowNumber].name;    
    else if (columnNumber == 2)
        return filesToImport[rowNumber].name2;
}

void DataBaseFeeder::setText(const int columnNumber, const int rowNumber, const juce::String& newText)
{
    FeedThread::FileToImport f;
    f = filesToImport[rowNumber];
    if (columnNumber == 1)
        f.name = newText;
    else if (columnNumber == 2)
        f.name2 = newText;
    filesToImport.set(rowNumber, f);
}

int DataBaseFeeder::getType(const int rowNumber) const
{
    return filesToImport[rowNumber].type;
}

void DataBaseFeeder::setType(const int rowNumber, const int newType)
{
    FeedThread::FileToImport f;
    f = filesToImport[rowNumber];
    f.type = newType;
    filesToImport.set(rowNumber, f);
}



juce::Component* DataBaseFeeder::refreshComponentForCell(int rowNumber, int columnId, bool /*isRowSelected*/, juce::Component* existingComponentToUpdate)
{
    if (columnId == 1 || columnId == 2)
    {
        auto* textLabel = static_cast<EditableTextCustomComponent*> (existingComponentToUpdate);

        if (textLabel == nullptr)
            textLabel = new EditableTextCustomComponent(*this);

        textLabel->setRowAndColumn(rowNumber, columnId);
        return textLabel;
    }
    else if (columnId == 4)
    {
        auto* typeSelector = static_cast<TypeSelectorCustomComponent*> (existingComponentToUpdate);

        if (typeSelector == nullptr)
            typeSelector = new TypeSelectorCustomComponent(*this);

        typeSelector->setRowAndColumn(rowNumber, columnId);
        return typeSelector;
    }
    else    
        return nullptr;
}

void DataBaseFeeder::clear()
{
    transport.setSource(nullptr);
    thumbnail.setSource(nullptr);
    filesToImport.clear();
    names.clear();
    durations.clear();
    durationsMMSS.clear();
    sortedNames.clear();
    sortedDurations.clear();
    sortedDurationsMMSS.clear();
    numRows = 0;
    table.updateContent();
}

void DataBaseFeeder::returnKeyPressed(int lastRowSelected)
{
    filesToImport.remove(lastRowSelected);
    names.remove(lastRowSelected);
    durations.remove(lastRowSelected);
    durationsMMSS.remove(lastRowSelected);
    numRows = names.size();
    table.updateContent();
}

void DataBaseFeeder::deleteKeyPressed(int lastRowSelected)
{
    returnKeyPressed(lastRowSelected);
}

void DataBaseFeeder::cellClicked(int rowNumber, int columnID, const juce::MouseEvent&)
{
    loadFile(filesToImport[rowNumber].path, true);
    if (autoPlayButton.getToggleState())
    {
        if (!transport.isPlaying())
            startOrStop();
    }
}

void DataBaseFeeder::selectedRowsChanged(int lastRowSelected)
{
    loadFile(filesToImport[lastRowSelected].path, true);
    if (autoPlayButton.getToggleState())
    {
        if (!transport.isPlaying())
            startOrStop();
    }
}
void DataBaseFeeder::feedDB()
{
    feedDbThread.setFiles(filesToImport);
    feedDbThread.launchThread(10);
}



bool DataBaseFeeder::loadFile(const juce::String& path, bool dispalyThumbnail)
{

    juce::File file = juce::File(path);
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

void DataBaseFeeder::startOrStop()
{
    if (transport.isPlaying())
    {
        transport.stop();
        transport.setPosition(0);
        startStopButton.setColour(juce::TextButton::buttonColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    }
    else if (transport.getLengthInSeconds() != 0)
    {
        transport.start();
        startStopButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
    }
}

void DataBaseFeeder::mouseDown(const juce::MouseEvent& e)
{

    if (e.x > thumbnailBounds.getX())
    {
        mouseDrag(e);
    }
}

void DataBaseFeeder::mouseDrag(const juce::MouseEvent& e)
{
    if (e.x > thumbnailBounds.getX() && e.y > thumbnailBounds.getY())
    {
        double mouseDragRelativeXPosition = getMouseXYRelative().getX() - thumbnailBounds.getPosition().getX();
        double mouseDragInSeconds = (((float)mouseDragRelativeXPosition * (float)transport.getLengthInSeconds()) / (float)thumbnailBounds.getWidth());
        transport.setPosition(mouseDragInSeconds);
    }

}

void DataBaseFeeder::timerCallback()
{
    auto audioPosition = (float)transport.getCurrentPosition();
    auto currentPosition = transport.getLengthInSeconds();
    auto drawPosition = ((audioPosition / currentPosition) * (float)thumbnailBounds.getWidth())
        + (float)thumbnailBounds.getX();
    playHead.setTopLeftPosition(drawPosition, 30);
}

void DataBaseFeeder::changeListenerCallback(juce::ChangeBroadcaster* source)
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
    else if (source == feedDbThread.importFinishedBroadcaster.get())
    {
        clear();
    }
}

juce::String DataBaseFeeder::secondsToMMSS(int seconds)
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

void DataBaseFeeder::valueChanged(juce::Value& value)
{
    if (value.refersToSameSourceAs(Settings::sampleRateValue))
    {
        actualSampleRate = Settings::sampleRate;
        resampledSource.setResamplingRatio(fileSampleRate / Settings::sampleRate);
    }
}

void DataBaseFeeder::openButtonClicked()
{
    juce::FileChooser chooser("Choose an audio File", juce::File::getSpecialLocation(juce::File::userDesktopDirectory), "*.wav; *.WAV; *.mp3; *.MP3; *.bwf; *.BWF; *.aiff; *.opus; *.flac");
    if (chooser.browseForMultipleFilesToOpen())
    {
        juce::Array<juce::File> f = chooser.getResults();
        juce::StringArray files;
        for (auto file : f)
        {
            files.add(file.getFullPathName());
        }
        addFiles(files);
    }
}

void DataBaseFeeder::addFiles(const juce::StringArray files)
{
    for (auto file : files)
    {
        FeedThread::FileToImport f;
        juce::File currentFile(file);
        if (currentFile.exists())
        {
            if (juce::AudioFormatReader* reader = formatManager.createReaderFor(file))
            {
                std::unique_ptr<juce::AudioFormatReaderSource> tempSource(new juce::AudioFormatReaderSource(reader, true));
                f.duration = tempSource->getAudioFormatReader()->lengthInSamples / tempSource->getAudioFormatReader()->sampleRate;
                f.name = currentFile.getFileNameWithoutExtension();
                f.path = currentFile.getFullPathName();
                DBG("durée : " << f.duration);
                DBG("nom : " << f.name);
                DBG("chemin : " << f.path);
                filesToImport.add(f);
            }
        }
    }
    table.updateContent();
}
