/*
  ==============================================================================

    Playlist.cpp
    Created: 10 Feb 2021 2:39:14pm
    Author:  Lucien

  ==============================================================================
*/

#include <JuceHeader.h>
#include "Playlist.h"
#include "Settings.h"

//==============================================================================
Playlist::Playlist(int splaylistType)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    playerNumber = 4;
    playlistType = splaylistType;
    //if (playlistType == 1)
        juce::Timer::startTimer(500);


    if (playlistType == 1)
    {
        handleFader3(127);
        handleFader3(0);
        fader1IsPlaying == false;
    }
    //playlistMixer.prepareToPlay(actualSamplesPerBlockExpected, actualSampleRate);

    setSize(getParentWidth(), getParentHeight());

    //addPlayer(1);

    //addKeyListener(this);
    minimumPlayer.addListener(this);
    fader1Value.addListener(this);
    rearrangePlayers();
    updateNextPlayer();
    mouseDragX = 0;
    mouseDragY = 0;
    mouseDragSource = -1 ;
    mouseDraggedUp = -1 ;
    draggedPlayer = -1;

    assignLeftFader(-1);
    assignLeftFader(0);
    assignRightFader(-1);
    assignRightFader(1);

    //DBG(players[1]->importFromNetia("\\WMDRTSV00147742\SONS$\TECHNIQUE\cf68d5d4 - 98c1 - 4502 - adcc - f137d8cb4c62.BWF"));
}

Playlist::~Playlist()
{
    removeMouseListener(this);
    playlistMixer.removeAllInputs();
    playlistCueMixer.removeAllInputs();
    players.clear(true);
    for (auto i = 0; i < players.size(); i++)
        players.remove(i, true);
}

void Playlist::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()


   /* for (auto i = 0; i < playerNumber; ++i)
    {
       players[i]->playerPrepareToPlay(samplesPerBlockExpected, sampleRate);
    }*/

    playlistMixer.prepareToPlay(samplesPerBlockExpected, sampleRate);
    playlistCueMixer.prepareToPlay(samplesPerBlockExpected, sampleRate);
    actualSampleRate = sampleRate;
    actualSamplesPerBlockExpected = samplesPerBlockExpected;


}

void Playlist::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    //bufferToFill.clearActiveBufferRegion();

    //playlistMixer.getNextAudioBlock(bufferToFill);
}

void Playlist::paint(juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)

    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    //Vertical and horizontal lines between players

    //g.drawRect(0, 0, 600, 470);
    for (auto i = 1; i < playerNumber; i++)
    {
            g.setColour(juce::Colour(37, 45, 49));
            g.drawHorizontalLine((playersStartHeightPosition + (i * (playerHeight + spaceBetweenPlayer)) - 4), 0, getParentWidth());
            g.setColour(juce::Colours::black);
            g.drawHorizontalLine((playersStartHeightPosition + (i * (playerHeight + spaceBetweenPlayer)) - 3), 0, getParentWidth());
            //g.drawHorizontalLine((playersStartHeightPosition + (i * (playerHeight + spaceBetweenPlayer)) - 2), 0, getParentWidth());
            g.setColour(juce::Colour(37, 45, 49));
            g.drawHorizontalLine((playersStartHeightPosition + (i * (playerHeight + spaceBetweenPlayer)) - 2), 0, getParentWidth());
    }
    if (playlistType == 0)
    {
        for (auto i = 0; i < players.size(); i++)
        {
            g.setColour(juce::Colour(40, 134, 189));
            g.fillRoundedRectangle(2, ( 10 + (105 * i)), 32, 83, 3);
        }
    }
    else if (playlistType == 1)
    {
        for (auto i = 0; i < players.size(); i++)
        {
            g.setColour(juce::Colour(40, 134, 189));
            g.fillRoundedRectangle(getParentWidth() - 36, (10 + (105 * i)), 32, 83, 3);
        }
    }



    //Paint Rectangle surrounding players when dragging
    if (dragPaintSourceRectangle == true)
    {
        g.setColour(juce::Colour(40, 134, 189));
        //Draw a rectangle surroundig the player :
        //two top lines
        g.drawHorizontalLine((playersStartHeightPosition + (fileDragPlayerSource * (playerHeight + spaceBetweenPlayer)) - 3), 0, getParentWidth());
        g.drawHorizontalLine((playersStartHeightPosition + (fileDragPlayerSource * (playerHeight + spaceBetweenPlayer)) - 2), 0, getParentWidth());
        //g.drawHorizontalLine((playersStartHeightPosition + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) - 1), 0, getParentWidth());
        //two bottoms lines
        g.drawHorizontalLine((playersStartHeightPosition + (playerHeight + spaceBetweenPlayer) + (fileDragPlayerSource * (playerHeight + spaceBetweenPlayer)) - 3), 0, getParentWidth());
        g.drawHorizontalLine((playersStartHeightPosition + (playerHeight + spaceBetweenPlayer) + (fileDragPlayerSource * (playerHeight + spaceBetweenPlayer)) - 2), 0, getParentWidth());
        //two lines at the left
        g.drawVerticalLine(0, playersStartHeightPosition + (fileDragPlayerSource * (playerHeight + spaceBetweenPlayer)) - 2,
            playersStartHeightPosition + (fileDragPlayerSource * (playerHeight + spaceBetweenPlayer)) + playerHeight + 2);
        g.drawVerticalLine(1, playersStartHeightPosition + (fileDragPlayerSource * (playerHeight + spaceBetweenPlayer)) - 2,
            playersStartHeightPosition + (fileDragPlayerSource * (playerHeight + spaceBetweenPlayer)) + playerHeight + 2);
        //two lines at the right
        g.drawVerticalLine(getParentWidth() - 1, playersStartHeightPosition + (fileDragPlayerSource * (playerHeight + spaceBetweenPlayer)) - 2,
            playersStartHeightPosition + (fileDragPlayerSource * (playerHeight + spaceBetweenPlayer)) + playerHeight + 2);
        g.drawVerticalLine(getParentWidth() - 2, playersStartHeightPosition + (fileDragPlayerSource * (playerHeight + spaceBetweenPlayer)) - 2,
            playersStartHeightPosition + (fileDragPlayerSource * (playerHeight + spaceBetweenPlayer)) + playerHeight + 2);


    }
    g.setColour(juce::Colours::red);
    if (fileDragPaintLine == true)
    {
        //Draw two lines at the insert position
        g.drawHorizontalLine((playersStartHeightPosition + (playerHeight + spaceBetweenPlayer) + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) - 4), 0, getParentWidth());
        g.drawHorizontalLine((playersStartHeightPosition + (playerHeight + spaceBetweenPlayer) + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) - 3), 0, getParentWidth());
        g.drawHorizontalLine((playersStartHeightPosition + (playerHeight + spaceBetweenPlayer) + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) - 2), 0, getParentWidth());
        g.drawHorizontalLine((playersStartHeightPosition + (playerHeight + spaceBetweenPlayer) + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) - 1), 0, getParentWidth());
    }
    else if (fileDragPaintRectangle == true && (fileDragPlayerSource != fileDragPlayerDestination))
    {
        //Draw a rectangle surroundig the player :
        //two top lines
        g.drawHorizontalLine((playersStartHeightPosition + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) - 3), 0, getParentWidth());
        g.drawHorizontalLine((playersStartHeightPosition + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) - 2), 0, getParentWidth());
        //g.drawHorizontalLine((playersStartHeightPosition + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) - 1), 0, getParentWidth());
        //two bottoms lines
        g.drawHorizontalLine((playersStartHeightPosition + (playerHeight + spaceBetweenPlayer) + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) - 3), 0, getParentWidth());
        g.drawHorizontalLine((playersStartHeightPosition + (playerHeight + spaceBetweenPlayer) + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) - 2), 0, getParentWidth());
        //two lines at the left
        g.drawVerticalLine(0, playersStartHeightPosition + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) - 2,
            playersStartHeightPosition + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) + playerHeight + 2);
        g.drawVerticalLine(1, playersStartHeightPosition + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) - 2,
            playersStartHeightPosition + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) + playerHeight + 2);
        //two lines at the right
        g.drawVerticalLine(getParentWidth() - 1, playersStartHeightPosition + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) - 2,
            playersStartHeightPosition + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) + playerHeight + 2);
        g.drawVerticalLine(getParentWidth() - 2, playersStartHeightPosition + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) - 2,
            playersStartHeightPosition + (fileDragPlayerDestination * (playerHeight + spaceBetweenPlayer)) + playerHeight + 2);
        if (players[fileDragPlayerDestination] != nullptr)
            g.fillRect(players[fileDragPlayerDestination]->getBounds());
    }
    //dragPaintSourceRectangle

}

