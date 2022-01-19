
#pragma once

//#include "../Assets/DemoUtilities.h"
#include <JuceHeader.h>
#include "Settings.h"
//==============================================================================
class DemoThumbnailComp : public juce::Component,
    public juce::ChangeListener,
    public juce::FileDragAndDropTarget,
    public juce::ChangeBroadcaster,
    private juce::ScrollBar::Listener,
    private juce::Timer
{
public:
    DemoThumbnailComp(juce::AudioFormatManager& formatManager,
        juce::AudioTransportSource& source,
        juce::Slider& slider)
        : transportSource(source),
        zoomSlider(slider),
        thumbnail(512, formatManager, thumbnailCache)
    {
        thumbnail.addChangeListener(this);

        //addAndMakeVisible(scrollbar);
        scrollbar.setRangeLimits(visibleRange);
        scrollbar.setAutoHide(false);
        scrollbar.addListener(this);

        currentPositionMarker.setFill(juce::Colours::white.withAlpha(0.85f));
        addAndMakeVisible(currentPositionMarker);
        setWantsKeyboardFocus(false);
    }

    ~DemoThumbnailComp() override
    {
        scrollbar.removeListener(this);
        thumbnail.removeChangeListener(this);
    }

    void setURL(const juce::URL& url)
    {
        juce::InputSource* inputSource = nullptr;

#if ! JUCE_IOS
        if (url.isLocalFile())
        {
            inputSource = new juce::FileInputSource(url.getLocalFile());
        }
        else
#endif
        {
            if (inputSource == nullptr)
                inputSource = new juce::URLInputSource(url);
        }

        if (inputSource != nullptr)
        {
            thumbnail.setSource(inputSource);

            juce::Range<double> newRange(0.0, thumbnail.getTotalLength());
            scrollbar.setRangeLimits(newRange);
            setRange(newRange);

            startTimerHz(40);
        }
    }

    juce::URL getLastDroppedFile() const noexcept { return lastFileDropped; }

    void setZoomFactor(double amount)
    {
        if (thumbnail.getTotalLength() > 0)
        {
            auto newScale = juce::jmax(0.001, thumbnail.getTotalLength() * (1.0 - juce::jlimit(0.0, 0.99, amount)));
            auto timeAtCentre = xToTime((float)getWidth() / 2.0f);

            setRange({ timeAtCentre - newScale * 0.5, timeAtCentre + newScale * 0.5 });
        }
    }

    void setRange(juce::Range<double> newRange)
    {
        visibleRange = newRange;
        scrollbar.setCurrentRange(visibleRange);
        updateCursorPosition();
        repaint();
    }

    void setFollowsTransport(bool shouldFollow)
    {
        isFollowingTransport = shouldFollow;
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        g.setColour(juce::Colour(40, 134, 189));

        if (thumbnail.getTotalLength() > 0.0)
        {
            auto thumbArea = getLocalBounds();

            thumbArea.removeFromBottom(scrollbar.getHeight() + 4);
            thumbnail.drawChannels(g, thumbArea.reduced(2),
                visibleRange.getStart(), visibleRange.getEnd(), 1.0f);
        }
        else
        {
            g.setFont(14.0f);
            g.drawFittedText("(No audio file selected)", getLocalBounds(), juce::Justification::centred, 2);
        }
    }

    void resized() override
    {
        scrollbar.setBounds(getLocalBounds().removeFromBottom(14).reduced(2));
    }

    void changeListenerCallback(ChangeBroadcaster*) override
    {
        // this method is called by the thumbnail when it has changed, so we should repaint it..
        repaint();
    }

   bool isInterestedInFileDrag(const juce::StringArray& /*files*/) override
   {
       return true;
   }

    void filesDropped(const juce::StringArray& files, int /*x*/, int /*y*/) override
    {
        lastFileDropped = juce::URL(juce::File(files[0]));
        sendChangeMessage();
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        mouseDrag(e);
    }

    void mouseDrag(const juce::MouseEvent& e) override
    {
        if (canMoveTransport())
            transportSource.setPosition(juce::jmax(0.0, xToTime((float)e.x)));
    }

    void mouseUp(const juce::MouseEvent&) override
    {
        //transportSource.start();
        //startStopButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
    }

    void mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails& wheel) override
    {
        if (thumbnail.getTotalLength() > 0.0)
        {
            auto newStart = visibleRange.getStart() - wheel.deltaX * (visibleRange.getLength()) / 10.0;
            newStart = juce::jlimit(0.0, juce::jmax(0.0, thumbnail.getTotalLength() - (visibleRange.getLength())), newStart);

            if (canMoveTransport())
                setRange({ newStart, newStart + visibleRange.getLength() });

            if (wheel.deltaY != 0.0f)
                zoomSlider.setValue(zoomSlider.getValue() - wheel.deltaY);

            repaint();
        }
    }


