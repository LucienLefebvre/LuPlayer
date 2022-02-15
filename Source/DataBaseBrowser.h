/*
  ==============================================================================

    DataBaseBrowser.h
    Created: 24 Mar 2021 7:16:03pm
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
#include "ffmpegConvert.h"
#include "convertObject.h"
#include "batchConvertThread.h"
#include "Settings/KeyMapper.h"
//==============================================================================
/*
*/
class DataBaseBrowser  : public juce::Component,
    public juce::Label::Listener,
    public juce::TableListBoxModel,
    public juce::ComponentListener,
    public juce::ChangeListener,
    public juce::Timer,
    public juce::TableHeaderComponent::Listener,
    public juce::ChangeBroadcaster,
    public juce::Value::Listener
{
public:
    DataBaseBrowser();
    ~DataBaseBrowser() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    int DataBaseBrowser::getNumRows() override;
    void paintRowBackground(juce::Graphics& g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected) override;
    void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool /*rowIsSelected*/) override;
    void DataBaseBrowser::prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    //bool keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent, KeyMapper* keyMapper);
    void play();
    juce::File DataBaseBrowser::getSelectedFile();
    juce::String DataBaseBrowser::getSelectedSoundName();
    juce::AudioTransportSource transport;
    juce::ResamplingAudioSource resampledSource;
    juce::ChangeBroadcaster* fileDraggedFromDataBase;
    juce::ChangeBroadcaster* fileDroppedFromDataBase;
    juce::ChangeBroadcaster* cuePlay;

private:
    void DataBaseBrowser::initialize();
    void DataBaseBrowser::labelTextChanged(juce::Label* labelThatHasChanged);
    void DataBaseBrowser::clearListBox();
    void DataBaseBrowser::cellClicked(int rowNumber, int columnID, const juce::MouseEvent& e);
    void selectedRowsChanged(int row);
    void DataBaseBrowser::sqlQuery(std::string search, int sortColum = 0, int sortDirection = 0);
    void DataBaseBrowser::changeListenerCallback(juce::ChangeBroadcaster* source);
    void DataBaseBrowser::startOrStop();
    void DataBaseBrowser::timerCallback();
    void DataBaseBrowser::mouseDown(const juce::MouseEvent& e) override;
    void DataBaseBrowser::mouseDrag(const juce::MouseEvent& e) override;
    void DataBaseBrowser::mouseUp(const juce::MouseEvent& e) override;
    void DataBaseBrowser::tableSortOrderChanged(juce::TableHeaderComponent*) override;
    void DataBaseBrowser::tableColumnsChanged(juce::TableHeaderComponent* tableHeader) override;
    void DataBaseBrowser::tableColumnsResized(juce::TableHeaderComponent* tableHeader) override;
    void DataBaseBrowser::tableColumnDraggingChanged(juce::TableHeaderComponent* tableHeader, int columnIdNowBeingDragged) override;
    void DataBaseBrowser::valueChanged(juce::Value& value);

    bool showFileColumn = false;

    bool fileLoaded = false;
    bool wantToPlay = false;
    bool wantToLoadFile = false;

    
    bool mouseIsUp = false;
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
    juce::ToggleButton todayButton;

    juce::TextButton batchConvertButton;


    juce::Label timeLabel;
    int timeLabelWidth = 150;
    juce::Label nameLabel;

    juce::File file;
    double fileSampleRate = 48000;
    Ebu128LoudnessMeter loudnessMeter;
    LoudnessBar loudnessBarComponent;

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
    bool DataBaseBrowser::loadFile(const juce::String& path);
    std::wstring DataBaseBrowser::utf8_to_utf16(const std::string& utf8);
    void convertAllSounds();
    void todayButtonClicked();
    bool checkAndConvert(int rowNumber);
    void batchConvertButtonClicked();
    void batchConvert();
    juce::String DataBaseBrowser::secondsToMMSS(int seconds);
    const juce::String DataBaseBrowser::startFFmpeg(std::string filePath);


    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DataBaseBrowser)
};