void Playlist::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
    for (auto i = 0; i < players.size()+1; i++)
    {
        if (players[i] != nullptr)
        {
            rearrangePlayers();
        }
    }

}

void Playlist::handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message)
{
    int midiMessageValue = message.getControllerValue();
    int midiMessageNumber = message.getControllerNumber();
    if (playlistType == 0)
    {
        if (midiMessageNumber < playerNumber)
        {
            if (midiMessageNumber == 0 + Settings::midiShift)
                handleFader1(midiMessageValue);
            if (midiMessageNumber == 1 + Settings::midiShift)
                handleFader2(midiMessageValue);
        }
    }
    if (playlistType == 1)
    {
        if (midiMessageNumber == 2 + Settings::midiShift)
            handleFader3(midiMessageValue);
        if (midiMessageNumber == 3 + Settings::midiShift)
            handleFader4(midiMessageValue);
    }
    const juce::ScopedValueSetter<bool> scopedInputFlag(isAddingFromMidiInput, true);

}

void Playlist::fader1Start()
{
    players[fader1Player]->transport.setGain(0.0);
    players[fader1Player]->play();
    players[fader1Player]->fader1IsPlaying = true;
    fader1IsPlaying = true;
    fader1Stopped = false;
    fader1StartPlayer = fader1Player;
    updateButtonsStates();
    //fader1PreviousMidiLevel = fader1ActualMidiLevel;
    if (fader2IsPlaying == false)
        fader2Player++;
    //if (fader2Player > (playerNumber - 1))
    //    fader2Player = playerNumber - 1;
    fader1StartTime = juce::Time::currentTimeMillis();
    updateNextPlayer();
}
void Playlist::fader1Stop(bool stoppedByFader)
{
    players[fader1Player]->stopButtonClicked();
    players[fader1Player]->fader1IsPlaying = false;
    fader1IsPlaying = false;
    fader1Stopped = true;
    updateButtonsStates();
    fader1PreviousMidiLevel = fader1ActualMidiLevel;
    fader1StopTime = juce::Time::currentTimeMillis();
    if (fader1StopTime - fader1StartTime >= Settings::faderTempTime)
    {
        if (fader2IsPlaying == false)
        {
            fader2Player = std::max(fader1Player, fader2Player);
            fader1Player = std::max(fader1Player, fader2Player);
        }
        else if (fader2IsPlaying == true)
        {
            fader1Player = std::max(fader1Player, fader2Player) + 1;
        }
    }
    else if (fader2IsPlaying == false)
    {
        fader2Player--;
        if (fader1StopTime - fader2StartTime < Settings::faderTempTime)
        {
            fader2Player = std::max(fader2Player, fader1Player);
        }
    }
    updateNextPlayer();
}
void Playlist::fader2Start()
{
    players[fader2Player]->transport.setGain(0.0);
    players[fader2Player]->play();
    players[fader2Player]->fader2IsPlaying = true;
    fader2IsPlaying = true;
    fader2StartPlayer = fader2Player;
    updateButtonsStates();
    fader2Stopped = false;
    fader2StartTime = juce::Time::currentTimeMillis();
    //fader2PreviousMidiLevel = fader2ActualMidiLevel;
    if (fader1IsPlaying == false)
        fader1Player++;
    //if (fader1Player > (playerNumber - 1))
    //    fader1Player = playerNumber - 1;
    updateNextPlayer();
}
void Playlist::fader2Stop(bool stoppedByFader)
{
    players[fader2Player]->stopButtonClicked();
    players[fader2Player]->fader2IsPlaying = false;
    fader2IsPlaying = false;
    updateButtonsStates();
    fader2PreviousMidiLevel = fader2ActualMidiLevel;
    fader2StopTime = juce::Time::currentTimeMillis();
    if (fader2StopTime - fader2StartTime >= Settings::faderTempTime)
    {
        if (fader1IsPlaying == false)
        {
            fader1Player = std::max(fader2Player, fader1Player);
            fader2Player = std::max(fader2Player, fader1Player);
        }
        else if (fader1IsPlaying == true)
        {
            fader2Player = std::max(fader2Player, fader1Player) + 1;
        }
    }
    else if (fader1IsPlaying == false)
    {
        fader1Player--;
        if (fader2StopTime - fader1StartTime < Settings::faderTempTime)
        {
            fader1Player = std::max(fader2Player, fader1Player);
        }
    }
    updateNextPlayer();
}

