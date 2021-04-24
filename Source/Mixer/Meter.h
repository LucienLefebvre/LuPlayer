#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class Meter  : public juce::Component, public juce::Timer
{
public:
    Meter()
    {
        // In your constructor, you should add any child components, and
        // initialise any special settings that your component needs.
        juce::Timer::startTimer(50);
        channelColour = getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId);
    }

    ~Meter() override
    {
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (channelColour);   // clear the background
        juce::NormalisableRange<float>range(-100.0f, 0.0f, 0.01, 3);

        //DRAW RMS
        float limitedRmsLevel = juce::jlimit<float>(-100.0f, 0.0f, juce::Decibels::gainToDecibels(rmsLevel.load()));
        float rangedRmsLevel = range.convertTo0to1(limitedRmsLevel);
        int rectHeight = rangedRmsLevel* getHeight();
        g.setColour(juce::Colours::green);
        g.fillRect(0, getHeight() - rectHeight, meterWidth, rectHeight);
        
        //DRAW PEAK
        float limitedPeakLevel = juce::jlimit<float>(-100.0f, 0.0f, juce::Decibels::gainToDecibels(maxPeak));
        float rangedPeakLevel = range.convertTo0to1(limitedPeakLevel);
        int linePosition = getHeight() - rangedPeakLevel * getHeight();
        g.setColour(juce::Colours::lightgrey);
        if (linePosition < 0)
        {
            linePosition = 0;
            g.setColour(juce::Colours::red);
        }
        g.drawLine(0, linePosition, meterWidth, linePosition);

        //DRAW REDUCTION GAIN
        float limitedReductionLevel = juce::jlimit<float>(-100.0f, 0.0f, - reductionGain.load());
        float rangedReductionLevel = range.convertTo0to1(limitedReductionLevel);
        int reductionRectHeight = getHeight() - rangedReductionLevel * getHeight();
        g.setColour(juce::Colours::orange);
        if (limitedReductionLevel < 0)
            g.fillRect(meterWidth, 0, meterWidth, reductionRectHeight);

        //DRAW SCALE
        for (auto db : dbScale)
        {
            auto yPos = getHeight() - range.convertTo0to1(db) * getHeight();
            if (linePosition == 0 && db == 0.0f)
                g.setColour(juce::Colours::red);
            else
                g.setColour(juce::Colours::black);
            g.drawLine(0, yPos, meterWidth, yPos);
            if ((db != 0.0f) && (db != -6.0f) && (db != -9.0f))
            {
                g.setColour(juce::Colours::lightgrey);
                g.setOpacity(0.7f);
                g.drawText(juce::String(db), 0, yPos + 2, meterWidth, 10, juce::Justification::centred);
            }
        }
    }

    void measureBlock(juce::AudioBuffer<float>* buffer, int channel)
    {
        rmsLevel.store(buffer->getRMSLevel(channel, 0, buffer->getNumSamples()));
        peakLevel.store(buffer->getMagnitude(channel, 0, buffer->getNumSamples()));
    }

    void resized() override
    {
        meterWidth = getWidth() - reductionGainWidth;

    }

    void setReductionGain(float r)
    {
        reductionGain.store(r);
    }
    void timerCallback()
    {
        auto peak = peakLevel.load();
        if (peak > maxPeak)
        {
            maxPeak = peak;
            ticks++;
        }
        else if ((ticks % 6) == 0) //300ms for 50ms timer
        {
            maxPeak = peak;
            ticks = 0;
        }
        else
            ticks++;
        repaint();
    }

    void setColour(juce::Colour c)
    {
        channelColour = c;
        repaint();
    }
private:
    juce::Colour channelColour;

    float lowLevel = -100.0f;

    std::atomic<float> rmsLevel;
    std::atomic<float> peakLevel;
    std::atomic<float> reductionGain = 0.0f;
    float maxPeak = 0.0f;

    int ticks = 0;

    int meterWidth;
    int reductionGainWidth = 10;

    float dbScale[8] = { -40.0f, -30.0f, -20.0f, -12.0f, -9.0f, -6.0f, -3.0f, 0.0f };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Meter)
};