private:
    juce::AudioTransportSource& transportSource;
    juce::Slider& zoomSlider;
    juce::ScrollBar scrollbar{ false };

    juce::AudioThumbnailCache thumbnailCache{ 5 };
    juce::AudioThumbnail thumbnail;
    juce::Range<double> visibleRange;
    bool isFollowingTransport = false;
    juce::URL lastFileDropped;

    juce::DrawableRectangle currentPositionMarker;

    float timeToX(const double time) const
    {
        if (visibleRange.getLength() <= 0)
            return 0;

        return (float)getWidth() * (float)((time - visibleRange.getStart()) / visibleRange.getLength());
    }

    double xToTime(const float x) const
    {
        return (x / (float)getWidth()) * (visibleRange.getLength()) + visibleRange.getStart();
    }

    bool canMoveTransport() const noexcept
    {
        return !(isFollowingTransport && transportSource.isPlaying());
    }

    void scrollBarMoved(juce::ScrollBar* scrollBarThatHasMoved, double newRangeStart) override
    {
        if (scrollBarThatHasMoved == &scrollbar)
            if (!(isFollowingTransport && transportSource.isPlaying()))
                setRange(visibleRange.movedToStartAt(newRangeStart));
    }

    void timerCallback() override
    {
        if (canMoveTransport())
            updateCursorPosition();
        else
            setRange(visibleRange.movedToStartAt(transportSource.getCurrentPosition() - (visibleRange.getLength() / 2.0)));
    }

    void updateCursorPosition()
    {
        currentPositionMarker.setVisible(transportSource.isPlaying() || isMouseButtonDown());

        currentPositionMarker.setRectangle(juce::Rectangle<float>(timeToX(transportSource.getCurrentPosition()) - 0.75f, 0,
            1.5f, (float)(getHeight() - scrollbar.getHeight())));
    }
};

//==============================================================================
class AudioPlaybackDemo : public juce::Component,
    public juce::MouseListener,
    public juce::ChangeBroadcaster,
    public juce::Value::Listener,
    public juce::Timer,
#if (JUCE_ANDROID || JUCE_IOS)
    private Button::Listener,
#else
    private juce::FileBrowserListener,
#endif
    private juce::ChangeListener
{
public:
    AudioPlaybackDemo() : resamplingSource(&transportSource, false, 2)
    {
        juce::Timer::startTimer(500);
        setName("browser");
        setWantsKeyboardFocus(false);
        fileFolder = new juce::File(juce::File::getSpecialLocation(juce::File::userDesktopDirectory));
        int flags = juce::FileBrowserComponent::openMode |
            juce::FileBrowserComponent::canSelectFiles |
            juce::FileBrowserComponent::filenameBoxIsReadOnly;
        m_wcFileFilter = new juce::WildcardFileFilter(("*.wav;*.WAV;*.mp3;*.MP3;*.bwf;*.BWF;*.aif;*.AIF;*.aiff;*.AIFF;*.flac;*.FLAC"), ("*"), ("Audio FIles"));

        Settings::sampleRateValue.addListener(this);

        // create the browser component
        fileBrowser = new juce::FileBrowserComponent(flags, *fileFolder, m_wcFileFilter, NULL);
        fileDraggedFromBrowser = new juce::ChangeBroadcaster();
        fileDroppedFromBrowser = new juce::ChangeBroadcaster();
        cuePlay = new juce::ChangeBroadcaster();
        // add browser compoenent to form and add us as a listener
        addAndMakeVisible(*fileBrowser);
        fileBrowser->setColour(juce::FileBrowserComponent::ColourIds::filenameBoxTextColourId, juce::Colours::white);

        fileBrowser->addListener(this);
        fileBrowser->addMouseListener(this, false);
        addMouseListener(this, true);
        addAndMakeVisible(zoomLabel);
        zoomLabel.setFont(juce::Font(15.00f, juce::Font::plain));
        zoomLabel.setJustificationType(juce::Justification::centredRight);
        zoomLabel.setEditable(false, false, false);
        zoomLabel.setColour(juce::TextEditor::textColourId, juce::Colours::black);
        zoomLabel.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x00000000));

        addAndMakeVisible(followTransportButton);
        followTransportButton.onClick = [this] { updateFollowTransportState(); };

        addAndMakeVisible(&timeLabel);
        timeLabel.setFont(juce::Font(20.00f, juce::Font::plain).withTypefaceStyle("Regular"));
        timeLabel.setJustificationType(juce::Justification::centred);

