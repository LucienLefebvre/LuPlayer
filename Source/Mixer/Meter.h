#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class Meter  : public juce::Component, public juce::Timer
{
public:
    enum Mode
    {
        Mono,
        Mono_ReductionGain,
        Stereo,
        Stereo_ReductionGain,
        ReductionGain
    };

    struct MeterData
    {
        float rmsL;
        float rmsR;
        float peakL;
        float peakR;
    };
    Meter(Mode m) : meterMode(m)
    {
        // In your constructor, you should add any child components, and
        // initialise any special settings that your component needs.
        //juce::Timer::startTimer(timerRateMs);
        juce::Timer::startTimerHz(timerRateHz);
        channelColour = getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId);
        dataAvailable.store(false);
    }

    ~Meter() override
    {
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (channelColour);   // clear the background
        juce::NormalisableRange<float>range(-100.0f, 0.0f, 0.01, meterSkewFactor);

        int linePosition;
        //CHANNEL 0
        if (meterMode != ReductionGain)
        {
            {
                //DRAW RMS

                float limitedRmsLevel = juce::jlimit<float>(-100.0f, 0.0f, juce::Decibels::gainToDecibels(rmsL));
                float rangedRmsLevel = range.convertTo0to1(limitedRmsLevel);
                int rectHeight = rangedRmsLevel * getHeight();
                g.setColour(meterColour);
                if (rectHeight > 1)
                    g.fillRoundedRectangle(0, getHeight() - rectHeight, meterWidth - 2, rectHeight, rectangleRoundSize);
                if (limitedRmsLevel > -9.0f)//ORANGE if >-3db
                {
                    int orangeRectYStart = range.convertTo0to1(-9.0f) * getHeight();
                    int orangeRextHeight = rectHeight - orangeRectYStart;
                    g.setColour(juce::Colours::orange);
                    g.fillRect(0, getHeight() - orangeRectYStart - orangeRextHeight - 1, meterWidth - 2, orangeRextHeight);
                }


                //DRAW PEAK L
                float limitedPeakLevel = juce::jlimit<float>(-100.0f, 0.0f, juce::Decibels::gainToDecibels(maxPeakL));
                float rangedPeakLevel = range.convertTo0to1(limitedPeakLevel);
                linePosition = getHeight() - rangedPeakLevel * getHeight();
                juce::jlimit(0, getHeight(), linePosition);
                g.setColour(meterColour);
                if (limitedPeakLevel > -9.0f)
                    g.setColour(peakColour);
                if (linePosition == 0)
                    g.setColour(juce::Colours::red);
                if (linePosition != getHeight())
                {
                g.drawLine(0, linePosition, meterWidth, linePosition);
                g.drawLine(0, linePosition + 1, meterWidth, linePosition + 1);
                }

                //FILL BETWEEN RMS AND PEAK
                /*float limitedInstantPeakLevel = juce::jlimit<float>(-100.0f, 0.0f, juce::Decibels::gainToDecibels(peakLevelL.load()));
                float rangedInstantPeakLevel = range.convertTo0to1(limitedInstantPeakLevel);
                int lineInstantPosition = getHeight() - rangedInstantPeakLevel * getHeight();
                juce::jlimit(0, getHeight(), lineInstantPosition);
                auto fillRectYStart = lineInstantPosition + 2;
                auto fillRectHeight = getHeight() - rectHeight - fillRectYStart;
                if (fillRectHeight > 0 && fillRectYStart > 0)
                {
                    g.setColour(juce::Colours::forestgreen);
                    g.setOpacity(0.5f);
                    g.fillRect(0, fillRectYStart, meterWidth - 2, fillRectHeight);
                }*/
            }

            //CHANNEL 1
            if (meterMode == Stereo || meterMode == Stereo_ReductionGain)
            {

                //DRAW RMS
                float limitedRmsLevel = juce::jlimit<float>(-100.0f, 0.0f, juce::Decibels::gainToDecibels(rmsR));
                float rangedRmsLevel = range.convertTo0to1(limitedRmsLevel);
                int rectHeight = rangedRmsLevel * getHeight();
                g.setColour(meterColour);
                if (rectHeight > 1)
                    g.fillRoundedRectangle(meterWidth + 1, getHeight() - rectHeight, meterWidth - 1, rectHeight, rectangleRoundSize);
                if (limitedRmsLevel > -9.0f)//ORANGE if >-3db
                {
                    int orangeRectYStart = range.convertTo0to1(-9.0f) * getHeight();
                    int orangeRextHeight = rectHeight - orangeRectYStart;
                    g.setColour(juce::Colours::blue);
                    g.fillRect(meterWidth + 1, getHeight() - orangeRectYStart - orangeRextHeight - 1, meterWidth - 1, 2);
                }
                //DRAW PEAK HOLD
                float limitedPeakLevel = juce::jlimit<float>(-100.0f, 0.0f, juce::Decibels::gainToDecibels(maxPeakR));
                float rangedPeakLevel = range.convertTo0to1(limitedPeakLevel);
                linePosition = getHeight() - rangedPeakLevel * getHeight();
                juce::jlimit(0, getHeight(), linePosition);
                g.setColour(meterColour);
                if (limitedPeakLevel > -9.0f)
                    g.setColour(peakColour);
                if (linePosition == 0)
                    g.setColour(juce::Colours::red);
                if (linePosition != getHeight())
                {
                    g.drawLine(meterWidth + 2, linePosition, 2 * meterWidth, linePosition);
                    g.drawLine(meterWidth + 2, linePosition + 1, 2 * meterWidth, linePosition + 1);
                }
                //FILL BETWEEN RMS AND PEAK
                /*float limitedInstantPeakLevel = juce::jlimit<float>(-100.0f, 0.0f, juce::Decibels::gainToDecibels(peakLevelR.load()));
                float rangedInstantPeakLevel = range.convertTo0to1(limitedInstantPeakLevel);
                int lineInstantPosition = getHeight() - rangedInstantPeakLevel * getHeight();
                juce::jlimit(0, getHeight(), lineInstantPosition);
                auto fillRectYStart = lineInstantPosition + 2;
                auto fillRectHeight = getHeight() - rectHeight - fillRectYStart;
                if (fillRectHeight > 0 && fillRectYStart > 0)
                {
                    g.setColour(meterColour);
                    g.setOpacity(0.5f);
                    g.fillRect(meterWidth + 1, fillRectYStart, meterWidth - 1, fillRectHeight);
                }*/
            }
        }
        //DRAW REDUCTION GAIN
        float limitedReductionLevel = juce::jlimit<float>(-100.0f, 0.0f, - reductionGain.load());
        float rangedReductionLevel = range.convertTo0to1(limitedReductionLevel);
        int reductionRectHeight = getHeight() - rangedReductionLevel * getHeight();
        g.setColour(juce::Colours::orangered);
        if (limitedReductionLevel < 0)
            g.fillRect(reductionGainXStart, 0, reductionGainWidth, reductionRectHeight);

        //DRAW SCALE
        if (drawScale)
        {
            for (auto db : dbScale)
            {
                auto yPos = getHeight() - range.convertTo0to1(db) * getHeight();
                if (linePosition == 0 && db == 0.0f)
                    g.setColour(juce::Colours::red);
                else if (db == -9.0f)
                    g.setColour(juce::Colours::orange);
                else if (db == 0.0f)
                    g.setColour(juce::Colours::red);
                else if (meterMode != ReductionGain)
                    g.setColour(juce::Colours::black);
                else
                {
                    g.setColour(juce::Colours::darkgrey);
                    g.setOpacity(0.8f);
                }
                auto lineEnd = (reductionGainXStart == 0) ? getWidth() : reductionGainXStart;
                g.drawLine(0, yPos, lineEnd, yPos);
                g.setColour(juce::Colours::lightgrey);
                g.setOpacity(0.7f);
                if (drawScaleNumbers)
                    g.drawText(juce::String(db), 0, yPos + 2, getWidth() - reductionGainWidth, 10, juce::Justification::centred);
            }
        }

        if (drawExteriorLine)
        {
            g.setColour(juce::Colours::black);
            g.drawLine(0, 0, 0, getHeight());
            g.drawLine(getWidth(), 0, getWidth(), getHeight());
        }
    }

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate)
    {
        actualSampleRate = sampleRate;
        actualSamplesPerBlockExpected = samplesPerBlockExpected;

        //vector to sum severals rms values measuring each block
        numberOfBlocksPerTicks = (sampleRate * ((float)timerRateMs / 1000.0f)) / (float)samplesPerBlockExpected;
        rmsBufferLevelsL.assign(numberOfBlocksPerTicks, 0.0f);
        rmsBufferLevelsR.assign(numberOfBlocksPerTicks, 0.0f);

        //vector to sum above value to get longer integration time
        numberOfTimerCyclePerIntegrationTick = rmsIntegrationTime / timerRateMs;
        rmsLevelsL.assign(numberOfTimerCyclePerIntegrationTick, 0.0f);
        rmsLevelsR.assign(numberOfTimerCyclePerIntegrationTick, 0.0f);
    }

    void measureBlock(juce::AudioBuffer<float>* buffer)
    {

        if (rmsBufferTick < numberOfBlocksPerTicks)
        {
            rmsBufferLevelsL[rmsBufferTick] = pow(buffer->getRMSLevel(0, 0, buffer->getNumSamples()), 2);

            if ((meterMode == Stereo || meterMode == Stereo_ReductionGain)
                && buffer->getNumChannels() > 1)
                rmsBufferLevelsR[rmsBufferTick] = pow(buffer->getRMSLevel(1, 0, buffer->getNumSamples()), 2);

            rmsBufferTick++;
        }
        else
        {
            rmsAverageL.store(sqrt(std::accumulate(rmsBufferLevelsL.begin(), rmsBufferLevelsL.end(), 0.0f) / (float)rmsBufferLevelsL.size()));

            if ((meterMode == Stereo || meterMode == Stereo_ReductionGain)
                && buffer->getNumChannels() > 1)
            rmsAverageR.store(sqrt(std::accumulate(rmsBufferLevelsR.begin(), rmsBufferLevelsR.end(), 0.0f) / (float)rmsBufferLevelsR.size()));

            rmsBufferTick = 0;
        }

        peakLevelL.store(buffer->getMagnitude(0, 0, buffer->getNumSamples()));
        if ((meterMode == Stereo || meterMode == Stereo_ReductionGain)
            && buffer->getNumChannels() > 1)
        {
            peakLevelR.store(buffer->getMagnitude(1, 0, buffer->getNumSamples()));
        }
        
    }

    MeterData getMeterData()
    {
        MeterData data;
        data.rmsL = rmsL;
        data.rmsR = rmsR;
        data.peakL = maxPeakL;
        data.peakR = maxPeakR;
        return data;
    }

    void setMeterData(MeterData meterData)
    {
        rmsAverageL.store(meterData.rmsL);
        rmsAverageR.store(meterData.rmsR);
        peakLevelL.store(meterData.peakL);
        peakLevelR.store(meterData.peakR);
    }

    void setRMSMeterData(float l, float r)
    {
        rmsAverageL.store(l);
        rmsAverageR.store(r);
    }

    void setPeakMeterDate(float l, float r)
    {
        peakLevelL.store(l);
        peakLevelR.store(r);
    }

    void resized() override
    {
        updateBarsSize();
    }

    void updateBarsSize()
    {
        switch (meterMode)
        {
        case Mono:
            meterWidth = getWidth();
            reductionGainWidth = 0;
            reductionGainXStart = 0;
            break;
        case Mono_ReductionGain:
            meterWidth = getWidth() - reductionGainWidth;
            reductionGainXStart = meterWidth;
            break;
        case Stereo:
            meterWidth = getWidth() / 2;
            reductionGainXStart = 0;
            reductionGainWidth = 0;
            break;
        case Stereo_ReductionGain:
            meterWidth = (getWidth() - reductionGainWidth) / 2;
            reductionGainXStart = meterWidth * 2;
            break;
        case ReductionGain:
            meterWidth = 0;
            reductionGainXStart = 0;
        }
    }

    void setReductionGain(float r)
    {
        reductionGain.store(r);
    }

    float getReductionGain()
    {
        return reductionGain.load();
    }

    float getRMSLevel(int channel)
    {

    }
    void timerCallback()
    {
        //RMS
        /*rmsL = rmsAverageL.load();
        rmsR = rmsAverageR.load();*/
        if (rmsIntegrationTick < numberOfTimerCyclePerIntegrationTick)
        {
            rmsLevelsL[rmsIntegrationTick] = pow(rmsAverageL.load(), 2);

            if (meterMode == Stereo || meterMode == Stereo_ReductionGain)
                rmsLevelsR[rmsIntegrationTick] = pow(rmsAverageR.load(), 2);
            rmsIntegrationTick++;
        }
        else if (rmsIntegrationTick == numberOfTimerCyclePerIntegrationTick)
        {
            //rmsLevelsL[rmsIntegrationTick] = pow(rmsAverageL.load(), 2);
            rmsIntegrationTick = 0;
        }
        rmsL = sqrt(std::accumulate(rmsLevelsL.begin(), rmsLevelsL.end(), 0.0f) / (float)rmsLevelsL.size());
        rmsR = sqrt(std::accumulate(rmsLevelsR.begin(), rmsLevelsR.end(), 0.0f) / (float)rmsLevelsR.size());
        
        
        //********PEAK HOLDING********
        //CHANNEL 0
        auto peakL = peakLevelL.load();
        if (peakL > maxPeakL)
        {
            maxPeakL = peakL;
            ticksL++;
        }
        else if ((ticksL % 10) == 0) //1s for 50ms timer
        {
            maxPeakL = peakL;
            ticksL = 0;
        }
        else
            ticksL++;
        //CHANNEL 1
        auto peakR = peakLevelR.load();
        if (peakR > maxPeakR)
        {
            maxPeakR = peakR;
            ticksR++;
        }
        else if ((ticksR % 10) == 0) //1s for 50ms timer
        {
            maxPeakR = peakR;
            ticksR = 0;
        }
        else
            ticksR++;
        repaint();
    }

    void setColour(juce::Colour c)
    {
        channelColour = c;
        repaint();
    }

    void setMeterMode(Meter::Mode mode)
    {
        meterMode = mode;
        updateBarsSize();
    }

    void setReductionGainWidth(int w)
    {
        reductionGainWidth = w;
    }

    void setSkewFactor(float s)
    {
        meterSkewFactor = s;
    }

    void shouldDrawScale(bool draw)
    {
        drawScale = draw;
    }

    void shouldDrawScaleNumbers(bool draw)
    {
        drawScaleNumbers = draw;
    }

    void shouldDrawExteriorLines(bool draw)
    {
        drawExteriorLine = draw;
    }

    void setMeterColour(juce::Colour c)
    {
        meterColour = c;
    }

    void setPeakColour(juce::Colour c)
    {
        peakColour = c;
    }
    void setRectangleRoundSize(int s)
    {
        rectangleRoundSize = s;
    }


    Meter::Mode meterMode;