void Playlist::handleFader1(int faderValue)
{
    fader1ActualMidiLevel = faderValue;
    if (spaceBarIsPlaying == false)
    {
        if (players[fader1Player] != nullptr)
        {

                if (players[fader1Player]->fileLoaded != 0) //if there is a loaded file
                {

                    if (fader1IsPlaying == false) //if it is not playing
                    {
                        if (fader1ActualMidiLevel >= 1 && fader1PreviousMidiLevel == 0) //fader start
                        {

                            fader1Start();
                        }
                    }
                    if (fader1IsPlaying == true) //if it is playing
                    {
                        if (fader1ActualMidiLevel == 0 && fader1PreviousMidiLevel >= 1) //fader stop
                        {
                            players[fader1Player]->transport.stop();
                            //fader1IsFlying = false;
                        }
                    }
                }
                fader1PreviousMidiLevel = fader1ActualMidiLevel;
                //if (!fader1IsFlying)
                //{
                if (players[fader1Player] != nullptr && fader1IsPlaying)
                    players[fader1Player]->handleMidiMessage(0, faderValue);
                //}

        }
    }
}

void Playlist::handleFader1OSC(float faderValue)
{
    fader1ActualMidiLevel = juce::int32(faderValue*127);
    if (spaceBarIsPlaying == false)
    {
        if (players[fader1Player] != nullptr)
        {
            if (players[fader1Player]->fileLoaded != 0) //if there is a loaded file
            {
                if (fader1IsPlaying == false) //if it is not playing
                {
                    if (fader1ActualMidiLevel >= 1 && fader1PreviousMidiLevel == 0) //fader start
                    {
                        fader1Start();
                    }
                }
                if (fader1IsPlaying == true) //if it is playing
                {
                    if (fader1ActualMidiLevel == 0 && fader1PreviousMidiLevel >= 1) //fader stop
                    {
                        fader1Stop(true);
                        fader1Stopped = false;
                    }
                }
            }
            if (players[fader1Player] != nullptr)
                players[fader1Player]->handleOSCMessage(faderValue);

        }
    }
}

void Playlist::handleFader2(int faderValue)
{
    if (spaceBarIsPlaying == false)
    {
        fader2ActualMidiLevel = faderValue;
        if (players[fader2Player] != nullptr)
        {
            if (players[fader2Player]->fileLoaded != 0) //if there is a loaded file
            {
                if (fader2IsPlaying == false) //if it is not playing
                {
                    if (fader2ActualMidiLevel >= 1 && fader2PreviousMidiLevel == 0) //fader start
                    {
                        fader2Start();
                    }
                }
                if (fader2IsPlaying == true) //if it is playing
                {
                    if (fader2ActualMidiLevel == 0 && fader2PreviousMidiLevel >= 1) //fader stop
                    {
                        players[fader2Player]->transport.stop();
                        //fader2Stopped = false;
                    }
                }
            }
            fader2PreviousMidiLevel = fader2ActualMidiLevel;
            if (players[fader2Player] != nullptr && fader2IsPlaying)
                players[fader2Player]->handleMidiMessage(0, faderValue);
        }
    }
}

void Playlist::handleFader2OSC(float faderValue)
{
    fader2ActualMidiLevel = juce::int32(faderValue * 127);
    if (spaceBarIsPlaying == false)
    {
        if (players[fader2Player] != nullptr)
        {
            if (players[fader2Player]->fileLoaded != 0) //if there is a loaded file
            {
                if (fader2IsPlaying == false) //if it is not playing
                {
                    if (fader2ActualMidiLevel >= 1 && fader2PreviousMidiLevel == 0) //fader start
                    {
                        fader2Start();
                    }
                }
                if (fader2IsPlaying == true) //if it is playing
                {
                    if (fader2ActualMidiLevel == 0 && fader2PreviousMidiLevel >= 1) //fader stop
                    {
                        fader2Stop(true);
                    }

                }

            }
            if (players[fader2Player] != nullptr)
                players[fader2Player]->handleOSCMessage(faderValue);
        }
    }
}

void Playlist::handleFader3(int faderValue)
{
    fader1ActualMidiLevel = faderValue;
        if (players[fader1Player] != nullptr)
        {
            if (players[fader1Player]->fileLoaded != 0) //if there is a loaded file
            {
                if (fader1IsPlaying == false) //if it is not playing
                {
                    if (fader1ActualMidiLevel >= 1 && fader1PreviousMidiLevel == 0) //fader start
                    {
                            players[fader1Player]->play();
                            players[fader1Player]->fader1IsPlaying = true;
                            fader1IsPlaying = true;

                    }
                }
                if (fader1IsPlaying == true) //if it is playing
                {
                    if (fader1ActualMidiLevel == 0 && fader1PreviousMidiLevel >= 1) //fader stop
                    {
                        players[fader1Player]->stopButtonClicked();
                        players[fader1Player]->fader1IsPlaying = false;
                        fader1IsPlaying = false;
                        //fader1IsFlying = false;
                    }

                }

            }
            fader1PreviousMidiLevel = fader1ActualMidiLevel;
            if (players[fader1Player] != nullptr)
                players[fader1Player]->handleMidiMessage(0, faderValue);
        }
}

void Playlist::handleFader3OSC(float faderValue)
{
    fader1ActualMidiLevel = juce::int32(faderValue * 127);
    if (players[fader1Player] != nullptr)
    {
        if (players[fader1Player]->fileLoaded != 0) //if there is a loaded file
        {
            if (fader1IsPlaying == false) //if it is not playing
            {
                if (fader1ActualMidiLevel >= 1 && fader1PreviousMidiLevel == 0) //fader start
                {
                    players[fader1Player]->play();
                    players[fader1Player]->fader1IsPlaying = true;
                    fader1IsPlaying = true;
                    fader1PreviousMidiLevel = fader1ActualMidiLevel;
                }
            }
            if (fader1IsPlaying == true) //if it is playing
            {
                if (fader1ActualMidiLevel == 0 && fader1PreviousMidiLevel >= 1) //fader stop
                {
                    players[fader1Player]->stopButtonClicked();
                    players[fader1Player]->fader1IsPlaying = false;
                    fader1IsPlaying = false;
                    fader1PreviousMidiLevel = fader1ActualMidiLevel;
                }

            }

        }
        if (players[fader1Player] != nullptr)
            players[fader1Player]->handleOSCMessage(faderValue);
    }
}

void Playlist::handleFader4(int faderValue)
{
    fader2ActualMidiLevel = faderValue;
    if (players[fader2Player] != nullptr)
    {
        if (players[fader2Player]->fileLoaded != 0) //if there is a loaded file
        {
            if (fader2IsPlaying == false) //if it is not playing
            {
                if (fader2ActualMidiLevel >= 1 && fader2PreviousMidiLevel == 0) //fader start
                {
                    players[fader2Player]->play();
                    players[fader2Player]->fader2IsPlaying = true;
                    fader2IsPlaying = true;
                    fader2PreviousMidiLevel = fader2ActualMidiLevel;

                }
            }
            if (fader2IsPlaying == true) //if it is playing
            {
                if (fader2ActualMidiLevel == 0 && fader2PreviousMidiLevel >= 1) //fader stop
                {
                    players[fader2Player]->stopButtonClicked();
                    players[fader2Player]->fader2IsPlaying = false;
                    fader2IsPlaying = false;
                    fader2PreviousMidiLevel = fader2ActualMidiLevel;
                }

            }

        }
        if (players[fader2Player] != nullptr)
            players[fader2Player]->handleMidiMessage(0, faderValue);
    }
}

