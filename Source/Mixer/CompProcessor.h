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
        float threshold = -0.0f;
        float ratio = 1.0f;
        float attack = 20.0f;
        float release = 200.0f;
        float gain = 0.0f;
        bool bypassed = false;
    };
    struct GateParameters
    {
        float threshold = -100.0f;
        float ratio = 1.0f;
        float attack = 100.0f;
        float release = 1000.0f;
        bool bypassed = false;
    };
    struct LimitParameters
    {
        float threshold = 0.0f;
        float release = 50.0f;
        bool bypassed = false;
    };
    struct DeesserParameters
    {
        float threshold = 0.0f;
        float range = 1.0f;
        float frequency = 8000.0f;
        bool bypassed = false;
    };
    CompProcessor()
    {
        juce::Timer::startTimer(50);

        compressor.setThreshold(compParams.threshold);
        compressor.setRatio(compParams.ratio);
        compressor.setAttack(compParams.attack);
        compressor.setRelease(compParams.release);
        gain.setGainDecibels(compParams.gain);

        gate.setThreshold(gateParams.threshold);
        gate.setRatio(gateParams.ratio);
        gate.setAttack(gateParams.attack);
        gate.setRelease(gateParams.release);
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
        gate.prepare(spec);
    }

    void getNextAudioBlock(juce::AudioBuffer<float>* buffer)
    {
        juce::dsp::AudioBlock<float> block(*buffer);

        //INPUT RMS
        inputRMS.store(buffer->getRMSLevel(0, 0, buffer->getNumSamples()));

        //GATE
        if (!gateParams.bypassed)
        {
            beforeGateRMS.store(buffer->getRMSLevel(0, 0, buffer->getNumSamples()));

            gate.process(juce::dsp::ProcessContextReplacing<float>(block));

            afterGateRMS.store(buffer->getRMSLevel(0, 0, buffer->getNumSamples()));

            /*if (reductionMeasured)
                compReductionsGain.clear();
            compReductionsGain.add(juce::Decibels::gainToDecibels(beforeCompRMS / afterCompRMS));*/
        }

        //COMPRESSOR
        if (!compParams.bypassed)
        {
            //measure input RMS
            beforeCompRMS = buffer->getRMSLevel(0, 0, buffer->getNumSamples());
            //process comp
            compressor.process(juce::dsp::ProcessContextReplacing<float>(block));
            //measure output RMS
            afterCompRMS = buffer->getRMSLevel(0, 0, buffer->getNumSamples());
            //makeup gain
            gain.process(juce::dsp::ProcessContextReplacing<float>(block));
            //add the reduction ratios to an array
            if (reductionMeasured)
                compReductionsGain.clear();
            compReductionsGain.add(juce::Decibels::gainToDecibels(beforeCompRMS / afterCompRMS));
        }

        //OUTPUT RMS
        outputRMS.store(buffer->getRMSLevel(0, 0, buffer->getNumSamples()));


    }

    void timerCallback()
    {
        //COMP
        auto gains = compReductionsGain;
        compReductionDB = 1;
        for (auto i = 0; i < compReductionsGain.size(); i++)
            compReductionDB = compReductionDB * compReductionsGain[i];

        //GATE
        reductionMeasured = true;
    }

    float getGeneralReductionDB()
    {
        return juce::Decibels::gainToDecibels(inputRMS.load() / outputRMS.load());
    }

    float getCompReductionDB()
    {
        if (!compParams.bypassed)
            return compReductionDB;
        else
            return 0;
    }

    float getGateReductionDB()
    {

        if (!gateParams.bypassed)
            return juce::Decibels::gainToDecibels(beforeGateRMS.load() / afterGateRMS.load());
        else
            return 0;
    }

    //*************COMPRESSOR*************
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

    void setBypass(bool isBypassed)
    {
        compParams.bypassed = isBypassed;
    }

    CompParameters getCompParams()
    {
        return compParams;
    }

    //*************GATE*************
    void setGateThreshold(float t)
    {
        gateParams.threshold = t;
        gate.setThreshold(t);
    }

    void setGateRatio(float r)
    {
        gateParams.ratio = r;
        gate.setRatio(r);
    }

    void setGateAttack(float a)
    {
        gateParams.attack = a;
        gate.setAttack(a);
    }

    void setGateRelease(float r)
    {
        gateParams.release = r;
        gate.setRelease(r);
    }

    void setGateBypass(bool isBypassed)
    {
        gateParams.bypassed = isBypassed;
    }

    GateParameters getGateParams()
    {
        return gateParams;
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



private:
    double actualSampleRate;
    double actualSamplesPerBlockExpected;

    //DYNAMICS RMS
    std::atomic<float> inputRMS;
    std::atomic<float> outputRMS;

    //COMP
    juce::dsp::Compressor<float> compressor;
    juce::dsp::Gain<float> gain;
    CompParameters compParams;
    float beforeCompRMS = 0.0f;
    float afterCompRMS = 0.0f;
    juce::Array<float> compReductionsGain;
    float compReductionDB = 0.0f;

    //GATE
    juce::dsp::NoiseGate<float> gate;
    GateParameters gateParams;
    std::atomic<float> beforeGateRMS = 0.0f;
    std::atomic<float> afterGateRMS = 0.0f;
    juce::Array<float> gateReductionsGain;
    float gateReductionDB = 0.0f;

    juce::dsp::Limiter<float> limiter;
    LimitParameters limitParams;

    juce::dsp::Compressor<float> deesser;
    DeesserParameters deesserParams;

    

    bool reductionMeasured = false;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CompProcessor)
};
