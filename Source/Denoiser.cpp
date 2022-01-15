/*
  ==============================================================================

    Denoiser.cpp
    Created: 15 Jan 2022 1:59:15pm
    Author:  DPR

  ==============================================================================
*/

#include <JuceHeader.h>
#include "Denoiser.h"

//==============================================================================
Denoiser::Denoiser() : resampledSource(&transport, false, 2), thumbnailCache(5), thumbnail(521, formatManager, thumbnailCache)
{
    setSize(400, 115);

    denoiseDoneBroadcaster = new juce::ChangeBroadcaster();
    processStartedBroadcaster = new juce::ChangeBroadcaster();

    thread.previewEndedBroadcaster->addChangeListener(this);
    thread.denoiseEndedBroadcaster->addChangeListener(this);

    transport.addChangeListener(this);
    formatManager.registerBasicFormats();

    noiseReductionSlider.reset(new juce::Slider("noiseReductionSlider"));
    addAndMakeVisible(noiseReductionSlider.get());
    noiseReductionSlider->setRange(1, 24, 1);
    noiseReductionSlider->setSliderStyle(juce::Slider::LinearHorizontal);
    noiseReductionSlider->setTextBoxStyle(juce::Slider::TextBoxLeft, false, 50, 20);
    noiseReductionSlider->addListener(this);
    noiseReductionSlider->setNumDecimalPlacesToDisplay(1);
    noiseReductionSlider->setValue(6.);
    noiseReductionSlider->setTextValueSuffix("dB");

    noiseReductionSlider->setBounds(150, 0, 250, 25);

    noiseFloorSlider.reset(new juce::Slider("noiseFloorSlider"));
    addAndMakeVisible(noiseFloorSlider.get());
    noiseFloorSlider->setRange(-50, -20, 1);
    noiseFloorSlider->setSliderStyle(juce::Slider::LinearHorizontal);
    noiseFloorSlider->setTextBoxStyle(juce::Slider::TextBoxLeft, false, 50, 20);
    noiseFloorSlider->addListener(this);
    noiseFloorSlider->setValue(-30.);
    noiseFloorSlider->setNumDecimalPlacesToDisplay(1);
    noiseFloorSlider->setTextValueSuffix("dB");

    noiseFloorSlider->setBounds(150, 25, 250, 25);

    nrLabel.reset(new juce::Label("new label",
        TRANS("Noise Reduction")));
    addAndMakeVisible(nrLabel.get());
    nrLabel->setFont(juce::Font(15.00f, juce::Font::plain).withTypefaceStyle("Regular"));
    nrLabel->setJustificationType(juce::Justification::centredLeft);
    nrLabel->setEditable(false, false, false);
    nrLabel->setColour(juce::TextEditor::textColourId, juce::Colours::black);
    nrLabel->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x00000000));

    nrLabel->setBounds(0, 0, 150, 24);

    nfLabel.reset(new juce::Label("nfLabel",
        TRANS("Noise Floor\n")));
    addAndMakeVisible(nfLabel.get());
    nfLabel->setFont(juce::Font(15.00f, juce::Font::plain).withTypefaceStyle("Regular"));
    nfLabel->setJustificationType(juce::Justification::centredLeft);
    nfLabel->setEditable(false, false, false);
    nfLabel->setColour(juce::TextEditor::textColourId, juce::Colours::black);
    nfLabel->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x00000000));

    nfLabel->setBounds(0, 25, 150, 25);

    noiseComboBox.reset(new juce::ComboBox("new combo box"));
    addAndMakeVisible(noiseComboBox.get());
    noiseComboBox->setEditableText(false);
    noiseComboBox->setJustificationType(juce::Justification::centredLeft);
    noiseComboBox->setTextWhenNothingSelected(juce::String());
    noiseComboBox->setTextWhenNoChoicesAvailable(TRANS("(no choices)"));
    noiseComboBox->addItem(TRANS("White Noise"), 1);
    noiseComboBox->addItem(TRANS("Vinyl Noise"), 2);
    noiseComboBox->addItem(TRANS("Shellac Noise"), 3);
    noiseComboBox->setSelectedId(1, juce::NotificationType::dontSendNotification);
    noiseComboBox->addSeparator();
    noiseComboBox->addListener(this);

    noiseComboBox->setBounds(150, 50, 245, 25);

    noiseTypeLabel.reset(new juce::Label("noiseTypeLabel",
        TRANS("Noise Type\n")));
    addAndMakeVisible(noiseTypeLabel.get());
    noiseTypeLabel->setFont(juce::Font(15.00f, juce::Font::plain).withTypefaceStyle("Regular"));
    noiseTypeLabel->setJustificationType(juce::Justification::centredLeft);
    noiseTypeLabel->setEditable(false, false, false);
    noiseTypeLabel->setColour(juce::TextEditor::textColourId, juce::Colours::black);
    noiseTypeLabel->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x00000000));

    noiseTypeLabel->setBounds(0, 50, 150, 25);

    previewButton.reset(new juce::TextButton);
    addAndMakeVisible(previewButton.get());
    previewButton->setButtonText("Preview");
    previewButton->onClick = [this] { previewButtonClicked(); };
    previewButton->setBounds(25, 82, 100, 25);

    processButton.reset(new juce::TextButton);
    addAndMakeVisible(processButton.get());
    processButton->setButtonText("Process");
    processButton->onClick = [this] { processButtonClicked(); };
    processButton->setBounds(275, 82, 100, 25);

    addChildComponent(previewBar);
    previewBar.setBounds(150, 82, 100, 25);
    previewBar.setColour(juce::ProgressBar::ColourIds::backgroundColourId, juce::Colour(40, 134, 189));
}

