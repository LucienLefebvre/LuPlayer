/*
  ==============================================================================

    KeyMapper.h
    Created: 1 Feb 2022 1:46:49pm
    Author:  DPR

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
    public juce::KeyListener
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
    void loadKeyMapping();
private:
    //KeyMapping keyMap;
    juce::Array<int> keyMapping;
    juce::StringArray keyMapCommands
    {
        "Play next sound in playlist (playlist mode only) or start cue",// 0
        "Go to first sound in playlist",                            // 1
        "Go to next sound in playlist",                             // 2
        "Go to previous sound in playlist",                         // 3
        "Cue play at playhead",                                     // 4
        "Cue play from start",                                      // 5
        "Cue play last 5 seconds",                                  // 6
        "Set in mark",                                              // 7
        "Delete in mark",                                           // 8
        "Set out mark",                                             // 9
        "Delete out mark",                                          // 10
        "Launch record",                                            // 11
        "Start timer",                                              // 12
        "Play cart 1 (playlist) or player 1 (8 players mode)",      // 13
        "Play cart 2 (playlist) or player 2 (8 players mode)",      // 14
        "Play cart 3 (playlist) or player 3 (8 players mode)",      // 15
        "Play cart 4 (playlist) or player 4 (8 players mode)",      // 16
        "Play cart 5 (playlist) or player 5 (8 players mode)",      // 17
        "Play cart 6 (playlist) or player 6 (8 players mode)",      // 18
        "Play cart 7 (playlist) or player 7 (8 players mode)",      // 19
        "Play cart 8 (playlist) or player 8 (8 players mode)",      // 20
        "Play cart 9 (playlist)",                                   // 21
        "Play cart 10 (playlist)",                                  // 22
        "Play cart 11 (playlist)",                                  // 23
        "Play cart 12 (playlist)",                                  // 24
        "Save (Ctrl +)",                                            // 25
        "Open (Ctrl +)",                                            // 26
        "Quit (Ctrl +)",                                            // 27
        "Open genral settings (Ctrl +)",                            // 28
        "Open audio & midi settings (Ctrl +)",                      // 29
        "Open keyboard shortcuts editor (Ctrl +)",                  // 30
        "Open midi mapping editor (Ctrl +)"                         // 31
    };
    juce::Array<int> defaultMapping
    {
        juce::KeyPress::spaceKey,                                    // 0
        juce::KeyPress::escapeKey,                                   // 1
        juce::KeyPress::pageDownKey,                                 // 2
        juce::KeyPress::pageUpKey,                                   // 3
        juce::KeyPress::createFromDescription("c").getKeyCode(),     // 4
        juce::KeyPress::createFromDescription("x").getKeyCode(),     // 5
        juce::KeyPress::createFromDescription("v").getKeyCode(),     // 6
        juce::KeyPress::createFromDescription("i").getKeyCode(),     // 7
        juce::KeyPress::createFromDescription("k").getKeyCode(),     // 8
        juce::KeyPress::createFromDescription("o").getKeyCode(),     // 9
        juce::KeyPress::createFromDescription("l").getKeyCode(),     // 10
        juce::KeyPress::createFromDescription("r").getKeyCode(),     // 11
        juce::KeyPress::createFromDescription("t").getKeyCode(),     // 12
        juce::KeyPress::F1Key,                                       // 13
        juce::KeyPress::F2Key,                                       // 14
        juce::KeyPress::F3Key,                                       // 15
        juce::KeyPress::F4Key,                                       // 16
        juce::KeyPress::F5Key,                                       // 17
        juce::KeyPress::F6Key,                                       // 18
        juce::KeyPress::F7Key,                                       // 19
        juce::KeyPress::F8Key,                                       // 20
        juce::KeyPress::F9Key,                                       // 21
        juce::KeyPress::F10Key,                                      // 22
        juce::KeyPress::F11Key,                                      // 23
        juce::KeyPress::F12Key,                                      // 24
        juce::KeyPress::createFromDescription("s").getKeyCode(),     // 25
        juce::KeyPress::createFromDescription("o").getKeyCode(),     // 26
        juce::KeyPress::createFromDescription("q").getKeyCode(),     // 27
        juce::KeyPress::createFromDescription("g").getKeyCode(),     // 28
        juce::KeyPress::createFromDescription("a").getKeyCode(),     // 29
        juce::KeyPress::createFromDescription("k").getKeyCode(),     // 30
        juce::KeyPress::createFromDescription("m").getKeyCode()      // 31
    };
    std::unique_ptr<juce::TableListBox> table;

    bool wantsKeyPress = false;
    int commandToMap = 0;

    Settings* settings;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KeyMapper)
};
