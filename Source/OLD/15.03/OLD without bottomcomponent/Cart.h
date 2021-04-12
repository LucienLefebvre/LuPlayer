/*
  ==============================================================================

    Cart.h
    Created: 11 Feb 2021 12:12:58am
    Author:  Lucien

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Player.h"
//==============================================================================
/*
*/
class Cart  : public juce::Component
{
public:
    Cart();
    ~Cart() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void Cart::prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void Cart::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill);

    juce::OwnedArray<Player> myPlayers;
    juce::MixerAudioSource cartMixer;

 private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Cart)
};
