/*
  ==============================================================================

    ClipEffect.h
    Created: 12 Jan 2022 4:22:43pm
    Author:  DPR

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Mixer/FilterProcessor.h"
#include "Mixer/FilterEditor.h"
#include "Mixer/Meter.h"
#include "Mixer/CompEditor.h"
#include "Player.h"
//==============================================================================
/*
*/
class ClipEffect : public juce::Component, public juce::Timer, public juce::ChangeListener
{
public:
    ClipEffect();
    ~ClipEffect() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void paint (juce::Graphics&) override;
    void resized() override;
    void setEditedFilterProcessor(FilterProcessor& fp);
    void setEditedCompProcessor(CompProcessor& cp);
    void setEditedBuffer(std::unique_ptr<juce::AudioBuffer<float>>& buffer);
    void setMeters(Meter& inputMeter, Meter& outputBuffer, Meter& compMeter);
    void setName(std::string n);
    void timerCallback();

    void setPlayer(Player* p);

    void setDummyPlayer();

    void changeListenerCallback(juce::ChangeBroadcaster* source);


private:
    FilterEditor filterEditor;
    CompEditor compEditor;
    std::unique_ptr<juce::AudioBuffer<float>>* editedBuffer;
    Player* editedPlayer = nullptr;
    Meter* inputMeter;
    Meter* outputMeter;
    Meter* compMeter;
    Meter displayInputMeter{ Meter::Mode::Stereo };
    Meter displayOutputMeter{ Meter::Mode::Stereo };
    Meter displayCompMeter{ Meter::Mode::Stereo_ReductionGain };
    Meter initializationMeter{ Meter::Mode::Stereo };

    FilterProcessor dummyFilterProcessor;
    Player dummyPlayer{ 0 };

    juce::String name;
    juce::Label nameLabel;

    int spaceBetweenComponents = 10;
    int meterSize = 50;
    int compWidth = 130;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ClipEffect)
};
