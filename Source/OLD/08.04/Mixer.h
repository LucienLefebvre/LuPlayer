/*
  ==============================================================================

    Mixer.h
    Created: 3 Apr 2021 12:32:50pm
    Author:  DPR

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "RemoteInput.h"
//==============================================================================
/*
*/
class Mixer  : public juce::Component
{
public:
    Mixer();
    ~Mixer() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    RemoteInput remoteInput1;
    void Mixer::prepareToPlay(int samplesPerBlockExpected, double sampleRate);

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Mixer)
};
