/*
  ==============================================================================

    Cart.cpp
    Created: 11 Feb 2021 12:12:58am
    Author:  Lucien

  ==============================================================================
*/

#include <JuceHeader.h>
#include "Cart.h"

//==============================================================================
Cart::Cart()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    //auto myPlayer = new Player(10);

    myPlayers.add(new Player(0));
    myPlayers[0]->setBounds(0, 0, 800, 100);
    addAndMakeVisible(myPlayers[0]);

    cartMixer.addInputSource(&myPlayers[0]->resampledSource, false);
    //myPlayers[0]->playerPrepareToPlay(samplesPerBlockExpected, sampleRate);
}

Cart::~Cart()
{
}

void Cart::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

}

void Cart::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}

void Cart::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()

    //myPlayer->prepareToPlay(samplesPerBlockExpected, sampleRate);
    //actualSampleRate = sampleRate;
    //actualSamplesPerBlockExpected = samplesPerBlockExpected;
    myPlayers[0]->playerPrepareToPlay(samplesPerBlockExpected, sampleRate);
    //myCartMixer.prepareToPlay(samplesPerBlockExpected, sampleRate);

}

void Cart::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    //bufferToFill.clearActiveBufferRegion();
    //myCartMixer.getNextAudioBlock(bufferToFill);


    //playlistMixer.getNextAudioBlock(bufferToFill);
}