void Playlist::handleFader4OSC(float faderValue)
{
    fader2ActualMidiLevel = juce::int32(faderValue * 127);
    if (players[fader2Player] != nullptr)
    {
        if (players[fader2Player]->fileLoaded != 0) //if there is a loaded file
        {
            if (fader2IsPlaying == false) //if it is not playing
            {
                if (fader2ActualMidiLevel >= 1 && fader2PreviousMidiLevel == 0) //fader start
                {
                    players[fader2Player]->play();
                    players[fader2Player]->fader2IsPlaying = true;
                    fader2IsPlaying = true;
                    fader2PreviousMidiLevel = fader2ActualMidiLevel;

                }
            }
            if (fader2IsPlaying == true) //if it is playing
            {
                if (fader2ActualMidiLevel == 0 && fader2PreviousMidiLevel >= 1) //fader stop
                {
                    players[fader2Player]->stopButtonClicked();
                    players[fader2Player]->fader2IsPlaying = false;
                    fader2IsPlaying = false;
                    fader2PreviousMidiLevel = fader2ActualMidiLevel;
                }

            }

        }
        if (players[fader2Player] != nullptr)
            players[fader2Player]->handleOSCMessage(faderValue);
    }
}

void Playlist::updateNextPlayer()
{
    if (players[fader1Player] != nullptr)
        fader1Name.setValue(players[fader1Player]->soundName.getText());
    if (players[fader2Player] != nullptr)
        fader2Name.setValue(players[fader2Player]->soundName.getText());

    minimumPlayer = std::min(fader1Player, fader2Player);
    if (playlistType == 0)
    {
        if (fader1Player == fader2Player)
            nextPlayer = fader1Player;
        else if (fader1IsPlaying && fader2IsPlaying)
            nextPlayer = -1;
        else if (fader1IsPlaying || fader2IsPlaying)
            nextPlayer = std::max(fader1Player, fader2Player);

        for (auto i = 0; i < playerNumber; i++)
        {
            if (players[i] != nullptr)
            {
                if (nextPlayer == i)
                    players[i]->setNextPlayer(true);
                else if (nextPlayer != i)
                    players[i]->setNextPlayer(false);
                if (playlistType == 0)
                {
                    const juce::MessageManagerLock mmLock;
                    if (i == fader1Player)
                        players[i]->setLeftFaderAssigned(true);
                    else
                        players[i]->setLeftFaderAssigned(false);
                    if (i == fader2Player)
                        players[i]->setRightFaderAssigned(true);
                    else
                        players[i]->setRightFaderAssigned(false);
                }
            }

        }

        spaceBarPlayerId = nextPlayer;
    }

    const juce::MessageManagerLock mmLock;
    repaint();
}

void Playlist::playersResetPositionClicked()
{
    if ((fader1IsPlaying == 0) && (fader2IsPlaying == 0))
    {
        fader1Player = 0;
        fader2Player = 0;
        updateNextPlayer();

    }
}

void Playlist::playersPreviousPositionClicked()
{
    if ((fader1IsPlaying == 0) && (fader2IsPlaying == 0))
    {
        nextPlayer = (std::min(fader1Player, fader2Player) - 1);
        fader1Player = fader2Player = nextPlayer;
    }
    else if ((fader2IsPlaying == 0) && (fader2Player > (fader1Player + 1)))
    {
        nextPlayer = fader2Player - 1;
        fader2Player = nextPlayer;
    }
    else if ((fader1IsPlaying == 0) && (fader1Player > (fader2Player + 1)))
    {
        nextPlayer = fader1Player - 1;
        fader1Player = nextPlayer;
    }

    if (fader1Player < 0)
        fader1Player = 0;
    if (fader2Player < 0)
        fader2Player = 0;

    updateNextPlayer();
}

void Playlist::playersNextPositionClicked()
{
    if (nextPlayer < playerNumber)
        nextPlayer++;
    if ((fader1IsPlaying == 0) && (fader2IsPlaying == 0))
    {
        fader1Player = nextPlayer;
        fader2Player = nextPlayer;
    }
    else if (fader2IsPlaying == 0)
        fader2Player = nextPlayer;
    else if (fader1IsPlaying == 0)
        fader1Player = nextPlayer;

    if (fader1Player == players.size())
        fader1Player = (players.size() - 1);
    if (fader2Player == players.size())
        fader2Player = (players.size() - 1);
    setOptions();
    updateNextPlayer();
}

void Playlist::addPlayer(int playerID)
{

    if (playlistType == 0)
    {

    }
    if (playlistType == 1)
    {
       /* if (playerID < fader1Player)
            fader1Player++;
        if (playerID < fader2Player)
            fader2Player++;*/
    }
    int idAddedPlayer = playerID + 1;
    players.insert(idAddedPlayer, new Player(idAddedPlayer));
    players[idAddedPlayer]->playerPrepareToPlay(actualSamplesPerBlockExpected, actualSampleRate);
    playlistMixer.addInputSource(&players[idAddedPlayer]->mixer, false);
    playlistCueMixer.addInputSource(&players[idAddedPlayer]->cueMixer, false);
    players[idAddedPlayer]->fileName.addListener(this);
    players[idAddedPlayer]->playerPositionLabel.addListener(this);
    players[idAddedPlayer]->draggedPlayer.addListener(this);
    players[idAddedPlayer]->cueStopped.addListener(this);
    players[idAddedPlayer]->transport.addChangeListener(this);
    players[idAddedPlayer]->cueTransport.addChangeListener(this);

    playersPositionLabels.insert(idAddedPlayer, new juce::Label);
    playersPositionLabels[idAddedPlayer]->setText(juce::String(idAddedPlayer + 1), juce::NotificationType::dontSendNotification);
    addAndMakeVisible(playersPositionLabels[idAddedPlayer]);
    playersPositionLabels[idAddedPlayer]->setInterceptsMouseClicks(false, false);
  
    if (playlistType == 0)
    {
        removePlayersButtons.insert(idAddedPlayer, new juce::TextButton());
        removePlayersButtons[idAddedPlayer]->setButtonText("-");
        removePlayersButtons[idAddedPlayer]->onClick = [idAddedPlayer, this] { removePlayer(idAddedPlayer); };

        addPlayersButtons.insert(idAddedPlayer, new juce::TextButton());
        addPlayersButtons[idAddedPlayer]->setButtonText("+");
        addPlayersButtons[idAddedPlayer]->onClick = [idAddedPlayer, this] { addPlayer(idAddedPlayer); };

        playersPositionLabels[idAddedPlayer]->setBounds(2, (105 * idAddedPlayer) + 30, 40, 40);
        playersPositionLabels[idAddedPlayer]->setFont(juce::Font(40.00f, juce::Font::plain).withTypefaceStyle("Regular"));
    }
    else if (playlistType == 1)
    {
        assignLeftFaderButtons.insert(idAddedPlayer, new juce::TextButton());
        assignLeftFaderButtons[idAddedPlayer]->setButtonText("");
        assignLeftFaderButtons[idAddedPlayer]->onClick = [idAddedPlayer, this] { assignLeftFader(idAddedPlayer); };

        assignRightFaderButtons.insert(idAddedPlayer, new juce::TextButton());
        assignRightFaderButtons[idAddedPlayer]->setButtonText("");
        assignRightFaderButtons[idAddedPlayer]->onClick = [idAddedPlayer, this] { assignRightFader(idAddedPlayer); };

        playersPositionLabels[idAddedPlayer]->setBounds(getParentWidth() - 36, (105 * idAddedPlayer) + 30, 40, 40);
        playersPositionLabels[idAddedPlayer]->setFont(juce::Font(40.00f, juce::Font::plain).withTypefaceStyle("Regular"));

      
    }
    playerNumber = players.size();
    rearrangePlayers();
    updateNextPlayer();
    setOptions();
    repaint();
}

