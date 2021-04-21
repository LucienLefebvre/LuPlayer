/*
  ==============================================================================

    SoundBoard.h
    Created: 21 Apr 2021 11:03:33am
    Author:  DPR

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PlayerProcessor.h"
#include "PlayerEditor.h"
//==============================================================================
/*
*/
class SoundBoard  : public juce::Component
{
public:
    SoundBoard();
    ~SoundBoard() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    juce::OwnedArray<PlayerProcessor> playerProcessors;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoundBoard)
};
