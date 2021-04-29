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
class CompProcessor  :  public juce::Timer
{
public:
    enum Mode
    {
        Mono,
        Stereo
    };
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
        float gain = 0.0f;
        float threshold = 0.0f;
        float release = 200.0f;
        bool bypassed = false;
    };
    struct DeesserParameters
    {
        float threshold = 0.0f;
        float range = 1.0f;
        float frequency = 8000.0f;
        bool bypassed = false;
    };
    CompProcessor(Mode channelMode)
    {
        juce::Timer::startTimer(50);

        mode = channelMode;

        compressor.setThreshold(compParams.threshold);
        compressor.setRatio(compParams.ratio);
        compressor.setAttack(compParams.attack);
        compressor.setRelease(compParams.release);
        gain.setGainDecibels(compParams.gain);

        gate.setThreshold(gateParams.threshold);
        gate.setRatio(gateParams.ratio);
        gate.setAttack(gateParams.attack);
        gate.setRelease(gateParams.release);

        limitGain.setGainDecibels(limitParams.gain);
        limiter.setThreshold(limitParams.threshold);
        limiter.setRelease(limitParams.release);
        limiterMakeUpGain.setGainDecibels(-3.0f);
    }

    ~CompProcessor() override
    {
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
        limiter.prepare(spec);
        limitGain.prepare(spec);
        limiterMakeUpGain.prepare(spec);
    }

    void getNextAudioBlock(juce::AudioBuffer<float>* buffer)
    {
        juce::dsp::AudioBlock<float> block(*buffer);

        //INPUT RMS
        inputRMS.store(buffer->getRMSLevel(0, 0, buffer->getNumSamples()));
        if (mode == Stereo)
            inputRMS.store(juce::jmax(inputRMS.load(), buffer->getRMSLevel(1, 0, buffer->getNumSamples())));

        //GATE
        if (!gateParams.bypassed)
        {
            beforeGateRMS.store(buffer->getRMSLevel(0, 0, buffer->getNumSamples()));
            if (mode == Stereo)
                beforeGateRMS.store(juce::jmax(beforeGateRMS.load(), buffer->getRMSLevel(1, 0, buffer->getNumSamples())));

            gate.process(juce::dsp::ProcessContextReplacing<float>(block));

            afterGateRMS.store(buffer->getRMSLevel(0, 0, buffer->getNumSamples()));
            if (mode == Stereo)
                afterGateRMS.store(juce::jmax(afterGateRMS.load(), buffer->getRMSLevel(1, 0, buffer->getNumSamples())));
        }

        //COMPRESSOR
        if (!compParams.bypassed)
        {
            //measure input RMS
            beforeCompRMS.store(buffer->getRMSLevel(0, 0, buffer->getNumSamples()));
            if (mode == Stereo)
                beforeCompRMS.store(juce::jmax(beforeCompRMS.load(), buffer->getRMSLevel(1, 0, buffer->getNumSamples())));
            //process comp
            compressor.process(juce::dsp::ProcessContextReplacing<float>(block));
            //measure output RMS
            afterCompRMS.store(buffer->getRMSLevel(0, 0, buffer->getNumSamples()));
            if (mode == Stereo)
                afterCompRMS.store(juce::jmax(afterCompRMS.load(), buffer->getRMSLevel(1, 0, buffer->getNumSamples())));
            //makeup gain
            gain.process(juce::dsp::ProcessContextReplacing<float>(block));
        }

        //LIMITER
        if (!limitParams.bypassed)
        {
            //this first gain is to mask the +dB induced by the limiter (I don't know why....)
            limiterMakeUpGain.process(juce::dsp::ProcessContextReplacing<float>(block));
            //limiter input gain
            limitGain.process(juce::dsp::ProcessContextReplacing<float>(block));
            //measure input RMS
            beforeLimitRMS.store(buffer->getRMSLevel(0, 0, buffer->getNumSamples()));
            if (mode == Stereo)
                beforeLimitRMS.store(juce::jmax(beforeLimitRMS.load(), buffer->getRMSLevel(1, 0, buffer->getNumSamples())));
            //process comp
            limiter.process(juce::dsp::ProcessContextReplacing<float>(block));
            //measure output RMS
            afterLimitRMS.store(buffer->getRMSLevel(0, 0, buffer->getNumSamples()));
            if (mode == Stereo)
                afterLimitRMS.store(juce::jmax(afterLimitRMS.load(), buffer->getRMSLevel(1, 0, buffer->getNumSamples())));
        }

        //OUTPUT RMS
        outputRMS.store(buffer->getRMSLevel(0, 0, buffer->getNumSamples()));
        if (mode == Stereo)
            outputRMS.store(juce::jmax(outputRMS.load(), buffer->getRMSLevel(1, 0, buffer->getNumSamples())));

    }

    void timerCallback()
    {
    }

    //********REDUCTION GAINS********
    float getGeneralReductionDB()
    {
        return juce::Decibels::gainToDecibels(inputRMS.load() / outputRMS.load());
    }

    float getCompReductionDB()
    {
        if (!compParams.bypassed)
            return juce::Decibels::gainToDecibels(beforeCompRMS.load() / afterCompRMS.load());
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

    float getLimiterReductionDB()
    {
        if (!limitParams.bypassed)
            return juce::Decibels::gainToDecibels(beforeLimitRMS.load() / afterLimitRMS.load());
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


    //*************LIMITER*************
    void setLimitThreshold(float t)
    {
        limitParams.threshold = t;
        limiter.setThreshold(t);
    }

    void setLimitGain(float g)
    {
        limitParams.gain = g;
        limitGain.setGainDecibels(g);
    }

    void setLimitRelease(float r)
    {
        limitParams.release = r;
        limiter.setRelease(r);
    }

    void setLimitBypass(bool isBypassed)
    {
        limitParams.bypassed = isBypassed;
    }

    LimitParameters getLimitParams()
    {
        return limitParams;
    }

private:
    double actualSampleRate;
    double actualSamplesPerBlockExpected;

    Mode mode;

    //DYNAMICS RMS
    std::atomic<float> inputRMS;
    std::atomic<float> outputRMS;

    //COMP
    juce::dsp::Compressor<float> compressor;
    juce::dsp::Gain<float> gain;
    CompParameters compParams;
    std::atomic<float> beforeCompRMS = 0.0f;
    std::atomic<float> afterCompRMS = 0.0f;
    juce::Array<float> compReductionsGain;
    float compReductionDB = 0.0f;

    //GATE
    juce::dsp::NoiseGate<float> gate;
    GateParameters gateParams;
    std::atomic<float> beforeGateRMS = 0.0f;
    std::atomic<float> afterGateRMS = 0.0f;
    juce::Array<float> gateReductionsGain;
    float gateReductionDB = 0.0f;

    //LIMITER
    juce::dsp::Gain<float> limitGain;
    juce::dsp::Limiter<float> limiter;
    juce::dsp::Gain<float> limiterMakeUpGain;
    LimitParameters limitParams;
    std::atomic<float> beforeLimitRMS = 0.0f;
    std::atomic<float> afterLimitRMS = 0.0f;
    juce::Array<float> limitReductionsGain;
    float limitReductionDB = 0.0f;



    juce::dsp::Compressor<float> deesser;
    DeesserParameters deesserParams;

    

    bool reductionMeasured = false;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CompProcessor)
};
