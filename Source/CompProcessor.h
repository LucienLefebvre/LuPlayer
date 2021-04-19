/*
  ==============================================================================

    CompProcessor.h
    Created: 19 Apr 2021 11:23:23pm
    Author:  DPR

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class CompProcessor  : public juce::Component,
    public juce::Timer
{
public:
    struct CompParameters
    {
        float threshold = 0.0f;
        float ratio = 1.0f;
        float attack = 20.0f;
        float release = 50.0f;
        float gain = 0.0f;
    };
    CompProcessor()
    {
        juce::Timer::startTimer(50);
        compressor.setThreshold(compParams.threshold);
        compressor.setRatio(compParams.ratio);
        compressor.setAttack(compParams.attack);
        compressor.setRelease(compParams.release);
        gain.setGainDecibels(compParams.gain);
    }

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate)
    {
        actualSampleRate = sampleRate;
        actualSamplesPerBlockExpected = samplesPerBlockExpected;
        juce::dsp::ProcessSpec spec;
        spec.sampleRate = actualSampleRate;
        spec.maximumBlockSize = actualSamplesPerBlockExpected;
        spec.numChannels = 2;
        compressor.prepare(spec);
        gain.prepare(spec);
    }

    void getNextAudioBlock(juce::AudioBuffer<float>* buffer)
    {
        juce::dsp::AudioBlock<float> block(*buffer);

        //measure input RMS
        inputRMS = buffer->getRMSLevel(0, 0, buffer->getNumSamples());
        //process
        compressor.process(juce::dsp::ProcessContextReplacing<float>(block));
        //measure output RMS
        beforeGainRms = buffer->getRMSLevel(0, 0, buffer->getNumSamples());
        //add the reduction ratio to an array
        if (reductionMeasured)
            reductionsGains.clear();
        reductionsGains.add(juce::Decibels::gainToDecibels(inputRMS / beforeGainRms));
        gain.process(juce::dsp::ProcessContextReplacing<float>(block));

    }

    void timerCallback()
    {
        auto gains = reductionsGains;
        reductionDB = 1;
        for (auto i = 0; i < reductionsGains.size(); i++)
            reductionDB = reductionDB * reductionsGains[i];
        DBG(reductionDB);
        reductionMeasured = true;
    }

    float getReductionDB()
    {
        return reductionDB;
    }
    void setThreshold(float threshold)
    {
        compParams.threshold = threshold;
        compressor.setThreshold(compParams.threshold);
    }

    void setRatio(float ratio)
    {
        compParams.ratio = ratio;
        compressor.setRatio(compParams.ratio);
    }

    void setAttack(float attack)
    {
        compParams.attack = attack;
        compressor.setAttack(compParams.attack);
    }

    void setRelease(float release)
    {
        compParams.release = release;
        compressor.setRelease(compParams.release);
    }

    void setGain(float gainToSet)
    {
        compParams.gain = gainToSet;
        gain.setGainDecibels(compParams.gain);
    }


    juce::dsp::Compressor<float>& getCompressor()
    {
        return compressor;
    }

    juce::dsp::Gain<float>& getGain()
    {
        return gain;
    }
    ~CompProcessor() override
    {
    }

    void paint (juce::Graphics& g) override{}
    void resized() override{}

    CompParameters getCompParams()
    {
        return compParams;
    }

private:
    double actualSampleRate;
    double actualSamplesPerBlockExpected;
    juce::dsp::Compressor<float> compressor;
    juce::dsp::Gain<float> gain;
    CompParameters compParams;
    float inputRMS = 0.0f;
    float beforeGainRms = 0.0f;
    juce::Array<float> reductionsGains;
    float reductionDB = 0;
    bool reductionMeasured = false;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CompProcessor)
};
