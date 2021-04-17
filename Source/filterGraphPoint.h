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
    public juce::ComponentDragger,
    public juce::ChangeBroadcaster
{
public:
    filterGraphPoint(int colourId) : mouseDraggedBroadcaster()
    {

        //addMouseListener(this, false);
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
        default:
            colour = juce::Colours::white;
        }
    }

    ~filterGraphPoint() override
    {
    }

    void paint (juce::Graphics& g) override
    {

        //g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
        g.setColour(colour);
        g.fillEllipse(0, 0, pointSize, pointSize);
    }

    void resized() override
    {
        // This method is where you should set the bounds of any child
        // components that your component contains..

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
        default:
            colour = juce::Colours::white;
        }
        repaint();
    }

    int getPointSize()
    {
        return pointSize;
    }

    juce::ChangeBroadcaster mouseDraggedBroadcaster;

private:
    juce::Colour colour;
    int pointSize = 15;

    void mouseDown(const juce::MouseEvent& e)
    {
        dragger.startDraggingComponent(this, e);
    }

    void mouseDrag(const juce::MouseEvent& e)
    {
        dragger.dragComponent(this, e, nullptr);
        mouseDraggedBroadcaster.sendChangeMessage();
    }

    juce::ComponentDragger dragger;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (filterGraphPoint)
};