private:
    double actualSampleRate;
    int actualSamplesPerBlockExpected;

    float meterSkewFactor = 3.0f;

    int timerRateMs = 50;
    int timerRateHz = 60;
    int rmsIntegrationTime = 50;
    
    int numberOfBlocksPerTicks;
    int rmsBufferTick = 0;

    int numberOfTimerCyclePerIntegrationTick;
    int rmsIntegrationTick = 0;

    std::atomic<bool> dataAvailable = false;
    MeterData data;


    float lowLevel = -100.0f;

    bool drawScale = true;
    bool drawScaleNumbers = true;
    bool drawExteriorLine = false;

    float rmsL;
    float rmsR;
    std::vector<double> rmsBufferLevelsL;
    std::vector<double> rmsBufferLevelsR;
    std::vector<double> rmsLevelsL;
    std::vector<double> rmsLevelsR;

    std::atomic<float> rmsAverageL;
    std::atomic<float> rmsAverageR;

    std::atomic<float> rmsLevelL;
    std::atomic<float> rmsLevelR;
    std::atomic<float> peakLevelL;
    std::atomic<float> peakLevelR;
    std::atomic<float> reductionGain = 0.0f;
    float maxPeakL = 0.0f;
    float maxPeakR = 0.0f;

    int ticksL = 0;
    int ticksR = 0;

    int meterWidth;
    int reductionGainWidth = 10;
    int reductionGainXStart;
    int rectangleRoundSize = 5;

    juce::Colour meterColour = juce::Colours::green;
    juce::Colour backgroundColour;
    juce::Colour channelColour;
    juce::Colour peakColour = juce::Colours::orange;

    float dbScale[8] = { -40.0f, -30.0f, -20.0f, -12.0f, -9.0f, -6.0f, -3.0f, 0.0f };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Meter)
};