void Playlist::removePlayer(int playerID)
{
    if (playlistType == 0)
    {
            if (players.size() > 0)
            {
                if (players[playerID] != nullptr 
                 /*  && !players[playerID]->fader1IsPlaying
                    && !players[playerID]->fader2IsPlaying
                    && !players[playerID]->keyIsPlaying*/)
                {
                    players[playerID]->stopButtonClicked();
                    playlistMixer.removeInputSource(&players[playerID]->mixer);
                    playlistCueMixer.removeInputSource(&players[playerID]->cueMixer);
                    players[playerID]->fileName.removeListener(this);
                    players[playerID]->playerPositionLabel.addListener(this);
                    players[playerID]->cueStopped.addListener(this);
                    players.remove(playerID);
                    playersPositionLabels.remove(playerID);
                    //swapNextButtons.remove(playerID);
                    removePlayersButtons.remove(playerID);
                    addPlayersButtons.remove(playerID);
                    playerNumber = players.size();
                    updateNextPlayer();
                    rearrangePlayers();
            }
        }
    }
    else if (playlistType == 1)
    {
            if (players.size() > 0)
            {
                if (players[playerID] != nullptr)
                {
                    players[playerID]->stopButtonClicked();
                    playlistMixer.removeInputSource(&players[playerID]->mixer);
                    playlistCueMixer.removeInputSource(&players[playerID]->cueMixer);
                    players[playerID]->fileName.removeListener(this);
                    players[playerID]->playerPositionLabel.addListener(this);
                    players[playerID]->cueStopped.addListener(this);
                    players.remove(playerID);
                    playersPositionLabels.remove(playerID);
                    assignLeftFaderButtons.remove(playerID);
                    assignRightFaderButtons.remove(playerID);
                    playerNumber = players.size();
                    updateNextPlayer();
                    rearrangePlayers();
                }
            }
        
    }
}

void Playlist::swapNext(int playerID)
{
    players.swap(playerID, playerID + 1);
    updateNextPlayer();
    rearrangePlayers();
}

