/*
  ==============================================================================

    batchConvertThread.h
    Created: 6 Jan 2022 1:48:52pm
    Author:  Lucien Lefebvre

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "nanodbc/nanodbc.h"
#include <iostream>
#include "PlayHead.h"
#include <Ebu128LoudnessMeter.h>
#include "LoudnessBar.h"
#include "ffmpegConvert.h"
#include "convertObject.h"
class batchConvertThread : public juce::Thread, public juce::ChangeListener

{
public:
    batchConvertThread::batchConvertThread(juce::StringArray fileList, juce::StringArray durationList) : juce::Thread("batch"), fL(fileList), dL(durationList)
    {
        formatManager.registerBasicFormats();
    }

    batchConvertThread::~batchConvertThread()
    {

    }

    void batchConvertThread::run()
    {
        DBG("convert thread run");
        int i = 0;
        while (i < fL.size())
        {
            if (myConvertObjects.size() < 2)
            {
                checkAndConvert(i);
                i++;
            }
        }
        if (threadShouldExit())
            return;
    }

    void batchConvertThread::setList(juce::StringArray fileList, juce::StringArray durationList)
    {
        fL = fileList;
        dL = durationList;
    }

    bool batchConvertThread::checkAndConvert(int rowNumber)
    {
        std::string filePath = std::string("D:\\SONS\\ADMIN\\" + fL[rowNumber].toStdString());
        juce::File sourceFile(filePath);
        std::string fileName = sourceFile.getFileNameWithoutExtension().toStdString();
        juce::String pathToTest = juce::String(Settings::convertedSoundsPath + "\\" + fileName + ".wav");
        juce::File fileToTest(pathToTest);
        if (juce::AudioFormatReader* reader = formatManager.createReaderFor(fileToTest))
        {
            return true;
        }
        else
        {
            int soundDuration = dL[rowNumber].getIntValue() / 1000;
            myConvertObjects.add(new convertObject(filePath, soundDuration, rowNumber));
            myConvertObjects.getLast()->finishedBroadcaster->addChangeListener(this);
            return false;
        }
    }


    void batchConvertThread::changeListenerCallback(juce::ChangeBroadcaster* source)
    {
        DBG("change listener");
        //delete conversion object
        for (int i = 0; i < myConvertObjects.size(); i++)
        {
            if (source == myConvertObjects[i]->finishedBroadcaster)
            {
                //load file and delete object

                myConvertObjects[i]->setVisible(false);
                myConvertObjects.remove(i);
                //remove progress bar
                if (myConvertObjects.size() == 0)
                {
                    //convertProgress.setVisible(false);
                    //isConverting = false;
                }
            }
        }
    }

    juce::StringArray fL;
    juce::StringArray dL;
    juce::AudioFormatManager formatManager;
    juce::OwnedArray<convertObject> myConvertObjects;
};