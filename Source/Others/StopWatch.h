/*
  ==============================================================================

    StopWatch.h
    Created: 31 Jan 2022 11:39:35am
    Author:  DPR

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class StopWatch  : public juce::Component,
    public juce::Timer
{
public:
    enum StopWatchState
    {
        Initialized,
        Running,
        Stopped
    };
    StopWatch()
    {
        timeLabel.reset(new juce::Label());
        addAndMakeVisible(timeLabel.get());
        timeLabel->setMouseClickGrabsKeyboardFocus(false);
        timeLabel->setText("0:00", juce::dontSendNotification);

        startStopButton.reset(new juce::TextButton());
        addAndMakeVisible(startStopButton.get());
        startStopButton->setButtonText("Start");
        startStopButton->onClick = [this] {startStopButtonClicked(); };
        startStopButton->setMouseClickGrabsKeyboardFocus(false);
    }

    ~StopWatch() override
    {
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
    }

    void resized() override
    {
        timeLabel->setBounds(0, 0, getWidth() - buttonWidth, getHeight());
        timeLabel->setFont(juce::Font(getHeight()));
        startStopButton->setBounds(timeLabel->getRight(), 0, buttonWidth, getHeight());
    }

    void startStopButtonClicked()
    {
        switch (state)
        {
        case Initialized :
            state = Running;
            juce::Timer::startTimer(250);
            startedTime = juce::Time::getMillisecondCounter();
            startStopButton->setButtonText("Stop");
            updateTimeLabel();
            break;
        case Running :
            state = Initialized;
            juce::Timer::stopTimer();
            elapsedTime = 0;
            startStopButton->setButtonText("Start");
            break;
        case Stopped :
            state = Initialized;
            startStopButton->setButtonText("Start");
            elapsedTime = 0;
            updateTimeLabel();
        }
    }

    void timerCallback()
    {
        if (state == Running)
        {
            elapsedTime = (juce::Time::getMillisecondCounter() - startedTime) / 1000;
            updateTimeLabel();
        }
    }

    void updateTimeLabel()
    {
        timeLabel->setText(secondsToMMSS(elapsedTime), juce::NotificationType::dontSendNotification);
    }

    juce::String secondsToMMSS(int seconds)
    {
        int timeSeconds = seconds % 60;
        int timeMinuts = trunc(seconds / 60);
        juce::String timeString;
        if (timeSeconds < 10)
            timeString = juce::String(timeMinuts) + ":0" + juce::String(timeSeconds);
        else
            timeString = juce::String(timeMinuts) + ":" + juce::String(timeSeconds);
        return timeString;
    }

private:
    std::unique_ptr<juce::Label> timeLabel;
    std::unique_ptr<juce::TextButton> startStopButton;

    int elapsedTime = 0;
    int startedTime = 0;
    int buttonWidth = 50;
    StopWatchState state = Initialized;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StopWatch)
};