void Playlist::assignLeftFader(int playerID)
{
    int previousfader1Player = fader1Player;
    if (players[playerID] != nullptr)
    {
        if (fader1IsPlaying == false)
        {
            if (fader2Player != playerID)
            {
                fader1Player = playerID;
                players[playerID]->setLeftFaderAssigned(true);
            }
            else if (fader2Player == playerID)
            {
                if (fader2IsPlaying)
                {
                    fader1Player = previousfader1Player;
                    players[playerID]->setLeftFaderAssigned(true);
                }
                else if (!fader2IsPlaying)
                {
                    fader1Player = playerID;
                    fader2Player = -1;
                    assignRightFaderButtons[playerID]->setColour(juce::TextButton::ColourIds::buttonColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
                    players[playerID]->setRightFaderAssigned(false);
                    players[playerID]->setLeftFaderAssigned(true);
                }
            }
        }
        for (auto i = 0; i < players.size(); i++)
        {
            if (i != fader1Player)
            {
                assignLeftFaderButtons[i]->setColour(juce::TextButton::ColourIds::buttonColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
            }
            else
                assignLeftFaderButtons[i]->setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::red);
        }
    }
    if (players[fader1Player] != nullptr)
        fader1Name.setValue(players[fader1Player]->soundName.getText());
    else
        fader1Name.setValue("");
}

void Playlist::assignRightFader(int playerID)
{
    int previousfader2Player = fader2Player;
    if (players[playerID] != nullptr)
    {
        if (fader2IsPlaying == false)
        {
            if (fader1Player != playerID)
            {
                fader2Player = playerID;
                players[playerID]->setRightFaderAssigned(true);
            }
            else if (fader1Player == playerID)
            {
                if (fader1IsPlaying)
                {
                    fader2Player = previousfader2Player;
                    players[playerID]->setRightFaderAssigned(true);
                }
                else if (!fader1IsPlaying)
                {
                    fader2Player = playerID;
                    fader1Player = -1;
                    assignLeftFaderButtons[playerID]->setColour(juce::TextButton::ColourIds::buttonColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
                    players[playerID]->setLeftFaderAssigned(false);
                    players[playerID]->setRightFaderAssigned(true);
                }
            }
        }
        for (auto i = 0; i < players.size(); i++)
        {
            if (i != fader2Player)
            {
                assignRightFaderButtons[i]->setColour(juce::TextButton::ColourIds::buttonColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
            }
            else
                assignRightFaderButtons[i]->setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::red);
        }

    }
    if (players[fader2Player] != nullptr)
        fader2Name.setValue(players[fader2Player]->soundName.getText());
    else
        fader2Name.setValue("");
}

void Playlist::rearrangePlayers()
{
    playerNumber = players.size();

    for (auto i = 0; i < players.size() + 1; i++)
    {
        if (players[i] != nullptr)
        {
            //addAndMakeVisible(playersPositionLabels[i]);
            playersPositionLabels[i]->setText(juce::String(i + 1), juce::NotificationType::dontSendNotification);
            if (i > 8)
                playersPositionLabels[i]->setFont(juce::Font(30.00f, juce::Font::plain).withTypefaceStyle("Regular"));

            // Player
            addAndMakeVisible(players[i]);
            if (playlistType == 1)
            {
                playersPositionLabels[i]->setBounds(getParentWidth() - 36, (105 * i) + 30, 40, 40);
                if (i > 8)
                    playersPositionLabels[i]->setBounds(getParentWidth() - 41, (105 * i) + 30, 40, 40);


                players[i]->setBounds(assignFaderButtonWidth, playersStartHeightPosition + (i * (playerHeight + spaceBetweenPlayer)), getParentWidth() - (assignFaderButtonWidth*2) - dragZoneWidth , playerHeight);
                players[i]->isCart = true;
                players[i]->resized();
                addAndMakeVisible(assignLeftFaderButtons[i]);
                assignLeftFaderButtons[i]->setBounds(1, 33 + playersStartHeightPosition + (i * (playerHeight + spaceBetweenPlayer)), assignFaderButtonWidth - 1, controlButtonHeight + spaceBetweenPlayer + 2);
                assignLeftFaderButtons[i]->setButtonText("");
                assignLeftFaderButtons[i]->onClick = [i, this] { assignLeftFader(i); };

                addAndMakeVisible(assignRightFaderButtons[i]);
                assignRightFaderButtons[i]->setBounds(getParentWidth() - assignFaderButtonWidth - dragZoneWidth, 33 + playersStartHeightPosition + (i * (playerHeight + spaceBetweenPlayer)), assignFaderButtonWidth, controlButtonHeight + spaceBetweenPlayer + 2);
                assignRightFaderButtons[i]->setButtonText("");
                assignRightFaderButtons[i]->onClick = [i, this] { assignRightFader(i); };

                players[i]->transport.addChangeListener(this);
                players[i]->setLabelPlayerPosition(i);
                setSize(getParentWidth(), ((playerHeight + spaceBetweenPlayer) * (playerNumber)));
            }
               
            // Swap Buttonns
            else if (playlistType == 0)
            {
                playersPositionLabels[i]->setBounds(2, (105 * i) + 30, 40, 40);
                if (i > 8)
                    playersPositionLabels[i]->setBounds(-3, (105 * i) + 30, 40, 40);
                controlButtonXStart = getParentWidth() - controlButtonWidth;
                players[i]->setBounds(dragZoneWidth, playersStartHeightPosition + (i * (playerHeight + spaceBetweenPlayer)), controlButtonXStart - dragZoneWidth, playerHeight);
                players[i]->loopButton.setVisible(false);
                if (i < (playerNumber - 1))
                {
                    //addAndMakeVisible(swapNextButtons[i]);
                    //swapNextButtons[i]->setBounds(controlButtonXStart, 16 + playersStartHeightPosition + ((i * (playerHeight + spaceBetweenPlayer)) + (2 * controlButtonHeight)), controlButtonWidth, controlButtonHeight + spaceBetweenPlayer + 2);
                    //swapNextButtons[i]->setButtonText("s");
                    //swapNextButtons[i]->onClick = [i, this] { swapNext(i); };
                }
                players[i]->resized();
                // Remove Buttons
                addAndMakeVisible(removePlayersButtons[i]);
                removePlayersButtons[i]->setBounds(controlButtonXStart, 16 + playersStartHeightPosition + ((i * (playerHeight + spaceBetweenPlayer))), controlButtonWidth - 3, controlButtonHeight);
                removePlayersButtons[i]->onClick = [i, this] { removePlayer(i); };

                // Add Buttons
                addAndMakeVisible(addPlayersButtons[i]);
                addPlayersButtons[i]->setBounds(controlButtonXStart, 16 + playersStartHeightPosition + ((i * (playerHeight + spaceBetweenPlayer))) + controlButtonHeight, controlButtonWidth - 3, controlButtonHeight);
                addPlayersButtons[i]->setButtonText("+");
                addPlayersButtons[i]->onClick = [i, this] { addPlayer(i); };
                players[i]->transport.addChangeListener(this);
                setSize(getParentWidth(), ((playerHeight + spaceBetweenPlayer) * (playerNumber)));
                updateButtonsStates();
                players[i]->setLabelPlayerPosition(i);
                //players[i]->volumeSliderValue.addListener(this);


            }

        }
    }
    updateNextPlayer();

    //setSize(715, getParentHeight());

}

void Playlist::updateButtonsStates()
{
    const juce::MessageManagerLock mmLock;
    for (auto i = 0; i < playerNumber; i++)
    {
        if (players[i] != nullptr)
        {
            if (fader1IsPlaying == true
                || fader2IsPlaying == true
                || spaceBarIsPlaying == true)
            {
                addPlayersButtons[i]->setEnabled(false);
                removePlayersButtons[i]->setEnabled(false);
                //if (fader1Player == i || fader2Player == i)
                //{
                //    swapNextButtons[i]->setEnabled(false);
                //    if (players[i - 1] != nullptr)
                //        swapNextButtons[i - 1]->setEnabled(false);
                //}
            }
            else
            {
                //swapNextButtons[i]->setEnabled(true);
                removePlayersButtons[i]->setEnabled(true);
                addPlayersButtons[i]->setEnabled(true);
            }

            //if (i == (players.size() - 1))
            //    swapNextButtons[i]->setEnabled(false);
        }

        
    }
}

void Playlist::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    for (auto i = 0; i < playerNumber; i++)
    {
        if (players[i] != nullptr)
        {
            if (source == &players[i]->transport)
            {
                if (players[i]->transport.isPlaying())
                {
                    players[i]->isPlaying = true;
                }
                else
                {
                    playerStoppedID = i;
                    players[i]->keyIsPlaying = false;
                    if (playlistType == 0)
                    {
                        if (i == fader1Player)
                        {
                            fader1IsFlying = true;
                            fader1Stop(true);
                        }
                        else if (i == fader2Player)
                        {
                            fader2IsFlying = true;
                            fader2Stop(true);
                        }
                    }
                    if (playlistType == 1)
                    {
                        if (i == fader1Player)
                        {
                            fader1IsPlaying = false;
                        }
                        if (i == fader2Player)
                            fader2IsPlaying = false;
                    }
                    if (i == fader1Player && fader1IsPlaying == false)
                    {
                        playerStoppedID = i;
                        spaceBarStop();
                        players[i]->isPlaying = false;
                        updateNextPlayer();
                    }
                    else if (i == fader2Player && fader2IsPlaying == false)
                    {
                        playerStoppedID = i;
                        spaceBarStop();
                        players[i]->isPlaying = false;
                        updateNextPlayer();
                    }
                }
            }
        }
    }
}

bool Playlist::keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent)
{



    if (playlistType == 0)
    {
        if (key == juce::KeyPress::spaceKey)
        {

            //spaceBarPressed();
            return true;
        }
        else if (key == 67)
        {
            if (players[draggedPlayer.toString().getIntValue()] != nullptr)
            {
                players[draggedPlayer.toString().getIntValue()]->cueButtonClicked();
            }
        }
        //else if (key == juce::KeyPress::escapeKey)
        //{
        //    if (spaceBarIsPlaying == false)
        //        playersResetPositionClicked();
        //}
    }
    else if (playlistType == 1)
    {
        /*if (key == juce::KeyPress::F1Key)
        {
            playPlayer(0);
        }
        else if (key == juce::KeyPress::F2Key)
        {
            playPlayer(1);
        }
        else if (key == juce::KeyPress::F3Key)
        {
            playPlayer(2);
        }
        else if (key == juce::KeyPress::F4Key)
        {
            playPlayer(3);
        }
        else if (key == juce::KeyPress::F5Key)
        {
            playPlayer(4);
        }
        else if (key == juce::KeyPress::F6Key)
        {
            playPlayer(5);
        }
        else if (key == juce::KeyPress::F7Key)
        {
            playPlayer(6);
        }
        else if (key == juce::KeyPress::F8Key)
        {
            playPlayer(7);
        }
        else if (key.getModifiers() == juce::ModifierKeys::ctrlModifier && key == juce::KeyPress::spaceKey)
        {
            if (players[draggedPlayer.toString().getIntValue()] != nullptr)
            {
                players[draggedPlayer.toString().getIntValue()]->cueButtonClicked();
            }
        }
        else if (key == 67)
        {
            if (players[draggedPlayer.toString().getIntValue()] != nullptr)
            {
                players[draggedPlayer.toString().getIntValue()]->cueButtonClicked();
            }
        }*/

    }


    return false;
}