#if (JUCE_ANDROID || JUCE_IOS)
        addAndMakeVisible(chooseFileButton);
        chooseFileButton.addListener(this);
#else
        //addAndMakeVisible(fileTreeComp);

        directoryList.setDirectory(juce::File::getSpecialLocation(juce::File::userDesktopDirectory), true, true);

        fileTreeComp.setColour(juce::FileTreeComponent::backgroundColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        fileTreeComp.addListener(this);

        addAndMakeVisible(explanation);
        explanation.setFont(juce::Font(14.00f, juce::Font::plain));
        explanation.setJustificationType(juce::Justification::bottomRight);
        explanation.setEditable(false, false, false);
        explanation.setColour(juce::TextEditor::textColourId, juce::Colours::black);
        explanation.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x00000000));
#endif

        addAndMakeVisible(zoomSlider);
        zoomSlider.setRange(0, 1, 0);
        zoomSlider.onValueChange = [this] { thumbnail->setZoomFactor(zoomSlider.getValue()); };
        zoomSlider.setSkewFactor(2);

        thumbnail.reset(new DemoThumbnailComp(formatManager, transportSource, zoomSlider));
        addAndMakeVisible(thumbnail.get());
        thumbnail->addChangeListener(this);

        addAndMakeVisible(startStopButton);
        startStopButton.setColour(juce::TextButton::buttonColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        startStopButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        startStopButton.onClick = [this] { startOrStop(); };
        startStopButton.setEnabled(false);

        addAndMakeVisible(autoPlayButton);
        autoPlayButton.setToggleState(true, juce::NotificationType::dontSendNotification);

        // audio setup
        formatManager.registerBasicFormats();
        transportSource.addChangeListener(this);
        thread.startThread(3);

#ifndef JUCE_DEMO_RUNNER
        juce::RuntimePermissions::request(juce::RuntimePermissions::recordAudio,
            [this](bool granted)
            {
                int numInputChannels = granted ? 2 : 0;
                audioDeviceManager.initialise(numInputChannels, 2, nullptr, true, {}, nullptr);
            });
#endif

        //audioDeviceManager.addAudioCallback(&audioSourcePlayer);
        //audioSourcePlayer.setSource(&transportSource);

        setOpaque(true);
        setSize(500, 500);
    }

    ~AudioPlaybackDemo() override
    {
        delete m_wcFileFilter;
        delete fileBrowser;
        delete fileFolder;
        delete fileDraggedFromBrowser;
        delete fileDroppedFromBrowser;
        delete cuePlay;
        transportSource.setSource(nullptr);
        //audioSourcePlayer.setSource(nullptr);

        //audioDeviceManager.removeAudioCallback(&audioSourcePlayer);

#if (JUCE_ANDROID || JUCE_IOS)
        chooseFileButton.removeListener(this);
#else
        fileTreeComp.removeListener(this);
#endif

        thumbnail->removeChangeListener(this);
    }


    void mouseDrag(const juce::MouseEvent& event)
    {
        fileBrowser->setMouseCursor(juce::MouseCursor::DraggingHandCursor);
        fileDraggedFromBrowser->sendChangeMessage();
    }

    void mouseUp(const juce::MouseEvent& event)
    {
        fileDroppedFromBrowser->sendChangeMessage();
        setMouseCursor(juce::MouseCursor::NormalCursor);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        g.setColour(juce::Colour(40, 134, 189));
        g.fillRect(getWidth() / 2, 0, 4, getHeight());
    }

    void resized() override
    {
        thumbnail->setBounds(getWidth() / 2 + 4, 30, getWidth() / 2, getHeight() - 30);
        startStopButton.setBounds(getWidth() / 2 + 4, 0, 100, 25);
        fileBrowser->setBounds(0, 0, getParentWidth() / 2, getParentHeight() - 25);
        autoPlayButton.setBounds(getWidth() / 2 + 4 + 101, 0, 100, 25);
        autoPlayButton.setToggleState(false, juce::NotificationType::dontSendNotification);
        timeLabel.setBounds(autoPlayButton.getRight(), 0, getWidth() - autoPlayButton.getRight(), 25);
        //fileTreeComp.
        auto r = getLocalBounds().reduced(4);

        auto controls = r.removeFromBottom(90);

        auto controlRightBounds = controls.removeFromRight(controls.getWidth() / 3);

#if (JUCE_ANDROID || JUCE_IOS)
        chooseFileButton.setBounds(controlRightBounds.reduced(10));
#else
        explanation.setBounds(controlRightBounds);
#endif

        auto zoom = controls.removeFromTop(25);
        //zoomLabel.setBounds(zoom.removeFromLeft(50));
        //zoomSlider.setBounds(zoom);

        //followTransportButton.setBounds(controls.removeFromTop(25));


        r.removeFromBottom(6);

#if JUCE_ANDROID || JUCE_IOS
        thumbnail->setBounds(r);
#else

#endif
    }
    void valueChanged(juce::Value& value)
    {
        if (value.refersToSameSourceAs(Settings::sampleRateValue))
        {
            resamplingSource.setResamplingRatio(fileSampleRate / Settings::sampleRate);
        }
    }

    void keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent)
    {
        if (isVisible())
        {
            if (key == juce::KeyPress::spaceKey)
            {
                startOrStop();
            }
        }
    }

    juce::FileBrowserComponent* fileBrowser;
    juce::File* fileFolder;
    juce::AudioTransportSource transportSource;
    juce::ResamplingAudioSource resamplingSource;
    juce::WildcardFileFilter* m_wcFileFilter;
    juce::ChangeBroadcaster* fileDraggedFromBrowser;
    juce::ChangeBroadcaster* fileDroppedFromBrowser;
    juce::ChangeBroadcaster* cuePlay;
    juce::Label timeLabel;
