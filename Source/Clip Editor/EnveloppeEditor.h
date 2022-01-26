#pragma once

#include <JuceHeader.h>
#include "EnveloppePoint.h"
#include "../PlayHead.h"
#include "../Player.h"
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
    void resized() override;

    juce::Point<int> getPointPosition(EnveloppePoint& point);

    juce::Point<float> getPointValue(juce::Point<int> position);

    float getXValue(int position);

    int timeToX(float time);

    float getYValue(int y);

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
    bool keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent);
    void timerCallback();
private:
    void changeListenerCallback(juce::ChangeBroadcaster* source);
    Player* editedPlayer = nullptr;

    juce::OwnedArray<EnveloppePoint> myPoints;
    juce::ComponentDragger myDragger;
    juce::ComponentBoundsConstrainer constrainer;
    juce::Component* draggedComponent = nullptr;

    juce::AudioThumbnail* thumbnail;
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
    
    bool drawPointInfo = false;
    int pointInfoToDraw = 0;
    bool mouseIsDragged = false;
    double lastPositionClickedorDragged = 0.5;

    float thumbnailHorizontalZoom = 1.0;
    float thumbnailDrawMiddle = 0.5;
    float thumbnailDrawStart = 0;
    float thumbnailDrawEnd = 0;
    int thumbnailDragStart = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnveloppeEditor)
};
