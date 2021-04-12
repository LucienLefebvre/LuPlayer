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
    public juce::Timer
{
public:
    DataBaseImport();
    ~DataBaseImport() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    juce::AudioTransportSource transport;

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
    const juce::String DataBaseImport::startFFmpeg(std::string filePath);
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

    juce::TableListBox table;
    juce::StringArray filesToImport;
    juce::StringArray names;
    juce::StringArray durations;
    juce::StringArray durationsMMSS;

    juce::TextButton importButton;
    juce::TextButton clearButton;
    juce::TextButton startStopButton{ "Play/Stop" };
    juce::ToggleButton autoPlayButton{ "Autoplay" };

    int numRows = 0;

    PlayHead playHead;
    juce::Rectangle<int> thumbnailBounds;
    double actualSampleRate;
    int actualSamplesPerBlockExpected;
    juce::AudioFormatManager formatManager;
    juce::AudioThumbnailCache thumbnailCache;
    juce::AudioThumbnail thumbnail;
    std::unique_ptr<juce::AudioFormatReaderSource> playSource;
    juce::ResamplingAudioSource resampledSource;

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
    std::wstring DataBaseImport::utf8_to_utf16(const std::string& utf8)
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DataBaseImport)
};
