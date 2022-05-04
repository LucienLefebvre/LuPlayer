/*
  ==============================================================================

    EnveloppePoint.h
    Created: 12 Jan 2022 10:00:08pm
    Author:  Lucien Lefebvre

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
        setSize(size, size);
        pointColour = juce::Colours::red;
    }

    ~EnveloppePoint() override
    {
    }

    void paint (juce::Graphics& g) override
    {
        g.setColour(pointColour);
        g.fillEllipse(0, 0, getWidth(), getHeight());
    }

    void resized() override
    {

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
        pointColour = juce::Colours::darkred;
    }

    bool getCanMoveTime()
    {
        return canMoveTime;
    }

    void setPointColour(juce::Colour c)
    {
        pointColour = c;
    }

    void setFixedPoint(bool isFixed)
    {
        setCanMoveTime(!isFixed);
    }
private:
    float xPos = 0.0;
    float yPos = 0.0;
    int size = 10;
    bool canMoveTime = true;
    juce::Colour pointColour;
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