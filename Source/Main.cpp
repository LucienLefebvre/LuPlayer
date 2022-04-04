/*
  ==============================================================================

    This file contains the basic startup code for a JUCE application.

  ==============================================================================
*/

#include <JuceHeader.h>
#include "MainComponent.h"
#include "Settings.h"
#define JUCE_ASIO 1
//==============================================================================
class MultiPlayerApplication  : public juce::JUCEApplication
{
public:
    //==============================================================================
    MultiPlayerApplication() {}

    const juce::String getApplicationName() override       { return ProjectInfo::projectName; }
    const juce::String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override             { return true; }

    //==============================================================================
    void initialise (const juce::String& commandLine) override
    {
        // This method is where you should put your application's initialisation code..

        mainWindow.reset (new MainWindow ("LuPlayer Beta"));
        mainWindow->setCommandLine(commandLine);
    }

    void shutdown() override
    {
        // Add your application's shutdown code here..

        mainWindow = nullptr; // (deletes our window)
    }

    //==============================================================================
    void systemRequestedQuit() override
    {
        // This is called when the app is being asked to quit: you can ignore this
        // request and let the app carry on running, or call quit() to allow the app to close.
        if (mainWindow->mainComponent.isPlayingOrRecording())
        {
            std::unique_ptr<juce::AlertWindow> quitWindow;
            bool result = quitWindow->showOkCancelBox(juce::AlertWindow::QuestionIcon, "Quit ?", "Some sounds are playing");
            if (result == true)
            {
                if (Settings::tempFiles.size() > 0)
                {
                    std::unique_ptr<juce::AlertWindow> deleteFilesWindow;
                    int result = deleteFilesWindow->showYesNoCancelBox(juce::AlertWindow::QuestionIcon, "Delete converted sounds ?", "");
                    if (result == 1)
                    {
                        mainWindow->mainComponent.deleteConvertedFiles();
                        quit();
                    }
                    else if (result == 2)
                    {
                        quit();
                    }
                }
                else
                {
                    quit();
                }
            }
            else
                return;
        }
        if (Settings::tempFiles.size() > 0 && !mainWindow->mainComponent.hasBeenSaved())
        {
            std::unique_ptr<juce::AlertWindow> deleteFilesWindow;
            int result = deleteFilesWindow->showYesNoCancelBox(juce::AlertWindow::QuestionIcon, "Delete converted sounds ?", "");
            if (result == 1)
            {
                mainWindow->mainComponent.deleteConvertedFiles();
                quit();
            }
            else if (result == 2)
            {
                quit();
            }
        }
        else
            quit();



    }
    void anotherInstanceStarted (const juce::String& commandLine) override
    {
        // When another instance of the app is launched while this one is running,
        // this method is invoked, and the commandLine parameter tells you what
        // the other instance's command-line arguments were.
    }

    //==============================================================================
    /*
        This class implements the desktop window that contains an instance of
        our MainComponent class.
    */
    class MainWindow    : public juce::DocumentWindow
    {
    public:
        MainWindow (juce::String name)
            : DocumentWindow (name,
                              juce::Desktop::getInstance().getDefaultLookAndFeel()
                                                          .findColour (juce::ResizableWindow::backgroundColourId),
                              DocumentWindow::allButtons)
        {
            //setUsingNativeTitleBar (true);
            setFullScreen(true);
            setVisible(true);
            setUsingNativeTitleBar(true);
            mainComponent.setSize(1280, 768);
            setContentOwned (&mainComponent, true);
            setResizable(true, false);


           #if JUCE_IOS || JUCE_ANDROID
            setFullScreen (true);
           #else
            //setResizable (true, true);
            //centreWithSize (getWidth(), getHeight());
           #endif
            //setSize(1300, 550);
            setResizeLimits(1000, 600, 10000, 10000);


            //setTitleBarHeight(30);

            //toFront(true);
            setWantsKeyboardFocus(true);
            mainComponent.grabKeyboardFocus();
            //setAlwaysOnTop(true);

            //runModalLoop(); 
        } 

        void setCommandLine(juce::String command)
        {
            //mainComponent.setCommandLine("8p");
        }
        void closeButtonPressed() override
        {
            // This is called when the user tries to close this window. Here, we'll just
            // ask the app to quit when this happens, but you can change this to do
            // whatever you need.
            juce::JUCEApplication::getInstance()->systemRequestedQuit();

        }
        MainComponent mainComponent;
    private:

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:

    std::unique_ptr<MainWindow> mainWindow;
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (MultiPlayerApplication)


