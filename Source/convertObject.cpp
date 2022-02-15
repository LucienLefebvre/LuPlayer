/*
  ==============================================================================

    convertObject.cpp
    Created: 22 Dec 2021 6:13:02pm
    Author:  lucie

  ==============================================================================
*/

#include <JuceHeader.h>
#include "convertObject.h"
#include <stdio.h>
#include <string>
#include "Windows.h"
#include "helpers.h"
#include <regex>
//==============================================================================
convertObject::convertObject(juce::String filePath, int lenghtInSeconds, int index) : id(index)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    addAndMakeVisible(&progressBar);
    progressBar.setBounds(0, 0, 200, 25);
    progressBar.setPercentageDisplay(true);
    progressBar.setColour(juce::ProgressBar::ColourIds::foregroundColourId, juce::Colours::aquamarine);

    convertThread.conversionEndedBroadcaster->addChangeListener(this);
    finishedBroadcaster = new juce::ChangeBroadcaster();

    fileLenght = lenghtInSeconds;
    juce::File fileToConvert(filePath);
    fileName = fileToConvert.getFileNameWithoutExtension();
    progressFilePath = Settings::convertedSoundsPath + "\\" + fileName + ".txt";
    convertThread.setFilePath(filePath);
    convertThread.stopThread(1000);
    convertThread.startThread();

    juce::Timer::startTimer(100);
}

convertObject::~convertObject()
{
    delete finishedBroadcaster;
}

void convertObject::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
}

void convertObject::resized()
{
    progressBar.setBounds(0, 0, getWidth(), getHeight());
}

void convertObject::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == convertThread.conversionEndedBroadcaster)
    {
        stopTimer();
        convertProgress = 0.;
        //supprime le fichier
        juce::File progressFile(progressFilePath);
        if (progressFile.exists())
        {
            progressFile.deleteFile();
        }
        finishedBroadcaster->sendChangeMessage();
    }
}


void convertObject::timerCallback()
{
    //Mise à jour de la barre de progès
    juce::File progressFile(progressFilePath);
    if (progressFile.exists())
    {
        juce::StringArray progressInfo;
        progressFile.readLines(progressInfo);
        int currentProgressLine = progressInfo.size() - 7; //récupère la ligne out_time_ms
        juce::String progressLine = progressInfo[currentProgressLine].trimCharactersAtStart("out_time_ms=");//enlève le début
        int progressInSeconds = progressLine.getIntValue() / 1000000;// enfait c'est pas des ms mais des µs
        convertProgress = float(progressInSeconds) / float(fileLenght);//pourcentage
    }
}
juce::String convertObject::getReturnedFile()
{
    return convertThread.getFile();
}

int convertObject::getId()
{
    return id;
}

double convertObject::getProgression()
{
    return convertProgress;
}

ffmpegConvert* convertObject::getThread()
{
    return &convertThread;
}