void Playlist::playPlayer(int playerID)
{
    if (players[playerID] != nullptr)
        if (!players[playerID]->transport.isPlaying() && players[playerID]->fileLoaded)
        {
            players[playerID]->playButtonClicked();
            players[playerID]->keyIsPlaying = true;
        }
        else if (players[playerID]->transport.isPlaying())
        {
            players[playerID]->stopButtonClicked();
            players[playerID]->keyIsPlaying = false;
        }
}

void Playlist::spaceBarPressed()
{
    if ((fader1IsPlaying == false) && (fader2IsPlaying == false))
    {
        if (players[nextPlayer] != nullptr)
        {
            if (spaceBarIsPlaying == false)
            {
                if (players[nextPlayer]->fileLoaded == true)
                {
                    if (players[nextPlayer]->transport.isPlaying())
                    { 
                        players[nextPlayer]->stopButtonClicked();
                    }
                    else
                    {
                        players[nextPlayer]->spaceBarPlay();
                        players[nextPlayer]->fader1IsPlaying = true;
                        players[nextPlayer]->fader2IsPlaying = true;
                        spaceBarIsPlaying = true;
                        spaceBarPlayerId = nextPlayer;
                        updateButtonsStates();
                    }
                }
            }
            else if (spaceBarIsPlaying == true)
            {
                spaceBarStop();
                updateButtonsStates();
            }
        }
    }
}

void Playlist::spaceBarStop()
{

    if (spaceBarIsPlaying == true)
    {
        if (players[spaceBarPlayerId] != nullptr)
        {
            players[spaceBarPlayerId]->stopButtonClicked();
            players[nextPlayer]->fader1IsPlaying = false;
            players[nextPlayer]->fader2IsPlaying = false;
            spaceBarPlayerId++;
            fader1Player++;
            fader2Player++;
            if (spaceBarPlayerId == players.size())
            {
                spaceBarPlayerId--;
                fader1Player--;
                fader2Player--;
            }
            //while ((players[spaceBarPlayerId] != nullptr) && (players[spaceBarPlayerId]->fileLoaded == false))
            //{
            //    spaceBarPlayerId++;
            //    fader1Player++;
            //    fader2Player++;
            //    spaceBarIsPlaying = false;
            //}
            updateNextPlayer();
        }
        spaceBarIsPlaying = false;
    }
    spaceBarIsPlaying = false;

}

bool Playlist::isInterestedInFileDrag(const juce::StringArray& files)
{
    for (auto file : files)

        if ((file.contains(".wav")) || (file.contains(".WAV"))
            || (file.contains(".bwf")) || (file.contains(".BWF"))
            || (file.contains(".aiff")) || (file.contains(".AIFF"))
            || (file.contains(".aif")) || (file.contains(".AIF"))
            || (file.contains(".flac")) || (file.contains(".FLAC"))
            || (file.contains(".opus")) || (file.contains(".OPUS"))
            || (file.contains(".mp3")) || (file.contains(".MP3")))
        {
            return true;
        }
    return false;
}

void Playlist::filesDropped(const juce::StringArray& files, int x, int y)
{
    int i = fileDragPlayerDestination;
    for (auto file : files)
    {
        if (isInterestedInFileDrag(files))
        {
            //DBG(files.size());
            if (files.size() > 1)
            {
                
                addPlayer(i);
                players[i + 1]->verifyAudioFileFormat(file);
                //if (fileDragPaintRectangle == true)
                    //removePlayer(fileDragPlayerDestination);
                i++;
            }
            else if (fileDragPaintLine == true)
            {
                addPlayer(fileDragPlayerDestination);
                players[fileDragPlayerDestination + 1]->verifyAudioFileFormat(file);
                if (i < fader1Player)
                    fader1Player++;
                if (i < fader2Player)
                    fader2Player++;
                if (i < spaceBarPlayerId)
                    spaceBarPlayerId++;

            }
            else if (fileDragPaintRectangle == true)
            {
                players[fileDragPlayerDestination]->verifyAudioFileFormat(file);
            }
            updateNextPlayer();
        }
    }
    if ((files.size() > 1) && fileDragPaintRectangle == true)
        removePlayer(fileDragPlayerDestination);
    fileDragPaintRectangle = false;
    fileDragPaintLine = false;
    repaint();
}

