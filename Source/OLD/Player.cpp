/*
  ==============================================================================

    Player.cpp
    Created: 26 Jan 2021 7:36:59pm
    Author:  Lucien

  ==============================================================================
*/

#include <JuceHeader.h>
#include "Player.h"
#include "Windows.h"
//#include <string>
//==============================================================================
Player::Player(int index): openButton("Open"), playButton("Play"), stopButton("Stop"), cueButton("Cue"), deleteButton("Delete"), thumbnailCache(5), thumbnail(521, formatManager, thumbnailCache), resampledSource(&transport, false, 2)
{
    playerIndex = index;
    juce::Timer::startTimer(100);
    state = Stopped;

    int playerPosition = playerIndex + 1;
    addAndMakeVisible(playerPositionLabel);
    playerPositionLabel.setText(juce::String(playerPosition), juce::NotificationType::dontSendNotification);
    playerPositionLabel.setBounds(borderRectangleWidth, 0, 50, 50);
    playerPositionLabel.setFont(juce::Font(40.00f, juce::Font::plain).withTypefaceStyle("Regular"));
    playerPositionLabel.setJustificationType(juce::Justification::centredTop);

    addAndMakeVisible(&openButton);
    openButton.onClick = [this] { openButtonClicked(); };
    openButton.setBounds(rightControlsStart, 80, openDeleteButtonWidth, 20);
    openButton.setWantsKeyboardFocus(false);


    addAndMakeVisible(&deleteButton);
    deleteButton.onClick = [this] { deleteFile(); };
    deleteButton.setBounds(rightControlsStart + openDeleteButtonWidth, 80, openDeleteButtonWidth, 20);
    deleteButton.setWantsKeyboardFocus(false);

    addAndMakeVisible(&playButton);
    playButton.onClick = [this] { playButtonClicked(); };
    playButton.setBounds(rightControlsStart, 0, playStopButtonWidth, 39);
    playButton.setEnabled(false);
    playButton.setWantsKeyboardFocus(false);


    addAndMakeVisible(&stopButton);
    stopButton.onClick = [this] { stopButtonClicked(); };
    stopButton.setBounds(rightControlsStart + playStopButtonWidth, 0, playStopButtonWidth, 39);
    stopButton.setEnabled(false);
    stopButton.setWantsKeyboardFocus(false);


    addAndMakeVisible(&cueButton);
    cueButton.onClick = [this] { cueButtonClicked(); };
    cueButton.setBounds(rightControlsStart + 2*playStopButtonWidth, 0, playStopButtonWidth, 39);
    cueButton.setEnabled(false);
    cueButton.setWantsKeyboardFocus(false);


    addAndMakeVisible(&loopButton);
    loopButton.setBounds(380, 80, 70, 20);
    loopButton.setButtonText("loop");
    loopButton.setEnabled(true);
    loopButton.onClick = [this]{ updateLoopButton(&loopButton, "loop");; };
    loopButton.setToggleState(false, true);
    loopButton.setWantsKeyboardFocus(false);

    addAndMakeVisible(volumeSlider);
    volumeSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);    
    volumeSlider.setBounds(rightControlsStart + rightControlsWidth, -5, volumeSliderWidth, 100);
    volumeSlider.setRange(0,1.0);
    volumeSlider.addListener(this);
    volumeSlider.setValue(0.);
    formatManager.registerBasicFormats();
    volumeSlider.setSkewFactor(0.5, false);
    volumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 50, 15);
    volumeSlider.setWantsKeyboardFocus(false);


    addAndMakeVisible(volumeLabel);
    volumeLabel.setFont(juce::Font(10.00f, juce::Font::plain).withTypefaceStyle("Regular"));
    volumeLabel.setJustificationType(juce::Justification::centred);
    volumeLabel.setEditable(false, false, false);
    volumeLabel.setBounds(rightControlsStart + rightControlsWidth, 90, volumeSliderWidth, 10);
    volumeLabel.setText("0", juce::NotificationType::dontSendNotification);
    volumeLabel.setWantsKeyboardFocus(false);


    addAndMakeVisible(trimVolumeSlider);
    trimVolumeSlider.setRange(-24, 24, 0.5);
    trimVolumeSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    trimVolumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 50, 10);
    trimVolumeSlider.addListener(this);
    trimVolumeSlider.setBounds(borderRectangleWidth -8, 40, 64, 56);
    trimVolumeSlider.setDoubleClickReturnValue(true, 0.);
    trimVolumeSlider.setPopupDisplayEnabled(true, true, this, 2000);
    trimVolumeSlider.setWantsKeyboardFocus(false);

 
    addAndMakeVisible(trimLabel);
    trimLabel.setFont(juce::Font(15.00f, juce::Font::plain).withTypefaceStyle("Regular"));
    trimLabel.setJustificationType(juce::Justification::centred);
    trimLabel.setEditable(false, false, false);
    trimLabel.setText(TRANS("Trim"), juce::NotificationType::dontSendNotification);
    trimLabel.setBounds(borderRectangleWidth, 81, 50, 24);
    trimLabel.setWantsKeyboardFocus(false);



    addAndMakeVisible(remainingTimeLabel);
    remainingTimeLabel.setBounds(450, 40, 150, 39);
    remainingTimeLabel.setText(TRANS("0"), juce::NotificationType::dontSendNotification);
    remainingTimeLabel.setJustificationType(juce::Justification::centred);
    remainingTimeLabel.setFont(juce::Font(35.00f, juce::Font::plain).withTypefaceStyle("Regular"));
    remainingTimeLabel.setWantsKeyboardFocus(false);


    addAndMakeVisible(soundName);
    soundName.setBounds(leftControlsWidth + borderRectangleWidth, 80, 330, 20);
    soundName.setJustificationType(juce::Justification::centred);
    soundName.setFont(juce::Font(15.0f, juce::Font::plain).withTypefaceStyle("Regular"));
    soundName.setText(juce::String(""), juce::NotificationType::dontSendNotification);
    soundName.setWantsKeyboardFocus(false);

    //TODO régler bug accents dans string

    addAndMakeVisible(playerIDLabel);
    //playerIDLabel.setBounds(400, 80, 50, 20);
    playerIDLabel.setText(juce::String(playerIndex), juce::NotificationType::dontSendNotification);
        
    thumbnail.addChangeListener(this);
    thumbnailBounds.setBounds(leftControlsWidth + borderRectangleWidth, 0, waveformThumbnailXSize, 80);


    /*auto preloadedFile = juce::File("D:\Sons\paris 22k.wav");
    loadFile(preloadedFile);*/

    int totalPlayerWidth = rightControlsStart + rightControlsWidth + volumeSliderWidth + borderRectangleWidth;
    DBG(totalPlayerWidth);
    repaint();
    }

