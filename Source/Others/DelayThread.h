/*
  ==============================================================================

    DelayThread.h
    Created: 11 Jun 2022 11:16:49am
    Author:  Lucien Lefebvre

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

#include "../Settings/Settings.h"
class DelayThread : public juce::Thread
{
public:
    DelayThread(const juce::String& threadName, size_t threadStackSize = 0) : Thread("Delay Thread")
    {

    }

    ~DelayThread() override
    {
        stopThread(1000);
    }

    void run()
    {
        juce::Time::waitForMillisecondCounter(juce::Time::getMillisecondCounter() + Settings::faderDelayTime);
        delayBroadcaster->sendChangeMessage();
    }

    std::unique_ptr<juce::ChangeBroadcaster> delayBroadcaster = std::make_unique<juce::ChangeBroadcaster>();
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DelayThread)
};