Denoiser::~Denoiser()
{
    thread.previewEndedBroadcaster->removeChangeListener(this);
    thread.denoiseEndedBroadcaster->removeChangeListener(this);
    transport.setSource(nullptr);
    transport.releaseResources();
    noiseReductionSlider = nullptr;
    noiseFloorSlider = nullptr;
    nrLabel = nullptr;
    nfLabel = nullptr;
    noiseComboBox = nullptr;
    noiseTypeLabel = nullptr;
    delete denoiseDoneBroadcaster;
    delete processStartedBroadcaster;
}

void Denoiser::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
    g.setColour(juce::Colour(40, 134, 189));
    g.drawRect(getLocalBounds(), 1);
}

void Denoiser::resized()
{

}

void Denoiser::sliderValueChanged(juce::Slider* sliderThatWasMoved)
{
    if (sliderThatWasMoved == noiseReductionSlider.get())
    {
        denoiseParams.noiseReductionDB = noiseReductionSlider->getValue();
    }
    else if (sliderThatWasMoved == noiseFloorSlider.get())
    {
        denoiseParams.noiseFloorDB = noiseFloorSlider->getValue();
    }
    isDirty = true;
    transport.stop();
}

void Denoiser::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == noiseComboBox.get())
    {
        int r = comboBoxThatHasChanged->getSelectedId();
        switch (r)
        {
        case 1 :
            denoiseParams.noiseType = "w";
            break;
        case 2 :
            denoiseParams.noiseType = "v";
            break;
        case 3 :
            denoiseParams.noiseType = "s";
            break;
        }
        isDirty = true;
        transport.stop();
    }
}

void Denoiser::setFilePath(std::string f)
{
    denoiseParams.filePath = f;
}

void Denoiser::sendDenoiseParams()
{
    thread.setDenoiseParams(denoiseParams);
}

void Denoiser::previewButtonClicked()
{
    if (!transport.isPlaying())
    {
        if (isDirty == true)
        {
            transport.stop();
            denoiseParams.isPreview = true;
            sendDenoiseParams();
            thread.startThread();
            isDirty = false;
            previewBar.setVisible(true);
        }
        else if (isDirty == false)
        {
            playPreview();
        }
    }
    else
        transport.stop();
}

void Denoiser::processButtonClicked()
{
    if (!denoiseParams.filePath.empty())
    {
        denoiseParams.isPreview = false;
        sendDenoiseParams();
        thread.startThread();
        processStartedBroadcaster->sendChangeMessage();
    }
}

void Denoiser::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == thread.denoiseEndedBroadcaster)
    {
        returnedFilePath = thread.getFile().toStdString();
        thread.stopThread(1000);
        denoiseDoneBroadcaster->sendChangeMessage();
    }
    else if (source == thread.previewEndedBroadcaster)
    {
        previewBar.setVisible(false);
        loadFile(thread.getFile().toStdString(), false);
        playPreview();
    }
    else if (source == &transport)
    {
        if (!transport.isPlaying())
        {
            previewButton->setColour(juce::TextButton::ColourIds::buttonColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
            previewButton->setButtonText("Preview");
        }
    }
}

std::string Denoiser::getDenoisedFile()
{
    return returnedFilePath;
}

bool Denoiser::loadFile(const juce::String& path, bool dispalyThumbnail)
{

    juce::File file = juce::File(path);
    //deleteFile();
    if (juce::AudioFormatReader* reader = formatManager.createReaderFor(file))
    {
        //transport
        std::unique_ptr<juce::AudioFormatReaderSource> tempSource(new juce::AudioFormatReaderSource(reader, true));
        transport.setSource(tempSource.get());
        transport.addChangeListener(this);
        resampledSource.setResamplingRatio(reader->sampleRate / Settings::sampleRate);
        fileSampleRate = reader->sampleRate;
        playSource.reset(tempSource.release());
        if (dispalyThumbnail)
        {
            thumbnail.setSource(new juce::FileInputSource(file));
            repaint();
        }
        //startStopButton.setEnabled(true);
        return true;
    }
    else
        return false;
}

void Denoiser::playPreview()
{
    transport.setPosition(0);
    transport.start();
    previewButton->setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::green);
    previewButton->setButtonText("Stop");
}

void Denoiser::setTransportGain(float g)
{
    transport.setGain(juce::Decibels::decibelsToGain(g));
}