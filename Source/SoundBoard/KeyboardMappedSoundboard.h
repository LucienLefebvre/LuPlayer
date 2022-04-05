/*
  ==============================================================================

    KeyboardMappedSoundboard.h
    Created: 16 Jan 2022 9:21:14pm
    Author:  Lucien Lefebvre

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "KeyMappedPlayer.h"
#include "../Settings.h"
//==============================================================================
/*
*/
class KeyboardMappedSoundboard : public juce::Component,
                                 public juce::KeyListener,
                                 public juce::ChangeListener
{
public:
    KeyboardMappedSoundboard(Settings* s);
    ~KeyboardMappedSoundboard() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void fileDragMove(const juce::StringArray& files, int x, int y);
    void setDroppedFile(juce::Point<int> p, juce::String path, juce::String name);
    KeyMappedPlayer* KeyboardMappedSoundboard::getPlayer(int i);
    void KeyboardMappedSoundboard::addPlayer(Player* p);
    void setShortcutKeys();
    void setPlayer(Player* p, int i);
    void fileDragExit();
    void mouseDrag(const juce::MouseEvent& event);
    bool keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent);

    void changeListenerCallback(juce::ChangeBroadcaster* source);

    juce::MixerAudioSource mixer;
private:
    juce::OwnedArray<KeyMappedPlayer> mappedPlayers;
    juce::StringArray qwertyShortcutArray{  "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P",
                                            "A", "S", "D", "F", "G", "H", "J", "K", "L", ";",
                                            "\\", "Z", "X", "C", "V", "B", "N", "M", ",", "."};
    juce::StringArray azertyShortcutArray{  "A", "Z", "E", "R", "T", "Y", "U", "I", "O", "P",
                                            "Q", "S", "D", "F", "G", "H", "J", "K", "L", "M",
                                            "<", "W", "X", "C", "V", "B", "N", ",", ";", ":" };
    int shortcutCodeArray[30]{ 1, 2 };
    int spaceBetweenPlayers = 4;
    int playerWidth = 50;
    int playerHeight = 50;

    Settings* settings;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KeyboardMappedSoundboard)
};
