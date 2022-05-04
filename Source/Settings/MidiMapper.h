/*
  ==============================================================================

    MidiMapper.h
    Created: 3 Feb 2022 10:58:09pm
    Author:  Lucien Lefebvre

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Settings.h"
//==============================================================================
/*
*/
class MidiMapper  : public juce::Component,
    public juce::TableListBoxModel
{
public:
    enum MidiCommands
    {
        Fader1Level = 200,
        Fader2Level,
        Fader3Level,
        Fader4Level,
        Fader5Level,
        Fader6Level,
        Fader7Level,
        Fader8Level,
        Fader1Trim,
        Fader2Trim,
        Fader3Trim,
        Fader4Trim,
        Fader5Trim,
        Fader6Trim,
        Fader7Trim,
        Fader8Trim,
        Fader1TrimR,
        Fader2TrimR,
        Fader3TrimR,
        Fader4TrimR,
        Fader5TrimR,
        Fader6TrimR,
        Fader7TrimR,
        Fader8TrimR,
        KeyMap1 = 250,
        KeyMap2,
        KeyMap3,
        KeyMap4,
        KeyMap5,
        KeyMap6,
        KeyMap7,
        KeyMap8,
        KeyMap9,        
        KeyMap10,
        KeyMap11,
        KeyMap12,
        KeyMap13,
        KeyMap14,
        KeyMap15,
        KeyMap16,
        KeyMap17,
        KeyMap18,
        KeyMap19,        
        KeyMap20,
        KeyMap21,
        KeyMap22,
        KeyMap23,
        KeyMap24,
        KeyMap25,
        KeyMap26,
        KeyMap27,
        KeyMap28,
        KeyMap29,
        KeyMap30,
        MIDICOMMANDS_NUM_ITEMS
    };

    struct MidiCommandMapping
    {
        juce::CommandID command;
        int cc;
        juce::String commandDescription;
        int faderNumber = -1;
    };
    MidiMapper();
    ~MidiMapper() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    int getNumRows();

    void paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected);

    void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected);

    void cellClicked(int rowNumber, int columnID, const juce::MouseEvent& e);

    void handleMidiMessage(const juce::MidiMessage& message);

    void addMidiCommand(juce::CommandID command, int midiCC, juce::String description, int faderNumber = -1);

    void initializeDefaultMapping();

    int getMidiCCForCommand(juce::CommandID c);

    int getFaderNumberForCommand(juce::CommandID c);

    bool getWantsKeyPress();
    void setWantsKeyPress(bool b);
    void setCommandManager(juce::ApplicationCommandManager* cm);
    void saveMidiMapping();
    void loadMappingFile();
private:
    Settings* settings;
    juce::ApplicationCommandManager* commandManager;

    std::unique_ptr<juce::TableListBox> table;

    bool wantsKeyPress = false;
    int commandToMap = 0;
    int numRows = 0;

    juce::Array<int> midiCCs;
    juce::Array<MidiCommandMapping> midiMappings;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiMapper)
};
