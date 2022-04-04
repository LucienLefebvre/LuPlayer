/*
  ==============================================================================

    KeyMapper.h
    Created: 1 Feb 2022 1:46:49pm
    Author:  Lucien Lefebvre

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../Settings.h"
//==============================================================================
/*
*/
class KeyMapper : 
    public juce::Component,
    public juce::TableListBoxModel,
    public juce::KeyListener,
    public juce::Timer
{
public:
    KeyMapper(Settings* s);
    ~KeyMapper() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    int getNumRows();

    void paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected);

    void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected);

    void cellClicked(int rowNumber, int columnID, const juce::MouseEvent& e);

    juce::Array<int> getKeyMapping();
    int getKeyMapping(int command);
    void setKeyMapping(juce::Array<int> m);
    bool keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent);
    bool getWantsKeyPress();
    void setCommandManager(juce::ApplicationCommandManager* cm);
    void loadMappingFile();
    void timerCallback();
private:
    //KeyMapping keyMap;
    juce::Array<int> keyMapping;
    juce::Array<int> commandUIDS;
    juce::StringArray keyMapCommands;
    std::unique_ptr<juce::TableListBox> table;

    bool wantsKeyPress = false;
    int commandToMap = 0;
    int numRows = 0;

    juce::uint32 timerStartTime = 0;
    int resetWantsKeyPressTime = 5000;

    Settings* settings;
    juce::ApplicationCommandManager* commandManager;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KeyMapper)
};