Player::~Player()
{
    //delete Player;
    //deleteAllChildren();
}

void Player::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

    float x = 0.0f, y = 0.0f, width = 650.0f, height = 100.0f;
    juce::Colour fillColour = juce::Colour(0x23000000);
    juce::Colour strokeColour = juce::Colours::black;

    /*g.setColour(fillColour);
    g.fillRoundedRectangle(x, y, width, height, 2.000f);
    g.setColour(strokeColour);
    g.drawRoundedRectangle(x, y, width, height, 2.000f, 1.200f);*/

    //Waveform
    if (thumbnail.getNumChannels() == 0)
        paintIfNoFileLoaded(g, thumbnailBounds);
    else
    {
        if (waveformPainted == false)
        {
            paintIfFileLoaded(g, thumbnailBounds, thumbnailZoomValue);
            //waveformPainted = true;
        }
        paintPlayHead(g, thumbnailBounds);
    }

    {//rectangle Fader1
        int x = 0, y = 0, width = borderRectangleWidth, height = borderRectangleHeight;
        if (fader1IsPlaying == false && isNextPlayer == true)
        {
            g.setColour(juce::Colour(255, 163, 0));
            g.fillRect(x, y, width, height);
        }
        else if (fader1IsPlaying == true)
        {
            g.setColour(juce::Colours::green);
            g.fillRect(x, y, width, height);
        }
        if (((transport.getLengthInSeconds() - transport.getCurrentPosition() < 6)) && fileLoaded == true && state == Playing)
        {
            g.setColour(juce::Colours::red);
            g.fillRect(x, y, width, height);
        }
    }

    {//rectangle Fader2
        int x = rightControlsStart + rightControlsWidth + volumeSliderWidth, y = 0, width = borderRectangleWidth, height = borderRectangleHeight;
        if (fader2IsPlaying == false && isNextPlayer == true)
        {
            g.setColour(juce::Colour(255, 163, 0));
            g.fillRect(x, y, width, height);
        }
        else if (fader2IsPlaying == true)
        {
            g.setColour(juce::Colours::green);
            g.fillRect(x, y, width, height);
        }
        if (((transport.getLengthInSeconds() - transport.getCurrentPosition() < 6)) && fileLoaded == true && state == Playing)
        {
            g.setColour(juce::Colours::red);
            g.fillRect(x, y, width, height);
        }

    }
}

