/*
  ==============================================================================

    filterGraphPoint.h
    Created: 17 Apr 2021 3:14:38pm
    Author:  DPR

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
//ajouter double clique pour reset
//ajouter control pour régler Q
//==============================================================================
/*
*/
class filterGraphPoint  : public juce::Component,
    //public juce::MouseListener,
    //public juce::ComponentDragger,
    public juce::ChangeBroadcaster
{
public:
    filterGraphPoint(int colourId) : mouseDraggedBroadcaster(), mouseCtrlDraggedBroadcaster(), mouseClickResetBroadcaster()
    {
        setColour(colourId);
    }

    ~filterGraphPoint() override
    {
    }

    void paint (juce::Graphics& g) override
    {
        g.setColour(colour);
        g.fillEllipse(0, 0, pointSize, pointSize);
    }

    void resized() override
    {
    }

    void setColour(int colourId)
    {
        switch (colourId)
        {
        case 0:
            colour = juce::Colours::red;
            break;
        case 1:
            colour = juce::Colours::green;
            break;
        case 2:
            colour = juce::Colours::blue;
            break;
        case 3:
            colour = juce::Colours::yellow;
            break;
        case 4:
            colour = juce::Colours::grey;
            break;
        default:
            colour = juce::Colours::white;
        }
        repaint();
    }
    juce::Colour getColour()
    {
        return colour;
    }

    int getPointSize()
    {
        return pointSize;
    }

    int getDragYDistance()
    {
        return ctrlDragDistance;
    }

    void setPlotBounds(juce::Rectangle<int>& plotBounds)
    {
        filterPlotBounds = plotBounds;
    }
    juce::ChangeBroadcaster mouseDraggedBroadcaster;
    juce::ChangeBroadcaster mouseCtrlDraggedBroadcaster;
    juce::ChangeBroadcaster mouseClickResetBroadcaster;

private:
    juce::Colour colour;
    int pointSize = 12;

    void mouseDown(const juce::MouseEvent& e)
    {
        dragger.startDraggingComponent(this, e);
        if (e.getNumberOfClicks() == 2)
        {
            mouseClickResetBroadcaster.sendChangeMessage();
        }
    }

    void mouseDrag(const juce::MouseEvent& e)
    {
        if (!e.mods.isCtrlDown() && mouseCtrlDragged == false)
        {
            /*dragger.dragComponent(this, e, nullptr);
            mouseDraggedBroadcaster.sendChangeMessage();*/
        }
        else if(e.mods.isCtrlDown())
        {
            /*ctrlDragDistance = e.getDistanceFromDragStartY();
            mouseCtrlDraggedBroadcaster.sendChangeMessage();
            mouseCtrlDragged = true;*/
        }
    }

    void mouseUp(const juce::MouseEvent& e)
    {
        ctrlDragDistance = 0;
        mouseCtrlDragged = false;
    }

    juce::Rectangle<int> filterPlotBounds;
    juce::ComponentDragger dragger;
    bool mouseCtrlDragged = false;
    int ctrlDragDistance = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (filterGraphPoint)
};
