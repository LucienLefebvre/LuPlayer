#pragma once

#include <JuceHeader.h>
#include "EnveloppePoint.h"
#include "../PlayHead.h"
#include "../Player.h"
#include "../Settings/KeyMapper.h"
//#include "../Thumbnail/gainThumbnail.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class EnveloppeEditor : public juce::Component,
    public juce::Timer,
    public juce::ChangeListener
{
public:
    //==============================================================================
    EnveloppeEditor();
    ~EnveloppeEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void drawTimeLines(juce::Graphics& g, int multiplier);
    void resized() override;

    juce::Point<int> getPointPosition(EnveloppePoint& point);

    juce::Point<float> getPointValue(juce::Point<int> position);

    float getXValue(int position);

    int timeToX(float time);

    float getYValue(int y);

    int gainToY(float gain);

    void mouseDown(const juce::MouseEvent& e);

    void mouseDrag(const juce::MouseEvent& e);

    void mouseUp(const juce::MouseEvent& e);

    void mouseMove(const juce::MouseEvent& e);

    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel);

    EnveloppePoint* addPoint(juce::Point<float> position, bool createPath = true);

    void deletePoint(EnveloppePoint* point);

    void sortPoints();

    std::atomic<float> getEnveloppeValue(float x, juce::Path& p);

    void createEnveloppePath();

    void createPointsFromPath(juce::Path* p);

    void setEditedPlayer(Player* p);
    void setNullPlayer();
    bool keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent, KeyMapper* keyMapper);
    void timerCallback();
    void createDefaultEnveloppePath();
    void scaleButtonClicked();
    void spaceBarPressed();
    void setOrDeleteStart(bool setOrDelete);

    void setOrDeleteStop(bool setOrDelete);

    void setSoundColour(juce::Colour c);

private:
    void changeListenerCallback(juce::ChangeBroadcaster* source);
    juce::Array<int> createTimeLines();
    juce::String secondsToMMSS(int seconds);
    float getRangeInSeconds(juce::Range<double>& r);
    void setPointsColour(juce::Colour c);
    void fixFirstAndLastPoint();

    Player* editedPlayer = nullptr;

    juce::OwnedArray<EnveloppePoint> myPoints;
    juce::ComponentDragger myDragger;
    juce::ComponentBoundsConstrainer constrainer;
    juce::Component* draggedComponent = nullptr;
    juce::TextButton scaleButton;

    GainThumbnail* thumbnail;
    juce::Rectangle<int> thumbnailBounds;
    juce::Range<double> thumbnailRange{ 0.0, 1.0 };
    double middleRangePoint = 0.5;
    juce::Path enveloppePath;

    int zeroDBY = 0;

    juce::Array<juce::Line<float>> linesArray;

    EnveloppePointComparator comparator;

    PlayHead playHead;
    PlayHead cuePlayHead;
    PlayHead inMark;
    PlayHead outMark;
    juce::Label infoLabel;
    
    juce::Colour soundColour;

    bool drawPointInfo = false;
    int pointInfoToDraw = 0;
    bool mouseIsDragged = false;
    double lastPositionClickedorDragged = 0.5;

    float thumbnailHorizontalZoom = 1.0;
    float thumbnailDrawMiddle = 0.5;
    float thumbnailDrawStart = 0;
    float thumbnailDrawEnd = 0;
    int thumbnailDragStart = 0;

    int scale = 24;
    int scaleButtonWidth = 100;
    int scaleButtonHeight = 25;

    juce::Array<float> dBLines{ -0.75f, -0.5f, -0.25, 0.25f, 0.5f, 0.75f };
    juce::Array<int> timeLines;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnveloppeEditor)
};