//WAVEFORM DRAWING
void Player::thumbnailChanged()
{
    repaint();
}

void Player::paintIfNoFileLoaded(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds)
{
    g.setColour(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.fillRect(thumbnailBounds);
    g.setColour(juce::Colours::white);
    g.drawFittedText("No File Loaded", thumbnailBounds, juce::Justification::centred, 1);
}

void Player::paintIfFileLoaded(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds, float thumbnailZoomValue)
{
    // waveform background
    g.setColour(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.fillRect(thumbnailBounds);

    //waveform green if playing, red if remaining time <5s, orange if next player
    if (state == Playing)
    {
        if ((transport.getLengthInSeconds() - transport.getCurrentPosition() < 6))
            g.setColour(juce::Colours::red);
        else
            g.setColour(juce::Colours::green);
    }
    else
        if (isNextPlayer == true) g.setColour(juce::Colour(255, 163, 0));
        else g.setColour(juce::Colour(0, 94, 255));


    thumbnail.drawChannels(g,
        thumbnailBounds,
        0.0,                                    // start time
        thumbnail.getTotalLength(),             // end time
        thumbnailZoomValue);                                  // vertical zoom


}

void Player::paintPlayHead(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds)
{

    g.setColour(juce::Colours::black);

    auto audioPosition = (float)transport.getCurrentPosition();
    auto currentPosition = transport.getLengthInSeconds();

    auto drawPosition = ((audioPosition / currentPosition) * (float)thumbnailBounds.getWidth())
        + (float)thumbnailBounds.getX();                                // [13]


    g.drawLine(drawPosition, (float)thumbnailBounds.getY(), drawPosition,
        (float)thumbnailBounds.getBottom(), 2.0f);



}

void Player::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}


void Player::setLabelPlayerPosition(int playerPosition)
{
    playerPositionLabel.setText(juce::String(playerPosition + 1), juce::NotificationType::dontSendNotification);

}

//TIMER
void Player::timerCallback()
{
    updateRemainingTime();
    repaint();
    //paintPlayHead(g, thumbnailBounds);
}

void Player::updateRemainingTime()
{
    int remainingSeconds = juce::int16(trunc(transport.getLengthInSeconds() - transport.getCurrentPosition()));
    int remainingTimeSeconds = remainingSeconds % 60;
    int remainingTimeMinuts = trunc(remainingSeconds / 60);
    juce::String remainingTimeString = juce::String(remainingTimeMinuts) + ":" + juce::String(remainingTimeSeconds);
    remainingTimeLabel.setText(remainingTimeString, juce::NotificationType::dontSendNotification);
}

//AUDIO OUTPUT
void Player::playerPrepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    resampledSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    actualSamplesPerBlockExpected = samplesPerBlockExpected;
    actualSampleRate = sampleRate;
    
}

