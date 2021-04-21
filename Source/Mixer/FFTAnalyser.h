/*
  ==============================================================================

    FFTAnalyser.h
    Created: 21 Apr 2021 5:34:43pm
    Author:  DPR

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class FFTAnalyser : public juce::Thread
{
public : 
    FFTAnalyser() : Thread("FFT Analyser")
    {
        averager.clear();
    }

    ~FFTAnalyser()
    {

    }

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate)
    {
        actualSampleRate = sampleRate;
        actualSamplesPerBlockExpected = samplesPerBlockExpected;
        fifo.setTotalSize(sampleRate);
        fifoBuffer.setSize(1, sampleRate);
        //startThread(5);
    }

    void run() override
    {
        while(!threadShouldExit())
        {
            if (fifo.getNumReady() >= fft.getSize())
            {
                fftBuffer.clear();
                int start1, block1, start2, block2;
                fifo.prepareToRead(fft.getSize(), start1, block1, start2, block2);
                if (block1 > 0)
                    fftBuffer.copyFrom(0, 0, fifoBuffer.getReadPointer(0, start1), block1);
                if (block2 > 0)
                    fftBuffer.copyFrom(0, block1, fifoBuffer.getReadPointer(0, start2), block2);
                fifo.finishedRead((block1 + block2) / 2);


                window.multiplyWithWindowingTable(fftBuffer.getWritePointer(0), fft.getSize());
                fft.performFrequencyOnlyForwardTransform(fftBuffer.getWritePointer(0));

                //juce::ScopedLock lockedForWriting(pathCreationLock);
                averager.addFrom(0, 0, averager.getReadPointer(averagerPtr), averager.getNumSamples(), -1.0f);
                averager.copyFrom(averagerPtr, 0, fftBuffer.getReadPointer(0), averager.getNumSamples(), 1.0f / (averager.getNumSamples() * (averager.getNumChannels() - 1)));
                averager.addFrom(0, 0, averager.getReadPointer(averagerPtr), averager.getNumSamples());
                if (++averagerPtr == averager.getNumChannels()) averagerPtr = 1;
                newDataAvailable = true;
            }

            if (fifo.getNumReady() < fft.getSize())
                waitForData.wait(20);

        }
    }

    void addAudioData(juce::AudioBuffer<float>& buffer)
    {
        if (fifo.getFreeSpace() < buffer.getNumSamples())
            return;
        int start1, block1, start2, block2;
        fifo.prepareToWrite(buffer.getNumSamples(), start1, block1, start2, block2);
        fifoBuffer.copyFrom(0, start1, buffer.getReadPointer(0), block1);
        if (block2 > 0)
            fifoBuffer.copyFrom(0, start2, buffer.getReadPointer(0, block1), block2);
        fifo.finishedWrite(block1 + block2);
        waitForData.signal();
    }

    bool checkForNewData()
    {
        auto available = newDataAvailable.load();
        newDataAvailable.store(false);
        return available;
    }

    juce::AudioBuffer<float> getFFTBuffer()
    {
        return fftBuffer;
    }

    int getFFTSize()
    {
        return fft.getSize();
    }
private : 
    juce::CriticalSection pathCreationLock;
    double actualSampleRate;
    int actualSamplesPerBlockExpected;
    juce::dsp::FFT                              fft{14};
    juce::dsp::WindowingFunction<float> window  {fft.getSize(), juce::dsp::WindowingFunction<float>::hann, true};
    juce::AbstractFifo fifo                     { 48000 };
    juce::AudioBuffer<float> fifoBuffer;
    juce::AudioBuffer<float> fftBuffer          { 1, fft.getSize() * 2 };
    juce::AudioBuffer<float> averager           { 5, fft.getSize() / 2 };
    int averagerPtr = 1;
    std::atomic<bool> newDataAvailable;
    juce::WaitableEvent waitForData;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FFTAnalyser)
};