private:
    // if this PIP is running inside the demo runner, we'll use the shared device manager instead
#ifndef JUCE_DEMO_RUNNER
    juce::AudioDeviceManager audioDeviceManager;
#else
    AudioDeviceManager& audioDeviceManager{ getSharedAudioDeviceManager(0, 2) };
#endif
    //std::unique_ptr<juce::FileBrowserComponent> m_fileBrowser = std::make_unique< juce::FileBrowserComponent>();
    juce::AudioFormatManager formatManager;
    juce::TimeSliceThread thread{ "audio file preview" };

#if (JUCE_ANDROID || JUCE_IOS)
    std::unique_ptr<FileChooser> fileChooser;
    TextButton chooseFileButton{ "Choose Audio File...", "Choose an audio file for playback" };
#else

    juce::DirectoryContentsList directoryList{ nullptr, thread };
    juce::FileTreeComponent fileTreeComp{ directoryList };
    juce::Label explanation{ {}, "" };
#endif

    juce::URL currentAudioFile;
    //juce::AudioSourcePlayer audioSourcePlayer;

    std::unique_ptr<juce::AudioFormatReaderSource> currentAudioFileSource;

    std::unique_ptr<DemoThumbnailComp> thumbnail;
    juce::Label zoomLabel{ {}, "zoom:" };
    juce::Slider zoomSlider{ juce::Slider::LinearHorizontal, juce::Slider::NoTextBox };
    juce::ToggleButton followTransportButton{ "Follow Transport" };
    juce::TextButton startStopButton{ "Play/Stop" };
    juce::ToggleButton autoPlayButton{ "Autoplay" };

    double fileSampleRate = 48000;
    //==============================================================================
    void showAudioResource(juce::URL resource)
    {
        if (loadURLIntoTransport(resource))
            currentAudioFile = std::move(resource);

        zoomSlider.setValue(0, juce::dontSendNotification);
        thumbnail->setURL(currentAudioFile);
    }

    bool loadURLIntoTransport(const juce::URL& audioURL)
    {
        // unload the previous file source and delete it..
        transportSource.stop();
        transportSource.setSource(nullptr);
        currentAudioFileSource.reset();

        juce::AudioFormatReader* reader = nullptr;

#if ! JUCE_IOS
        if (audioURL.isLocalFile())
        {
            reader = formatManager.createReaderFor(audioURL.getLocalFile());
        }
        else
#endif
        {
            if (reader == nullptr)
                reader = formatManager.createReaderFor(audioURL.createInputStream(false));
        }

        if (reader != nullptr)
        {
            currentAudioFileSource.reset(new juce::AudioFormatReaderSource(reader, true));

            // ..and plug it into our transport source
            transportSource.setSource(currentAudioFileSource.get(),
                32768,                   // tells it to buffer this many samples ahead
                &thread,                 // this is the background thread to use for reading-ahead
                reader->sampleRate);     // allows for sample rate correction
            resamplingSource.setResamplingRatio(reader->sampleRate / Settings::sampleRate);
            fileSampleRate = reader->sampleRate;
            return true;
        }

        return false;
    }

    void startOrStop()
    {
        if (transportSource.isPlaying())
        {
            transportSource.stop();
            startStopButton.setColour(juce::TextButton::buttonColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        }
        else
        {
            cuePlay->sendChangeMessage();
            transportSource.setPosition(0);
            transportSource.start();
            startStopButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
        }
    }

    void updateFollowTransportState()
    {
        thumbnail->setFollowsTransport(followTransportButton.getToggleState());
    }

#if (JUCE_ANDROID || JUCE_IOS)
    void buttonClicked(Button* btn) override
    {
        if (btn == &chooseFileButton && fileChooser.get() == nullptr)
        {
            SafePointer<AudioPlaybackDemo> safeThis(this);

            if (!RuntimePermissions::isGranted(RuntimePermissions::readExternalStorage))
            {
                RuntimePermissions::request(RuntimePermissions::readExternalStorage,
                    [safeThis](bool granted) mutable
                    {
                        if (granted)
                            safeThis->buttonClicked(&safeThis->chooseFileButton);
                    });
                return;
            }

            if (FileChooser::isPlatformDialogAvailable())
            {
                fileChooser.reset(new FileChooser("Select an audio file...", File(), "*.wav;*.mp3;*.aif"));

                fileChooser->launchAsync(FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles,
                    [safeThis](const FileChooser& fc) mutable
                    {
                        if (safeThis != nullptr && fc.getURLResults().size() > 0)
                        {
                            auto u = fc.getURLResult();

                            safeThis->showAudioResource(std::move(u));
                        }

                        safeThis->fileChooser = nullptr;
                    }, nullptr);
            }
            else
            {
                NativeMessageBox::showMessageBoxAsync(AlertWindow::WarningIcon, "Enable Code Signing",
                    "You need to enable code-signing for your iOS project and enable \"iCloud Documents\" "
                    "permissions to be able to open audio files on your iDevice. See: "
                    "https://forum.juce.com/t/native-ios-android-file-choosers");
            }
        }
    }
#else
    void selectionChanged() override
    {
        showAudioResource(juce::URL(fileBrowser->getSelectedFile(0)));
        startStopButton.setEnabled(true);
        if (autoPlayButton.getToggleState())
        {
            if (!transportSource.isPlaying())
                startOrStop();
        }
    }

    void fileClicked(const juce::File&, const juce::MouseEvent&) override {}
    void fileDoubleClicked(const juce::File&) override {}
    void browserRootChanged(const juce::File&) override {}
#endif

    void changeListenerCallback(juce::ChangeBroadcaster* source) override
    {
        if (source == thumbnail.get())
            showAudioResource(juce::URL(thumbnail->getLastDroppedFile()));
        if (source == &transportSource)
        {
            if (!transportSource.isPlaying())
                startStopButton.setColour(juce::TextButton::buttonColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
            else
                startStopButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
        }
    }

    void timerCallback()
    {
        auto elapsedTime = secondsToMMSS(transportSource.getCurrentPosition());
        auto remainingTime = secondsToMMSS(transportSource.getLengthInSeconds() - transportSource.getCurrentPosition());
        timeLabel.setText(elapsedTime + " // " + remainingTime, juce::NotificationType::dontSendNotification);
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
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPlaybackDemo)
};
