/*
  ==============================================================================

    KeyMappedPlayer.h
    Created: 16 Jan 2022 9:32:26pm
    Author:  DPR

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../Player.h"
//==============================================================================
/*
*/
class KeyMappedPlayer  :    public juce::Component,
                            public juce::ChangeListener,
                            public juce::Slider::Listener,
                            public juce::MultiTimer
{
public:
    KeyMappedPlayer();
    ~KeyMappedPlayer() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void setPlayer(Player* p);

    void setShortcut(juce::String s);

    void isDraggedOver(bool b);

    void loadFile(juce::String path, juce::String name);

    void changeListenerCallback(juce::ChangeBroadcaster* source);

    void updatePlayerInfo();

    void shortcutKeyPressed();

    void startOrStop();

    void mouseDown(const juce::MouseEvent& event);

    void sliderValueChanged(juce::Slider* slider);

    void timerCallback(int timerID);

    void setPlayerColours(juce::Colour c);

    void scrollNameLabel();

    void editButtonClicked();

private:
    //Player dummyPlayer;
    Player* soundPlayer = 0;
    Player::PlayerInfo playerInfos;

    std::unique_ptr<juce::Label> shortcutLabel;
    std::unique_ptr<juce::Label> elapsedTimeLabel;
    std::unique_ptr<juce::Label> nameLabel;
    std::unique_ptr<juce::Slider> volumeSlider;
    std::unique_ptr<juce::TextButton> editButton;
    int editButtonWidth = 0;
    juce::String shortcutKey;

    bool isDragged = false;

    juce::Colour currentColour;

    juce::AudioThumbnail* thumbnail;
    juce::Rectangle<int> thumbnailBounds;
    //DISPLAY POSITIONS
    int shortcutLabelWidth = 20;
    int shortcutLabelHeight = 20;
    int nameLabelHeight = 20;
    int volumeSliderHeight;
    int volumeSliderWidth = 30;
    int thumbnailHeight = 100;
    int elapsedTimeWidth = 100;
    int elapsedTimeHeight = 20;
    int nameLabelScrollX = 0;
    int nameLabelTextTotalWidth = 0;

    int startTickScrollTimer = 0;
    int endTickScrollTimer = 0;
    bool scrollLabel = false;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KeyMappedPlayer)
};