void Player::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();

    resampledSource.getNextAudioBlock(bufferToFill);
}


//Play Control
void Player::play()
{
    const juce::MessageManagerLock mmLock;
    transportStateChanged(Starting);

}

void Player::stop()
{
    const juce::MessageManagerLock mmLock;
    transportStateChanged(Stopping);
}

//BUTTONS
void Player::openButtonClicked()
{
    //create a chooser to open file
    juce::FileChooser chooser("Choose an audio File", juce::File::getSpecialLocation(juce::File::userDesktopDirectory), "*.wav; *.mp3; *.bwf; *.aiff; *.opus; *.flac"); 

    if (chooser.browseForFileToOpen())
    {
        juce::File myFile;
        myFile = chooser.getResult();
        
        auto filePath = myFile.getFullPathName();
        verifyAudioFileFormat(filePath);
    }
}





void Player::deleteFile()
{
    transport.releaseResources();
    thumbnail.setSource(nullptr);
    transport.setSource(nullptr);
    playButton.setEnabled(false);
    stopButton.setEnabled(false);
    cueButton.setEnabled(false);
    soundName.setText("", juce::NotificationType::dontSendNotification);
    fileLoaded = false;
}


void Player::playButtonClicked()
{
    //const juce::MessageManagerLock mmLock;
    volumeSlider.setValue(1.0);
    play();
}


void Player::stopButtonClicked()
{
    const juce::MessageManagerLock mmLock;
    if (looping == false)
    {
        volumeSlider.setValue(0.0);
        transport.setPosition(0.0);
        stop();
        stopButtonClickedBool = false;
    }
    else if (looping == true)
    {
        volumeSlider.setValue(0.0);
        transport.setPosition(0.0);
        stopButtonClickedBool = true;
        transportStateChanged(Stopping);
        stopButtonClickedBool = false;
    }

}

void Player::cueButtonClicked()
{
    if (transport.getLengthInSeconds() >= 6)
        transport.setPosition(transport.getLengthInSeconds() - 6);
    if (state != Playing)
        transportStateChanged(Starting);
}

void Player::updateLoopButton(juce::Button* button, juce::String name)
{
    auto state = button->getToggleState();
    looping = state;
    DBG(int(state));
    //transport.isLooping();
    transport.setLooping(looping);
    
}

void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{

}

//LISTENERS
void Player::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &transport)
    {
        if (transport.isPlaying())
        {
            transportStateChanged(Playing);
        }
        else
        {
            transportStateChanged(Stopping);

        }
    }

    if (source == &thumbnail)
    {
        thumbnailChanged();
    }
}

void Player::transportStateChanged(TransportState newState)
{

    if (newState != state)
    {
        state = newState;


        switch (state)
        {
        case Stopped:
                transport.setPosition(0.0);
                playButton.setEnabled(true);
                break;
        case Starting:
            playButton.setEnabled(false);
            transport.start();
            break;
        case Stopping:
            if (looping == true && stopButtonClickedBool == false)
            {
                playButton.setEnabled(true);
                transport.setPosition(0.0);
                //transport.stop();
                transport.start();
            }
            else if ((looping == true && stopButtonClickedBool == true) || looping == false)
            {
                playButton.setEnabled(true);
                transport.setPosition(0.0);
                transport.stop();
                stopButtonClickedBool == false;
            }
                break;       
        }

    }
}