void Playlist::fileDragEnter(const juce::StringArray& files, int x, int y)
{
    for (auto file : files)
    {

    }
}
void Playlist::fileDragMove(const juce::StringArray& files, int x, int y)
{

    if (x < getParentWidth())
    {
        for (auto i = 0; i < playerNumber; i++)
        {
            if ((playlistType == 0)|| (playlistType == 1))
            {
                if (y < (playersStartHeightPosition + playerInsertDragZoneHeight))
                {
                    //Insert at the top
                    fileDragPlayerDestination = -1;
                    fileDragPaintRectangle = false;
                    fileDragPaintLine = true;
                }
                else if ((y > (playersStartHeightPosition + (i * (playerHeight + spaceBetweenPlayer)) + playerHeight - playerInsertDragZoneHeight))
                    && (y < (playersStartHeightPosition + (i * (playerHeight + spaceBetweenPlayer)) + playerHeight + playerInsertDragZoneHeight)))
                {
                    //Insert between players
                    fileDragPlayerDestination = i;
                    fileDragPaintRectangle = false;
                    fileDragPaintLine = true;
                }
                else if ((y > (playersStartHeightPosition + playerInsertDragZoneHeight + (i * (playerHeight + spaceBetweenPlayer))))
                    && (y < (playersStartHeightPosition - playerInsertDragZoneHeight + playerHeight + (i * (playerHeight + spaceBetweenPlayer)))))
                {
                    //replace players
                    fileDragPlayerDestination = i;
                    if (players[fileDragPlayerDestination]->fileLoaded == false)
                        fileDragPaintRectangle = true;
                    fileDragPaintLine = false;
                }
            }
        }
    }
    else if (x > playerWidth)
    {
        fileDragPaintRectangle = false;
        fileDragPaintLine = false;
    }
    repaint();
}

void Playlist::fileDragExit()
{
    fileDragPaintRectangle = false;
    fileDragPaintLine = false;
    repaint();
}

void Playlist::setOptions()
{
    for (auto i = 0; i < playerNumber; i++)
    {
        if (players[i] != nullptr)
        {
            players[i]->setOptions();
        }   
    }
}

void Playlist::setMidiShift(int MidiShift)
{
    midiChannelShift = MidiShift;
}

void Playlist::valueChanged(juce::Value& value)
{
    if (players[fader1Player] != nullptr)
    {
        if (value.refersToSameSourceAs(players[fader1Player]->fileName))
        {
            fader1Name.setValue(players[fader1Player]->soundName.getText());
        }
    }
    else
        fader1Name.setValue("");

    if (players[fader2Player] != nullptr)
    {
        if (value.refersToSameSourceAs(players[fader2Player]->fileName))
        {
            fader2Name.setValue(players[fader2Player]->soundName.getText());
        }
    }
    else
        fader2Name.setValue("");
    for (auto i = 0; i < players.size(); i++)
    {
        if (value.refersToSameSourceAs(players[i]->draggedPlayer))
        {
            draggedPlayer.setValue(-1);
            draggedPlayer.setValue(i);
            //draggedPlayer = players[i]->draggedPlayer.toString().getIntValue();
        }

    }
    if (players[draggedPlayer.toString().getIntValue()] != nullptr)
    {
    if (value.refersToSameSourceAs(players[draggedPlayer.toString().getIntValue()]->cueStopped))
        {

        if (players[draggedPlayer.toString().getIntValue()]->cueStopped.toString().getIntValue() == 1)
            {

            }

        }
    }

}

void Playlist::buttonClicked(juce::Button* b)
{
    for (auto i = 0; i < players.size(); i++)
    {
        if (players[i] != nullptr)
        {
            if (b == &players[i]->playerPositionLabel)
            {
                assignPlaylistFader(i);
            }
        }
    }
}

void Playlist::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
    if (getParentComponent() != nullptr) // passes only if it's not a listening event 
        getParentComponent()->mouseWheelMove (event, wheel);
}
void Playlist::mouseDown(const juce::MouseEvent& event)
{

    if (playlistType == 0 && getMouseXYRelative().getX() < getWidth())
    {
        int draggedPlayer = getMouseXYRelative().getY() / (playerHeight + spaceBetweenPlayer);
        //if (draggedPlayer > std::max(fader1Player, fader2Player))
        //{
            if (!players[draggedPlayer]->transport.isPlaying())
            {
                mouseDragSource = draggedPlayer;
                mouseDraggedStartPositionOK = true;
                //assignPlaylistFader(draggedPlayer);
            }
        //}
    }
    else if (playlistType == 1 && getMouseXYRelative().getX() > (getParentWidth() - dragZoneWidth))
    {
        int draggedPlayer = getMouseXYRelative().getY() / (playerHeight + spaceBetweenPlayer);
            if (!players[draggedPlayer]->transport.isPlaying())
            {
                mouseDragSource = getMouseXYRelative().getY() / (playerHeight + spaceBetweenPlayer);
                mouseDraggedStartPositionOK = true;
            }
    }
}

void Playlist::mouseDrag(const juce::MouseEvent& event)
{
    if (mouseDraggedStartPositionOK)
    {
        mouseDragX = getMouseXYRelative().getX();
        mouseDragY = getMouseXYRelative().getY();
        mouseDraggedUp = 0;
        setMouseCursor(juce::MouseCursor::DraggingHandCursor);
        if (event.mods.isCtrlDown())
        {
            mouseCtrlModifier = true;
        }
        if (event.mods.isAltDown())
        {
            mouseAltModifier = true;
        }
    }



}

void Playlist::mouseUp(const juce::MouseEvent& event)
{
    if (playlistType == 0 && getMouseXYRelative().getX() < dragZoneWidth)
    {
        int player = getMouseXYRelative().getY() / (playerHeight + spaceBetweenPlayer);
        assignPlaylistFader(player);
    }
    mouseDraggedUp = 1;
    //mouseModifier = 0;
    setMouseCursor(juce::MouseCursor::NormalCursor);
    repaint();

}

void Playlist::removeButtonClicked()
{
    if (players.size() > 1)
        removePlayer(players.size() - 1);
    else
    {
        if (players[0] != nullptr
            && !players[0]->fader1IsPlaying
            && !players[0]->fader2IsPlaying
            && !players[0]->keyIsPlaying)
        players[0]->deleteFile();
    }
}

void Playlist::timerCallback()
{
    if (playlistType == 1)
    {
        for (auto i = 0; i < players.size(); i++)
        {
            if (i != fader1Player)
            {
                assignLeftFaderButtons[i]->setColour(juce::TextButton::ColourIds::buttonColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
            }
            else
                assignLeftFaderButtons[i]->setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::red);
            if (i != fader2Player)
            {
                assignRightFaderButtons[i]->setColour(juce::TextButton::ColourIds::buttonColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
            }
            else
                assignRightFaderButtons[i]->setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::red);
        }
    }
    else if (playlistType == 0)
        updateNextPlayer();
}


void Playlist::assignPlaylistFader(int playerToAssign)
{
    if ((fader1IsPlaying == 0) && (fader2IsPlaying == 0) && !spaceBarIsPlaying)
    {
        fader1Player = playerToAssign;
        fader2Player = playerToAssign;
        updateNextPlayer();
    }
    else if (playerToAssign > std::min(fader1Player, fader2Player))
    {
        if (fader1IsPlaying && !fader2IsPlaying)
            fader2Player = playerToAssign;
        else if (fader2IsPlaying && !fader1IsPlaying)
            fader1Player = playerToAssign;
        updateNextPlayer();
    }
}