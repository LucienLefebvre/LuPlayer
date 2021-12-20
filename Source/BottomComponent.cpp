/*
  ==============================================================================

    BottomComponent.cpp
    Created: 15 Mar 2021 6:43:01pm
    Author:  Lucien

  ==============================================================================
*/

#include <JuceHeader.h>
#include "BottomComponent.h"

//==============================================================================
BottomComponent::BottomComponent()
    : TabbedComponent(juce::TabbedButtonBar::Orientation::TabsAtTop)
{

    addTab("Sound Browser", getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), &audioPlaybackDemo, false);
    auto const connection_string = NANODBC_TEXT("Driver=ODBC Driver 17 for SQL Server;Server=localhost\\NETIA;Database=ABC4;Uid=SYSADM;Pwd=SYSADM;");
    noDbLabel.setSize(200, 30);
    noDbLabel.setText("No Database Found", juce::NotificationType::dontSendNotification);
    try
    {
        conn.connect(connection_string, 1000);
    }
    catch (std::runtime_error const& e)
    {
        std::cerr << e.what() << std::endl;
    }
    if (conn.connected())
    {
        addTab("Netia DataBase", getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), &dbBrowser, false);
        addTab("Netia Database Import", getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), &dbImport, false);
    }
    else
    {
        addTab("Netia DataBase", getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), &noDbLabel, false);
        addTab("Netia Database Import", getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), &noDbLabel, false);
    }
    addTab("Recorder", getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), &recorderComponent, false);
    addTab("Distant DataBase", getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), &distantDbBrowser, false);
    setTabBarDepth(tabBarHeight);
    setCurrentTabIndex(1);
    getTabbedButtonBar().setWantsKeyboardFocus(false);
    getTabbedButtonBar().addChangeListener(this);
    myMixer.addInputSource(&audioPlaybackDemo.transportSource, false);
    myMixer.addInputSource(&recorderComponent.cueTransport, false);
    myMixer.addInputSource(&dbBrowser.resampledSource, false);
    myMixer.addInputSource(&dbImport.resampledSource, false);
    myMixer.addInputSource(&distantDbBrowser.resampledSource, false);
    unfocusAllComponents();
    setWantsKeyboardFocus(false);
    
    recorderComponent.recordingBroadcaster->addChangeListener(this);
    dbBrowser.cuePlay->addChangeListener(this);
    audioPlaybackDemo.cuePlay->addChangeListener(this);

    recorderComponent.setName("recorder");
    audioPlaybackDemo.setName("browser");

    cuePlay = new juce::ChangeBroadcaster();
}


BottomComponent::~BottomComponent()
{
    delete cuePlay;
}

void BottomComponent::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

}

void BottomComponent::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
    audioPlaybackDemo.setBounds(0, 0, getWidth(), getHeight() - 25);
    recorderComponent.setBounds(0, 0, getWidth(), getHeight() - 25);
    dbBrowser.setBounds(0, 0, getWidth(), getHeight() - 25);
    TabbedComponent::resized();
    getTabbedButtonBar().setBounds(getTabbedButtonBar().getBounds());
}

void BottomComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{

    myMixer.prepareToPlay(samplesPerBlockExpected, sampleRate);
    recorderComponent.prepareToPlay(samplesPerBlockExpected, sampleRate);
    dbBrowser.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void BottomComponent::tabSelected()
{

    unfocusAllComponents();
    setWantsKeyboardFocus(false);
}

void BottomComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    //Color tab button in red if recording
    if (source == recorderComponent.recordingBroadcaster)
    {
        if (recorderComponent.isRecording())
        {
            getTabbedButtonBar().setTabBackgroundColour(3, juce::Colours::red);
        }
        else
            getTabbedButtonBar().setTabBackgroundColour(3, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    }
    //stop the cue on the others components of the panel and send change message to maincomponent
    else if (source == dbBrowser.cuePlay)
    {
        audioPlaybackDemo.transportSource.stop();
        cuePlay->sendChangeMessage();
    }
    else if (source == audioPlaybackDemo.cuePlay)
    {
        dbBrowser.transport.stop();
        cuePlay->sendChangeMessage();
    }
}

void BottomComponent::currentTabChanged(int newCurrentTabIndex, const juce::String& newCurrentTabName)
{

}

void BottomComponent::stopCue() //stop cues on this panel when a cue is launched on the soundplayer
{
    audioPlaybackDemo.transportSource.stop();
    dbBrowser.transport.stop();
}