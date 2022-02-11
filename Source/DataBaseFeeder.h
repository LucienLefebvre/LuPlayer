/*
  ==============================================================================

    DataBaseFeeder.h
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
#include "FeedThread.h"
//==============================================================================
/*
*/
class DataBaseFeeder : public juce::Component,
    public juce::FileDragAndDropTarget,
    public juce::TableListBoxModel,
    public juce::ChangeListener,
    public juce::ComponentListener,
    public juce::Timer,
    public juce::Value::Listener
{
public:
    DataBaseFeeder();
    ~DataBaseFeeder() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void DataBaseFeeder::prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    //const juce::String DataBaseFeeder::startFFmpeg(std::string filePath);

    juce::AudioTransportSource transport;
    juce::ResamplingAudioSource resampledSource;

private:
    void DataBaseFeeder::feedDB();
    bool DataBaseFeeder::isInterestedInFileDrag(const juce::StringArray& files);
    void DataBaseFeeder::filesDropped(const juce::StringArray& files, int x, int y);
    void DataBaseFeeder::fileDragMove(const juce::StringArray& files, int x, int y);
    void DataBaseFeeder::fileDragEnter(const juce::StringArray& files, int x, int y);
    void DataBaseFeeder::fileDragExit(const juce::StringArray& files);
    int DataBaseFeeder::getNumRows() override;
    void DataBaseFeeder::paintRowBackground(juce::Graphics& g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected) override;
    void DataBaseFeeder::paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool /*rowIsSelected*/) override;
    juce::String DataBaseFeeder::getText(const int columnNumber, const int rowNumber) const;
    void DataBaseFeeder::setText(const int columnNumber, const int rowNumber, const juce::String& newText);
    int getType(const int rowNumber) const;
    void setType(const int rowNumber, const int newType);
    juce::Component* DataBaseFeeder::refreshComponentForCell(int rowNumber, int columnId, bool /*isRowSelected*/, juce::Component* existingComponentToUpdate) override;
    bool DataBaseFeeder::loadFile(const juce::String& path, bool dispalyThumbnail);
    void DataBaseFeeder::changeListenerCallback(juce::ChangeBroadcaster* source);
    juce::String DataBaseFeeder::secondsToMMSS(int seconds);
    void DataBaseFeeder::clear();
    void DataBaseFeeder::returnKeyPressed(int lastRowSelected);
    void DataBaseFeeder::deleteKeyPressed(int lastRowSelected);
    void DataBaseFeeder::startOrStop();
    void DataBaseFeeder::cellClicked(int rowNumber, int columnID, const juce::MouseEvent&);
    void DataBaseFeeder::mouseDown(const juce::MouseEvent& e);
    void DataBaseFeeder::mouseDrag(const juce::MouseEvent& e);
    void DataBaseFeeder::timerCallback();
    void DataBaseFeeder::sortFiles(int columnToSort);
    void DataBaseFeeder::selectedRowsChanged(int lastRowSelected);
    void DataBaseFeeder::valueChanged(juce::Value& value);
    void DataBaseFeeder::openButtonClicked();
    void DataBaseFeeder::addFiles(const juce::StringArray files);
    double fileSampleRate = 48000;
    juce::TableListBox table;
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
        EditableTextCustomComponent(DataBaseFeeder& td) : owner(td)
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
        DataBaseFeeder& owner;
        int row, columnId;
        juce::Colour textColour;
    };
    class TypeSelectorCustomComponent : public Component
    {
    public:
        TypeSelectorCustomComponent(DataBaseFeeder& td) : owner(td)
        {
            // just put a combo box inside this component
            addAndMakeVisible(comboBox);
            comboBox.addItem("-", 1);
            comboBox.addItem("Speech", 2);
            comboBox.addItem("Music", 3);
            comboBox.addItem("Jingle", 4);
            comboBox.addItem("FX", 5);
            comboBox.addItem("Others", 6);

            comboBox.onChange = [this] { owner.setType(row, comboBox.getSelectedId()); };
            comboBox.setWantsKeyboardFocus(false);
        }

        void resized() override
        {
            comboBox.setBoundsInset(juce::BorderSize<int>(2));
        }

        // Our demo code will call this when we may need to update our contents
        void setRowAndColumn(int newRow, int newColumn)
        {
            row = newRow;
            columnId = newColumn;
            comboBox.setSelectedId(owner.getType(row), juce::dontSendNotification);
        }

    private:
        DataBaseFeeder& owner;
        juce::ComboBox comboBox;
        int row, columnId;
    };

   
    FeedThread feedDbThread;
    juce::Array<FeedThread::FileToImport> filesToImport;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DataBaseFeeder)
};

