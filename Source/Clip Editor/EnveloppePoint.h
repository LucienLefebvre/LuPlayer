/*
  ==============================================================================

    EnveloppePoint.h
    Created: 12 Jan 2022 10:00:08pm
    Author:  Lucien

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class EnveloppePoint  : public juce::Component
{
public:
    EnveloppePoint(float xAxis, float yAxis) : xPos(xAxis), yPos(yAxis)
    {
        // In your constructor, you should add any child components, and
        // initialise any special settings that your component needs.
        setSize(size, size);
    }

    ~EnveloppePoint() override
    {
    }

    void paint (juce::Graphics& g) override
    {
        //g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
        g.setColour(juce::Colours::red);
        g.fillEllipse(0, 0, getWidth(), getHeight());
    }

    void resized() override
    {
        // This method is where you should set the bounds of any child
        // components that your component contains..

    }

    void setPosition(float xAxis, float yAxis)
    {
        if (canMoveTime)
        {
            juce::Range<float> pointXRange(0.0, 1.0);
            xPos = pointXRange.clipValue(xAxis);
        }
        juce::Range<float> pointYRange(-1.0, 1.0);
        yPos = pointYRange.clipValue(yAxis);
    }

    void setPosition(juce::Point<float> p)
    {
        if (canMoveTime)
        {
            juce::Range<float> pointXRange(0.0, 1.0);
            xPos = pointXRange.clipValue(p.x);
        }
        juce::Range<float> pointYRange(-1.0, 1.0);
        yPos = pointYRange.clipValue(p.y);
    }

    void setXpos(float x)
    {
        if (canMoveTime)
        {
            juce::Range<float> pointXRange(0.0, 1.0);
            xPos = pointXRange.clipValue(x);
        }
    }

    void setYpos(float y)
    {
        juce::Range<float> pointYRange(-1.0, 1.0);
        yPos = pointYRange.clipValue(y);
    }

    juce::Point<float> getPos()
    {
        juce::Point<float> p;
        p.x = xPos;
        p.y = yPos;
        return p;
    }
    float getXPos() 
    {
        return xPos;
    }
    float getYPos()
    {
        return yPos;
    }

    int getRadius()
    {
        return size / 2;
    }

    void setCanMoveTime(bool b)
    {
        canMoveTime = b;
    }

    bool getCanMoveTime()
    {
        return canMoveTime;
    }


private:
    float xPos = 0.0;
    float yPos = 0.0;
    int size = 10;
    bool canMoveTime = true;
    //juce::ComponentDragger myDragger;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EnveloppePoint)
};

class  EnveloppePointComparator :   public juce::Component
{
public:
    EnveloppePointComparator()
    {
    }

    ~EnveloppePointComparator() override
    {
    }

    int compareElements(EnveloppePoint* first, EnveloppePoint* second)
    {
        if (first->getXPos() > second->getXPos())
            return 1;
        if (first->getXPos() < second->getXPos())
            return -1;
        if (first->getXPos() == second->getXPos())
            return 0;
    }
};