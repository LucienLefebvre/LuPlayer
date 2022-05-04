/*
  ==============================================================================

    Denoiser.h
    Created: 15 Jan 2022 1:59:15pm
    Author:  Lucien Lefebvre

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DenoiseThread.h"
//==============================================================================
/*
*/
class Denoiser  :   public juce::Component,
                    public juce::Slider::Listener,
                    public juce::ComboBox::Listener,
                    public juce::ChangeListener
{
public:
    Denoiser();
    ~Denoiser() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void sliderValueChanged(juce::Slider* sliderThatWasMoved);
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged);
    void setFilePath(std::string f);
    void sendDenoiseParams();
    void previewButtonClicked();
    void processButtonClicked();
    void changeListenerCallback(juce::ChangeBroadcaster* source);
    std::string getDenoisedFile();

    bool loadFile(const juce::String& path, bool dispalyThumbnail);

    void playPreview();

    void setTransportGain(float g);

    void killThread();

    juce::ChangeBroadcaster* denoiseDoneBroadcaster;
    juce::ChangeBroadcaster* processStartedBroadcaster;

    juce::AudioTransportSource transport;
    juce::ResamplingAudioSource resampledSource;

    DenoiseThread thread{ "Denoise thread" };
private:
    std::unique_ptr<juce::Slider> noiseReductionSlider;
    std::unique_ptr<juce::Slider> noiseFloorSlider;
    std::unique_ptr<juce::Label> nrLabel;
    std::unique_ptr<juce::Label> nfLabel;
    std::unique_ptr<juce::ComboBox> noiseComboBox;
    std::unique_ptr<juce::Label> noiseTypeLabel;

    std::unique_ptr<juce::TextButton> previewButton;
    std::unique_ptr<juce::TextButton> processButton;

    DenoiseThread::DenoiseParameters denoiseParams;


    std::string returnedFilePath;

    double actualSampleRate;
    int actualSamplesPerBlockExpected;
    juce::AudioFormatManager formatManager;
    juce::AudioThumbnailCache thumbnailCache;
    juce::AudioThumbnail thumbnail;
    std::unique_ptr<juce::AudioFormatReaderSource> playSource;
    double fileSampleRate = 48000;

    bool isDirty = false;

    double barDouble = -1.0;
    juce::ProgressBar previewBar{ barDouble };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Denoiser)
};