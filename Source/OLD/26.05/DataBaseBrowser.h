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
    public juce::ChangeBroadcaster
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
    juce::File DataBaseBrowser::getSelectedFile();
    juce::String DataBaseBrowser::getSelectedSoundName();
    juce::AudioTransportSource transport;
    juce::ChangeBroadcaster* fileDraggedFromDataBase;
    juce::ChangeBroadcaster* fileDroppedFromDataBase;

private:
    void DataBaseBrowser::initialize();
    void DataBaseBrowser::labelTextChanged(juce::Label* labelThatHasChanged);
    void DataBaseBrowser::clearListBox();
    void DataBaseBrowser::cellClicked(int rowNumber, int columnID, const juce::MouseEvent&);
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

    juce::File file;

    Ebu128LoudnessMeter loudnessMeter;
    LoudnessBar loudnessBarComponent;

    PlayHead playHead;
    juce::Rectangle<int> thumbnailBounds;
    double actualSampleRate;
    int actualSamplesPerBlockExpected;
    juce::AudioFormatManager formatManager;
    juce::AudioThumbnailCache thumbnailCache;
    juce::AudioThumbnail thumbnail;
    std::unique_ptr<juce::AudioFormatReaderSource> playSource;
    juce::ResamplingAudioSource resampledSource;
    bool DataBaseBrowser::loadFile(const juce::String& path);
    std::wstring DataBaseBrowser::utf8_to_utf16(const std::string& utf8);
    juce::String DataBaseBrowser::secondsToMMSS(int seconds);
    const juce::String DataBaseBrowser::startFFmpeg(std::string filePath);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DataBaseBrowser)
};
