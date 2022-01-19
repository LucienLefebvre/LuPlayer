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

        mainWindow.reset (new MainWindow ("MultiPlayer beta"));
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
            mainComponent.setSize(getWidth(), getHeight());
            setContentOwned (&mainComponent, true);
            setResizable(true, false);


           #if JUCE_IOS || JUCE_ANDROID
            setFullScreen (true);
           #else
            //setResizable (true, true);
            //centreWithSize (getWidth(), getHeight());
           #endif
            //setSize(1300, 550);
            setResizeLimits(900, 400, 1920, 1080);


            //setTitleBarHeight(30);

            //toFront(true);
            setWantsKeyboardFocus(true);

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
            if (mainComponent.isPlayingOrRecording())
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
                            mainComponent.deleteConvertedFiles();
                            JUCEApplication::getInstance()->systemRequestedQuit();
                        }
                        else if (result == 2)
                        {
                            JUCEApplication::getInstance()->systemRequestedQuit();
                        }
                    }
                    else
                    {
                        JUCEApplication::getInstance()->systemRequestedQuit();
                    }
                }
            }
            else if (Settings::tempFiles.size() > 0)
            {
                std::unique_ptr<juce::AlertWindow> deleteFilesWindow;
                int result = deleteFilesWindow->showYesNoCancelBox(juce::AlertWindow::QuestionIcon, "Delete converted sounds ?", "");
                if (result == 1)
                {
                    mainComponent.deleteConvertedFiles();
                    JUCEApplication::getInstance()->systemRequestedQuit();
                }
                else if (result == 2)
                {
                    JUCEApplication::getInstance()->systemRequestedQuit();
                }
            }
            else 
                JUCEApplication::getInstance()->systemRequestedQuit();


        }



        /* Note: Be careful if you override any DocumentWindow methods - the base
           class uses a lot of them, so by overriding you might break its functionality.
           It's best to do all your work in your content component instead, but if
           you really have to override any DocumentWindow methods, make sure your
           subclass also calls the superclass's method.
        */

    private:
        MainComponent mainComponent;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:

    std::unique_ptr<MainWindow> mainWindow;
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (MultiPlayerApplication)