void Player::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &transportPositionSlider)
    {
        if (transportPositionSlider.isMouseButtonDown() == 1)
        {
            transport.setPosition(transportPositionSlider.getValue());
        }
    }
    if (slider == &volumeSlider)
    {
        transport.setGain(juce::Decibels::decibelsToGain(trimVolumeSlider.getValue()) * volumeSlider.getValue());
        volumeLabel.setText(juce::String(juce::Decibels::gainToDecibels(volumeSlider.getValue())), juce::NotificationType::dontSendNotification);
    }
    if (slider == &trimVolumeSlider)
    {
        transport.setGain(juce::Decibels::decibelsToGain(trimVolumeSlider.getValue()) * volumeSlider.getValue());
        thumbnailZoomValue = juce::Decibels::decibelsToGain(trimVolumeSlider.getValue());
        if (thumbnail.getNumChannels() != 0)
        {
            //repaint();
        }
    }

}



//WAVEFORM CONTROL
void Player::mouseDrag(const juce::MouseEvent &event)
{
    if (thumbnail.getNumChannels() != 0)
    {
        mouseDragXPosition = getMouseXYRelative().getX();
        if (mouseDragXPosition >= waveformThumbnailXStart && mouseDragXPosition <= waveformThumbnailXEnd)
        {
            //get mouse drag position inside the waveform
            mouseDragRelativeXPosition = getMouseXYRelative().getX() - waveformThumbnailXStart;
            //rule of 3
            mouseDragInSeconds = ((float)mouseDragRelativeXPosition * transport.getLengthInSeconds()) / (float)waveformThumbnailXSize;
            transport.setPosition(mouseDragInSeconds);
            //repaint();
        }
        
    }
    

}

void Player::mouseDown(const juce::MouseEvent& event)
{
    if (thumbnail.getNumChannels() != 0)
    {
        mouseDragXPosition = getMouseXYRelative().getX();
        if (mouseDragXPosition >= waveformThumbnailXStart && mouseDragXPosition <= waveformThumbnailXEnd)
        {
            //get mouse drag position inside the waveform
            mouseDragRelativeXPosition = getMouseXYRelative().getX() - waveformThumbnailXStart;
            //rule of 3
            mouseDragInSeconds = ((float)mouseDragRelativeXPosition * transport.getLengthInSeconds()) / (float)waveformThumbnailXSize;
            transport.setPosition(mouseDragInSeconds);
            //repaint();
        }

    }


}


//MIDI CONTROL
void Player::handleMidiMessage(int midiMessageNumber, int midiMessageValue)
{
    const juce::MessageManagerLock mmLock;

        floatMidiMessageValue = midiMessageValue / 127.;

        volumeSlider.setValue(floatMidiMessageValue);
}

void Player::setNextPlayer(bool trueOrFalse)
{
    isNextPlayer = trueOrFalse;
}


////Drag & drop Files
//bool Player::isInterestedInFileDrag(const juce::StringArray& files)
//{
//    for (auto file : files)
//
//        if ((file.contains(".wav")) || (file.contains(".WAV")) 
//                || (file.contains(".bwf")) || (file.contains(".BWF")) 
//                || (file.contains(".aiff")) || (file.contains(".AIFF"))
//                || (file.contains(".aif")) || (file.contains(".AIF"))
//                || (file.contains(".flac")) || (file.contains(".FLAC"))
//                || (file.contains(".opus")) || (file.contains(".OPUS"))
//                || (file.contains(".mp3")) || (file.contains(".MP3")))
//        {
//            return true;
//        }
//    return false;
//}
//
//void Player::filesDropped(const juce::StringArray& files, int x, int y)
//{
//    for (auto file : files)
//        if (isInterestedInFileDrag(files))
//        {
//            verifyAudioFileFormat(file);
//        }
//}
void Player::verifyAudioFileFormat(const juce::String& path)
{
    juce::File file(path);
    if ((file.getFileExtension() == juce::String(".wav")) || (file.getFileExtension() == juce::String(".WAV")))
    {

        loadFile(file.getFullPathName());
    }
    else
    {
        std::string pathstd = path.toStdString();
        juce::String newpath = startFFmpeg(pathstd);
        std::string newpathstd = newpath.toStdString();
        loadFile(newpathstd);
    }


}

