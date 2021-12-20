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
    public juce::Value::Listener
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

    juce::Label timeLabel;

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
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DistantDataBaseBrowser)
};
