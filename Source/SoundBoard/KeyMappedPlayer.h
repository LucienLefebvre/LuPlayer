/*
  ==============================================================================

    KeyMappedPlayer.h
    Created: 16 Jan 2022 9:32:26pm
    Author:  Lucien Lefebvre

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Player.h"
#include "../Others/PlayHead.h"
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

    Player* getPlayer();

    void setShortcut(juce::String s);

    void isDraggedOver(bool b);

    void loadFile(juce::String path, juce::String name);

    void changeListenerCallback(juce::ChangeBroadcaster* source);

    void updatePlayerInfo();

    void updateInOutPoints();

    void shortcutKeyPressed(bool commandDown = false);

    void startOrStop();

    void mouseDown(const juce::MouseEvent& event);

    void mouseDrag(const juce::MouseEvent& event);

    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel);

    void setDBText();

    void mouseUp(const juce::MouseEvent& event);

    void sliderValueChanged(juce::Slider* slider);

    void timerCallback(int timerID);

    void setPlayerColours(juce::Colour c);

    void scrollNameLabel();

    void editButtonClicked();

    void setPlayerDefaultColour(juce::Colour c);

    juce::Colour getPlayerDefaultColour();

    juce::KeyPress getShortcut();

    bool isFileLoaded();

    std::unique_ptr<juce::ChangeBroadcaster> playerDraggedBroadcaster = std::make_unique<juce::ChangeBroadcaster>();
    std::unique_ptr<juce::ChangeBroadcaster> colourChangedBroadcaster = std::make_unique<juce::ChangeBroadcaster>();
private:
    //Player dummyPlayer;
    Player* soundPlayer = 0;
    Player::PlayerInfo playerInfos;

    std::unique_ptr<juce::Label> shortcutLabel;
    std::unique_ptr<juce::Label> elapsedTimeLabel;
    std::unique_ptr<juce::Label> nameLabel;
    std::unique_ptr<juce::Slider> volumeSlider;
    std::unique_ptr<juce::TextButton> editButton;
    std::unique_ptr<juce::Label> dBLabel;
    std::unique_ptr<PlayHead> playHead;
    std::unique_ptr<PlayHead> outPlayHead;
    std::unique_ptr<juce::ProgressBar> busyBar;

    int editButtonWidth = 0;
    juce::String shortcutKey;
    juce::KeyPress shortcutKeyPress;
    bool isDragged = false;

    juce::Colour currentColour;
    juce::Colour defaultColour;

    GainThumbnail* thumbnail;
    GainThumbnail* playThumbnail;
    juce::Rectangle<int> thumbnailBounds;
    juce::Rectangle<int> playThumbnailBounds;

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

    //Drag gain
    float gainAtDragStart = 0.0f;
    juce::uint32 gainTimeStartDisplay = 0;
    int gainDisplayTimeMs = 2000;

    juce::Rectangle<int> busyBarBounds;
    double busyBarValue = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KeyMappedPlayer)
};
