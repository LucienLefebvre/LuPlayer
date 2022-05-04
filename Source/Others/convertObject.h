/*
  ==============================================================================

    convertObject.h
    Created: 22 Dec 2021 6:13:02pm
    Author:  Lucien Lefebvre

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ffmpegConvert.h"
//==============================================================================
/*
*/
class convertObject  : public juce::Component,
    public juce::Timer,
    public juce::ChangeListener
{
public:
    convertObject(juce::String filePath, int lenghtInSeconds, int index);
    ~convertObject() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    juce::String getReturnedFile();

    int getId();

    double getProgression();

    ffmpegConvert* getThread();
    
    juce::ChangeBroadcaster* finishedBroadcaster;

private:
    void changeListenerCallback(juce::ChangeBroadcaster* source);
    void timerCallback();
    juce::TextButton startButton;

    juce::String fileName;
    juce::String progressFilePath;
    juce::File progressFile;

    double convertProgress = 0.;
    juce::ProgressBar progressBar{ convertProgress };

    int fileLenght = 1890;
    int id = 0;

    ffmpegConvert convertThread{ "Thread" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (convertObject)
};