void Player::loadFile(const juce::String& path)
{
    loadedFilePath = path.toStdString();
    juce::File file(path);

    if (juce::AudioFormatReader* reader = formatManager.createReaderFor(file))
    {
        std::unique_ptr<juce::AudioFormatReaderSource> tempSource(new juce::AudioFormatReaderSource(reader, true));

        auto fileName = file.getFileName();
        soundName.setText(fileName, juce::NotificationType::dontSendNotification);

        transport.setSource(tempSource.get());
        transport.addChangeListener(this);

        resampledSource.setResamplingRatio(reader->sampleRate / actualSampleRate);


        //replace temporary source by playing source
        playSource.reset(tempSource.release());

        playButton.setEnabled(true);
        cueButton.setEnabled(true);
        stopButton.setEnabled(true);
        trimVolumeSlider.setValue(0, juce::NotificationType::dontSendNotification);
        thumbnailZoomValue = 1.;

        transportPositionSlider.setEnabled(true);
        transportPositionSlider.setNumDecimalPlacesToDisplay(0);
        transportPositionSlider.setRange(0, transport.getLengthInSeconds());

        thumbnail.setSource(new juce::FileInputSource(file));
        waveformPainted = false;
        fileLoaded = true;
    }
}

const juce::String Player::startFFmpeg(std::string filePath)
{
    //////////*****************Create FFMPEG command Line
    //add double slash to path
    std::string newFilePath = std::regex_replace(filePath, std::regex(R"(\\)"), R"(\\)");
    //give Output Directory
    std::size_t botDirPos = filePath.find_last_of("\\");
    std::string outputFileDirectory = filePath.substr(0, botDirPos);
    //give file name with extension
    std::string fileOutputName = filePath.substr(botDirPos, filePath.length());
    std::string newFileOutputDir = std::regex_replace(outputFileDirectory, std::regex(R"(\\)"), R"(\\)");
    size_t lastindex = fileOutputName.find_last_of(".");
    //give file name without extension and add double dash before
    std::string rawname = fileOutputName.substr(0, lastindex);
    std::string rawnamedoubleslash = std::regex_replace(rawname, std::regex(R"(\\)"), R"(\\)");
    //create entire command string
    std::string cmdstring = std::string("\"C:\\JUCE\\Projects\\MultiPlayer\\ffmpeg.exe\" -i \"" + newFilePath + "\" -ar 48000 -y \"" + newFileOutputDir + rawnamedoubleslash + ".wav\"");
    //convert string to LPSTR
    LPSTR s = const_cast<char*>(cmdstring.c_str());

    ////////////Launch FFMPEG
    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    //launch process
    BOOL logDone = CreateProcessA(NULL, s, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    if (logDone)
    {
        WaitForSingleObject(pi.hProcess, INFINITE);
    }

    /////////////////////return created file path
    std::string returnFilePath = std::string(outputFileDirectory + rawname + ".wav");
    std::string returnFilePathBackslah = std::regex_replace(returnFilePath, std::regex(R"(\\)"), R"(\\)");
    juce::String returnedFile = juce::String(returnFilePathBackslah);

    return returnedFile;
}

void Player::handleAudioTags(std::string filePath)
{

}

std::string Player::getFilePath()
{
    return loadedFilePath;
}


float Player::getTrimVolume()
{
    return trimVolumeSlider.getValue();
}

void Player::setTrimVolume(double trimVolume)
{
    trimVolumeSlider.setValue(trimVolume);
}

bool Player::getIsLooping()
{
    return looping;
}

void Player::setIsLooping(bool isLooping)
{
    looping = isLooping;
    loopButton.setToggleState(looping, juce::NotificationType::dontSendNotification);
}