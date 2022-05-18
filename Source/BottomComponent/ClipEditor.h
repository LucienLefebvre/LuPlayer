/*
  ==============================================================================

    ClipEditor.h
    Created: 24 Jan 2022 2:47:44pm
    Author:  Lucien Lefebvre

  ==============================================================================
*/

#pragma once
#define BLUE juce::Colour(40, 134, 189)
#define ORANGE juce::Colour(229, 149, 0)
#include <JuceHeader.h>
#include "EnveloppeEditor.h"
#include "../Mixer/Meter.h"
#include "../Settings/Settings.h"
//==============================================================================
/*
*/
class ClipEditor  : public juce::Component,
    public juce::ChangeListener,
    public juce::Slider::Listener,
    public juce::Timer,
    public juce::Button::Listener,
    public juce::Label::Listener,
    public juce::ComponentListener,
    public juce::ComboBox::Listener
{
public:
    ClipEditor()
    {
        juce::Timer::startTimerHz(30);
        grabFocusBroadcaster.reset(new juce::ChangeBroadcaster());

        addAndMakeVisible(&enveloppeEditor);

        //Name Label
        nameLabel.reset(new juce::Label);
        addAndMakeVisible(nameLabel.get());
        nameLabel->setJustificationType(juce::Justification::centred);
        nameLabel->setFont(juce::Font(25.0f, juce::Font::bold).withTypefaceStyle("Regular"));
        nameLabel->setColour(juce::Label::ColourIds::textColourId, juce::Colour(229, 149, 0));
        nameLabel->addListener(this);
        nameLabel->setEditable(false, true);
        nameLabel->setInterceptsMouseClicks(false, false);

        //Trim Volume Slider
        trimVolumeSlider.reset(new juce::Slider);
        addAndMakeVisible(trimVolumeSlider.get());
        trimVolumeSlider->setRange(-24, 24, 0.5);
        trimVolumeSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        trimVolumeSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 15);
        trimVolumeSlider->setColour(juce::Slider::ColourIds::textBoxOutlineColourId,
            getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        trimVolumeSlider->addListener(this);
        trimVolumeSlider->setDoubleClickReturnValue(true, 0.);
        trimVolumeSlider->setPopupDisplayEnabled(true, true, this, 2000);
        trimVolumeSlider->setScrollWheelEnabled(true);
        trimVolumeSlider->setWantsKeyboardFocus(false);
        trimVolumeSlider->setTextValueSuffix("dB");
        //Cue Button
        cueButton.reset(new juce::TextButton);
        addAndMakeVisible(cueButton.get());
        cueButton->setButtonText("Cue");
        cueButton->onClick = [this] {cueButtonClicked(); };
        //Cue Time Label
        cueTimeLabel.reset(new juce::Label);
        addAndMakeVisible(cueTimeLabel.get());
        cueTimeLabel->setJustificationType(juce::Justification::centred);
        cueTimeLabel->setFont(juce::Font(35.0f, juce::Font::bold).withTypefaceStyle("Regular"));
        //In Button
        inButton.reset(new juce::TextButton());
        addAndMakeVisible(inButton.get());
        inButton->setButtonText("IN");
        inButton->onClick = [this] {inButtonClicked(); };
        inButton->addListener(this);
        //In Label
        inLabel.reset(new juce::Label());
        addAndMakeVisible(inLabel.get());
        inLabel->setFont(juce::Font(25.0f, juce::Font::bold).withTypefaceStyle("Regular"));
        inLabel->setJustificationType(juce::Justification::centred);
        //Out Button
        outButton.reset(new juce::TextButton());
        addAndMakeVisible(outButton.get());
        outButton->setButtonText("OUT");
        outButton->onClick = [this] {outButtonClicked(); };
        outButton->addListener(this);
        //Out Label
        outLabel.reset(new juce::Label());
        addAndMakeVisible(outLabel.get());
        outLabel->setFont(juce::Font(25.0f, juce::Font::bold).withTypefaceStyle("Regular"));
        inLabel->setJustificationType(juce::Justification::centred);
        //Loop Button
        loopButton.reset(new juce::ToggleButton());
        addAndMakeVisible(loopButton.get());
        loopButton->setButtonText("Loop");
        loopButton->onClick = [this] {if (editedPlayer != nullptr) editedPlayer->updateLoopButton(loopButton.get(), "loop"); };
        //Enveloppe button
        enveloppeButton.reset(new juce::TextButton());
        addAndMakeVisible(enveloppeButton.get());
        enveloppeButton->setButtonText("Enveloppe");
        enveloppeButton->addListener(this);
        enveloppeButton->onClick = [this] {enveloppeButtonClicked(); };
        //FX button
        fxButton.reset(new juce::TextButton());
        addAndMakeVisible(fxButton.get());
        fxButton->setButtonText("Effects");
        fxButton->addListener(this);
        fxButton->onClick = [this] {fxButtonClicked(); };
        //Normalise button
        normButton.reset(new juce::TextButton());
        addAndMakeVisible(normButton.get());
        normButton->setButtonText("Normalise");
        enveloppeButton->addListener(this);
        normButton->onClick = [this] {if (editedPlayer != nullptr) editedPlayer->normButtonClicked(); };

#if RFBUILD
        //Denoise button
        denoiseButton.reset(new juce::TextButton());
        addAndMakeVisible(denoiseButton.get());
        denoiseButton->setButtonText("Denoiser");
        denoiseButton->addListener(this);
        denoiseButton->onClick = [this] {denoiseButtonClicked(); };
#endif
        //Open Button
        openButton.reset(new juce::TextButton());
        addAndMakeVisible(openButton.get());
        openButton->setButtonText("Open");
        openButton->onClick = [this] {if (editedPlayer != nullptr) editedPlayer->openButtonClicked(); };
        //Delete Button
        deleteButton.reset(new juce::TextButton());
        addAndMakeVisible(deleteButton.get());
        deleteButton->setButtonText("Delete");
        deleteButton->onClick = [this] {if (editedPlayer != nullptr) editedPlayer->deleteFile(); };
        //Colour Button
        colourButton.reset(new juce::TextButton());
        addAndMakeVisible(colourButton.get());
        colourButton->setButtonText("Colour");
        colourButton->onClick = [this] {if (editedPlayer != nullptr) colourButtonClicked(); };
        //Meter
        meter.reset(new Meter(Meter::Mode::Stereo));
        addAndMakeVisible(meter.get());
        enableButttons(false);
        //Play mode
        playModeLabel.reset(new juce::Label);
        addAndMakeVisible(playModeLabel.get());
        playModeLabel->setText("Play mode :", juce::dontSendNotification);
        playModeLabel->setJustificationType(juce::Justification::right);

        playModeSelector.reset(new juce::ComboBox);
        addAndMakeVisible(playModeSelector.get());
        playModeSelector->addItem("Trigger", 1);
        playModeSelector->addItem("Start / Stop", 2);
        playModeSelector->addListener(this);

        addAndMakeVisible(&playHead);
        playHead.setColour(juce::Colours::green);
        addAndMakeVisible(&cuePlayHead);
        cuePlayHead.setColour(juce::Colours::black);
        addChildComponent(&inMark);
        inMark.setColour(juce::Colour(0, 196, 255));
        inMark.setAlwaysOnTop(true);
        addChildComponent(&outMark);
        outMark.setColour(juce::Colour(238, 255, 0));
        outMark.setAlwaysOnTop(true);
    }

    ~ClipEditor() override
    {
        setNullPlayer();
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
        g.setColour(BLUE);
        g.fillRect(volumeSliderWidth, toolBarHeight, dividerBarWidth, getHeight());
        g.setColour(getLookAndFeel().findColour(juce::TextButton::ColourIds::buttonColourId));
        g.fillRect(0, 0, getWidth() - meterWidth, toolBarHeight);
        g.setColour(getLookAndFeel().findColour(juce::TextButton::ColourIds::buttonColourId));
        g.fillRect(0, getHeight() - bottomButtonHeight, getWidth() - meterWidth, bottomButtonHeight);
        g.setColour(BLUE);
        g.fillRect(enveloppeEditor.getRight(), 0, dividerBarWidth, getHeight());
    }

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate)
    {
        meter->prepareToPlay(samplesPerBlockExpected, sampleRate);
    }

    void resized() override
    {
        openButton->setBounds(0, 0, barButtonsWidth, toolBarHeight);
        deleteButton->setBounds(openButton->getRight() + spacer, 0, barButtonsWidth, toolBarHeight);
        colourButton->setBounds(deleteButton->getRight() + spacer, 0, barButtonsWidth, toolBarHeight);

        enveloppeButton->setBounds(deleteButton->getRight() + 200, 0, barButtonsWidth, toolBarHeight);
        fxButton->setBounds(enveloppeButton->getRight() + spacer, 0, barButtonsWidth, toolBarHeight);
        normButton->setBounds(fxButton->getRight() + spacer, 0, barButtonsWidth, toolBarHeight);
#if RFBUILD
        denoiseButton->setBounds(normButton->getRight() + spacer, 0, barButtonsWidth, toolBarHeight);
#endif

        playModeLabel->setBounds(normButton->getRight() + 200, 0, 200, toolBarHeight);
        playModeSelector->setBounds(playModeLabel->getRight() + spacer, 0, 200, toolBarHeight);

        inOutButtonWidth = getWidth() / 24;
        enveloppeEditor.setBounds(volumeSliderWidth + dividerBarWidth, toolBarHeight, getWidth() - volumeSliderWidth - meterWidth - dividerBarWidth * 2 - 2, getHeight() - toolBarHeight - bottomButtonHeight - 2);
        meter->setBounds(getWidth() - meterWidth, 0, meterWidth, getHeight());

        nameLabel->setBounds(enveloppeEditor.getX(), getHeight() - nameLabelHeight - 3, enveloppeEditor.getWidth(), nameLabelHeight);

        cueButton->setBounds(buttonsSpacer, toolBarHeight + 6, volumeSliderWidth - 2 * buttonsSpacer, 40);
        trimVolumeSlider->setSize(volumeSliderWidth, volumeSliderWidth);
        trimVolumeSlider->setCentrePosition(volumeSliderWidth / 2, cueButton->getBottom() + volumeSliderWidth / 2);

        inButton->setBounds(buttonsSpacer, trimVolumeSlider->getBottom() + 5, volumeSliderWidth - 2 * buttonsSpacer, 20);
        inLabel->setBounds(buttonsSpacer, inButton->getBottom() + 5, volumeSliderWidth - 2 * buttonsSpacer, 20);
        outButton->setBounds(buttonsSpacer, inLabel->getBottom() + 5, volumeSliderWidth - 2 * buttonsSpacer, 20);
        outLabel->setBounds(buttonsSpacer, outButton->getBottom() + 5, volumeSliderWidth - 2 * buttonsSpacer, 20);
        loopButton->setBounds(buttonsSpacer, outLabel->getBottom() + 5, volumeSliderWidth - 2 * buttonsSpacer, 20);

        playHead.setSize(2, enveloppeEditor.getHeight());
        cuePlayHead.setSize(2, enveloppeEditor.getHeight());
        inMark.setSize(2, enveloppeEditor.getHeight());
        outMark.setSize(2, enveloppeEditor.getHeight());
    }

    void setPlayer(Player* p)
    {
        if (editedPlayer != nullptr)
        {
            editedPlayer->soundEditedBroadcaster->removeChangeListener(this);
        }

        editedPlayer = p;
        if (editedPlayer != nullptr)
        {
            juce::FileLogger::getCurrentLogger()->writeToLog("set edited player");
            enveloppeEditor.setEditedPlayer(editedPlayer);
            editedPlayer->soundEditedBroadcaster->addChangeListener(this);
            editedPlayer->playerDeletedBroadcaster->addChangeListener(this);
            enableButttons(true);
            updateInfos();
        }
    }

    void spaceBarPressed()
    {
        if (isVisible())
            enveloppeEditor.spaceBarPressed();
    }

    void buttonStateChanged(juce::Button* b)
    {
        auto& modifiers = juce::ModifierKeys::getCurrentModifiers();
        if (modifiers.isRightButtonDown())
        {
            rightClickDown = true;
        }
        else if (modifiers.isCommandDown())
        {
            commandDown = true;
        }
    }

    void changeListenerCallback(juce::ChangeBroadcaster* source)
    {
        if (editedPlayer != nullptr)
        {
            if (source == editedPlayer->soundEditedBroadcaster)
            {
                updateInfos();
                return;
            }
            else if (source == editedPlayer->playerDeletedBroadcaster)
            {
                juce::FileLogger::getCurrentLogger()->writeToLog("clip editor player deleted");
                editedPlayer->setEditedPlayer(false);
                editedPlayer->soundEditedBroadcaster->removeChangeListener(this);
                editedPlayer->playerDeletedBroadcaster->removeChangeListener(this);
                setNullPlayer();
                return;
            }
            else if (juce::ColourSelector* cs = dynamic_cast <juce::ColourSelector*> (source))
            {
                auto c = cs->getCurrentColour();
                editedPlayer->setPlayerColour(c);
                enveloppeEditor.setSoundColour(c);
            }
        }
    }

    void updateInfos()
    {
        if (editedPlayer != nullptr)
        {
            //Name
            nameLabel->setText(editedPlayer->getName(), juce::NotificationType::dontSendNotification);
            
            //Trim
            trimVolumeSlider->setValue(editedPlayer->getTrimVolume());

            //Cue Button
            if (editedPlayer->cueTransport.isPlaying())
                cueButton->setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::green);
            else
                cueButton->setColour(juce::TextButton::ColourIds::buttonColourId, 
                    getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

            //In button
            if (editedPlayer->isStartTimeSet())
                inButton->setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colour(0, 196, 255));
            else
                inButton->setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colour(0, 115, 150));

            //In label
            inLabel->setText(editedPlayer->secondsToMMSS(editedPlayer->getStart()), juce::NotificationType::dontSendNotification);

            //Out Button
            if (editedPlayer->isStopTimeSet())
            {
                outButton->setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colour(238, 255, 0));
                outButton->setColour(juce::TextButton::ColourIds::textColourOffId, juce::Colours::black);
            }
            else
            {
                outButton->setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colour(141, 150, 0));
                outButton->setColour(juce::TextButton::ColourIds::textColourOffId, juce::Colours::white);
            }
            //Out label
            outLabel->setText(editedPlayer->secondsToMMSS(editedPlayer->getStop()), juce::NotificationType::dontSendNotification);

            //Loop Button
            loopButton->setToggleState(editedPlayer->getIsLooping(), juce::NotificationType::dontSendNotification);

            //Enveloppe
            if (editedPlayer->isEnveloppeEnabled())
            {
                enveloppeButton->setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::green);
            }

            else
            {
                enveloppeButton->setColour(juce::TextButton::ColourIds::buttonColourId,
                    getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
            }

            //FX
            if (editedPlayer->isFxEnabled())
            {
                fxButton->setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::green);
            }

            else
            {
                fxButton->setColour(juce::TextButton::ColourIds::buttonColourId,
                    getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
            }

#if RFBUILD
            //Denoiser
            if (editedPlayer->getDenoisedFileLoaded())
            {
                denoiseButton->setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::green);
            }
            else
            {
                denoiseButton->setColour(juce::TextButton::ColourIds::buttonColourId,
                    getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
            }
#endif

            //loop
            loopButton->setEnabled(editedPlayer->isCart ? true : false);

            //Colour
            if (editedPlayer->getColourHasChanged())
            {
                enveloppeEditor.setSoundColour(editedPlayer->getPlayerColour());
                nameLabel->setColour(juce::Label::ColourIds::textColourId, editedPlayer->getPlayerColour());
            }
            else
            {
                enveloppeEditor.setSoundColour(BLUE);
                nameLabel->setColour(juce::Label::ColourIds::textColourId, BLUE);
            }

            if (Settings::preferedSoundPlayerMode == 3)
            {
                playModeLabel->setVisible(true);
                playModeSelector->setVisible(true);
                playModeSelector->setSelectedId(editedPlayer->getPlayMode());
            }
            else
            {
                playModeLabel->setVisible(false);
                playModeSelector->setVisible(false);
            }

        }
    }

    void sliderValueChanged(juce::Slider* slider)
    {
        if (editedPlayer != nullptr)
        {
            if (slider == trimVolumeSlider.get())
            {
                editedPlayer->setTrimVolume(trimVolumeSlider->getValue());
            }
        }
    }

    void buttonClicked(juce::Button* b)
    {
    }

    void cueButtonClicked()
    {
        if (editedPlayer != nullptr)
        {
            editedPlayer->cueButtonClicked();
        }
    }

    void timerCallback()
    {
        if (editedPlayer != nullptr)
        {
            cueTimeLabel->setText(editedPlayer->getCueTimeAsString(), juce::NotificationType::dontSendNotification);
            meter->setRMSMeterData(editedPlayer->outMeterSource.getRMSLevel(0), editedPlayer->outMeterSource.getRMSLevel(1));
            meter->setPeakMeterDate(editedPlayer->outMeterSource.getMaxLevel(0), editedPlayer->outMeterSource.getMaxLevel(1));

            //Playhead
            juce::AudioTransportSource* transport = &editedPlayer->transport;
            auto audioPosition = (float)transport->getCurrentPosition();
            auto transportLenght = transport->getLengthInSeconds();
            auto drawPosition = (enveloppeEditor.timeToX(audioPosition / transportLenght));
            if (drawPosition < 0 || drawPosition > enveloppeEditor.getWidth())
                drawPosition = -100;
            playHead.setTopLeftPosition(enveloppeEditor.getX() + drawPosition, enveloppeEditor.getY());

            //Cue Playhead
            juce::AudioTransportSource* cueTransport = &editedPlayer->cueTransport;
            auto cueaudioPosition = (float)cueTransport->getCurrentPosition();
            auto cuetransportLenght = cueTransport->getLengthInSeconds();
            auto cuedrawPosition = (enveloppeEditor.timeToX(cueaudioPosition / cuetransportLenght));
            if (cuedrawPosition < 0 || cuedrawPosition > enveloppeEditor.getWidth())
                cuedrawPosition = -100;
            cuePlayHead.setTopLeftPosition(enveloppeEditor.getX() + cuedrawPosition, enveloppeEditor.getY());

            //in mark
            if (editedPlayer->isStartTimeSet())
            {
                inMark.setVisible(true);
                auto inDrawPosition = enveloppeEditor.timeToX(editedPlayer->getStart() / editedPlayer->transport.getLengthInSeconds());
                if (inDrawPosition < 0 || inDrawPosition > enveloppeEditor.getWidth())
                    inDrawPosition = -100;
                inMark.setTopLeftPosition(enveloppeEditor.getX() + inDrawPosition, enveloppeEditor.getY());
            }
            else
                inMark.setVisible(false);

            //out mark
            if (editedPlayer->isStopTimeSet())
            {
                outMark.setVisible(true);
                auto outDrawPosition = enveloppeEditor.timeToX(editedPlayer->getStop() / editedPlayer->transport.getLengthInSeconds());
                if (outDrawPosition < 0 || outDrawPosition > enveloppeEditor.getWidth())
                    outDrawPosition = -100;
                outMark.setTopLeftPosition(enveloppeEditor.getX() + outDrawPosition, enveloppeEditor.getY());
            }
            else
                outMark.setVisible(false);

        }
    }

    void inButtonClicked()
    {
        if (editedPlayer != nullptr)
        {
            if (rightClickDown)
            {
                editedPlayer->deleteStart();
                rightClickDown = false;
            }
            else
                editedPlayer->setTimeClicked();
        }
    }

    void outButtonClicked()
    {
        if (editedPlayer != nullptr)
        {
            if (rightClickDown)
            {
                editedPlayer->deleteStop();
                rightClickDown = false;
            }
            else
                editedPlayer->stopTimeClicked();
        }
    }

    void enveloppeButtonClicked()
    {
        if (editedPlayer != nullptr)
        {
            if (commandDown)
            {

                editedPlayer->createDefaultEnveloppePath();
                editedPlayer->setEnveloppeEnabled(false);
                enveloppeEditor.setEditedPlayer(editedPlayer);
            }
            else
                editedPlayer->setEnveloppeEnabled(!editedPlayer->isEnveloppeEnabled());
        }
        commandDown = false;
    }

    void fxButtonClicked()
    {
        if (editedPlayer != nullptr)
        {
            if (rightClickDown)
            {
                editedPlayer->bypassFX(editedPlayer->isFxEnabled(), false);
                updateInfos();
            }
            else
                editedPlayer->fxButtonClicked();
        }
        rightClickDown = false;
    }

#if RFBUILD
    void denoiseButtonClicked()
    {
        if (editedPlayer != nullptr)
        {
            if (rightClickDown)
            {
                editedPlayer->setDenoisedFile(!editedPlayer->getDenoisedFileLoaded());
                updateInfos();
            }
            else
                editedPlayer->denoiseButtonClicked();
        }
        rightClickDown = false;
    }
#endif

    void colourButtonClicked()
    {
        if (editedPlayer != nullptr)
        {
            auto cs = std::make_unique<juce::ColourSelector>();
            cs->setName("colour");
            cs->addChangeListener(this);
            cs->setColour(juce::ColourSelector::backgroundColourId, juce::Colours::transparentBlack);
            cs->setCurrentColour(editedPlayer->getPlayerColour(), juce::NotificationType::dontSendNotification);
            cs->setSize(300, 400);
            cs->setMouseClickGrabsKeyboardFocus(false);
            cs->setWantsKeyboardFocus(false);
            cs->unfocusAllComponents();
            cs->addComponentListener(this);
            juce::CallOutBox::launchAsynchronously(std::move(cs), colourButton->getScreenBounds(), this);
        }
    }



    void enableButttons(bool isEnabled)
    {
        trimVolumeSlider->setEnabled(isEnabled);
        cueButton->setEnabled(isEnabled);
        inButton->setEnabled(isEnabled);
        outButton->setEnabled(isEnabled);
        loopButton->setEnabled(isEnabled);
        enveloppeButton->setEnabled(isEnabled);
        fxButton->setEnabled(isEnabled);
        normButton->setEnabled(isEnabled);
        openButton->setEnabled(isEnabled);
        deleteButton->setEnabled(isEnabled);
#if RFBUILD
        denoiseButton->setEnabled(isEnabled);
#endif
    }

    void setNullPlayer()
    {
        enableButttons(false);
        editedPlayer = nullptr;
        nameLabel->setText("", juce::NotificationType::dontSendNotification);
        inLabel->setText("", juce::NotificationType::dontSendNotification);
        outLabel->setText("", juce::NotificationType::dontSendNotification);
        loopButton->setToggleState(false, juce::NotificationType::dontSendNotification);
        cueTimeLabel->setText("", juce::NotificationType::dontSendNotification);
        trimVolumeSlider->setValue(0.0);
        enveloppeEditor.setNullPlayer();
        enveloppeButton->setColour(juce::TextButton::ColourIds::buttonColourId,
            getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        fxButton->setColour(juce::TextButton::ColourIds::buttonColourId,
            getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        inMark.setVisible(false);
        outMark.setVisible(false);
#if RFBUILD
        denoiseButton->setColour(juce::TextButton::ColourIds::buttonColourId,
            getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
#endif
    }

    void setStart()
    {
        if (isVisible())
            inButtonClicked();
    }

    void setStop()
    {
        if (isVisible())
            outButtonClicked();
    }

    void launchCue()
    {
        if (isVisible() && editedPlayer != nullptr)
            editedPlayer->cueButtonClicked();
    }

    EnveloppeEditor* getEnveloppeEditor()
    {
        return &enveloppeEditor;
    }

    void labelTextChanged(juce::Label* labelThatHasChanged)
    {
        if (editedPlayer != nullptr)
        {
            if (labelThatHasChanged == nameLabel.get())
                editedPlayer->setName(nameLabel->getText().toStdString(), true);
        }
    }
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
    {
        if (comboBoxThatHasChanged == playModeSelector.get())
        {
            if (editedPlayer != nullptr)
            {
                editedPlayer->setPlayMode(comboBoxThatHasChanged->getSelectedId());
            }
        }
    }

    void componentBeingDeleted(juce::Component& component)
    {
        grabFocusBroadcaster->sendChangeMessage();
    }
    std::unique_ptr<juce::ChangeBroadcaster> grabFocusBroadcaster;

    EnveloppeEditor enveloppeEditor;
    Player* editedPlayer = nullptr;

    bool rightClickDown = false;
    bool commandDown = false;

    int dividerBarWidth = 4;
    int volumeSliderWidth = 80;
    int inOutButtonWidth = 50;
    int meterWidth = 50;
    int toolBarHeight = 25;
    int bottomButtonHeight = 30;
    int nameLabelHeight = 25;
    int buttonsSpacer = 8;
    int barButtonsWidth = 100;
    int spacer = 5;

    std::unique_ptr<juce::Label> nameLabel;
    std::unique_ptr<juce::Slider> trimVolumeSlider;
    std::unique_ptr<juce::Label> cueTimeLabel;
    std::unique_ptr<juce::TextButton> cueButton;
    std::unique_ptr<juce::TextButton> inButton;
    std::unique_ptr<juce::TextButton> outButton;
    std::unique_ptr<juce::Label> inLabel;
    std::unique_ptr<juce::Label> outLabel;
    std::unique_ptr<juce::ToggleButton> loopButton;
    std::unique_ptr<juce::TextButton> enveloppeButton;
    std::unique_ptr<juce::TextButton> fxButton;
    std::unique_ptr<juce::TextButton> normButton;
    std::unique_ptr<juce::TextButton> openButton;
    std::unique_ptr<juce::TextButton> deleteButton;
    std::unique_ptr<juce::TextButton> colourButton;
    std::unique_ptr<juce::Label> playModeLabel;
    std::unique_ptr<juce::ComboBox> playModeSelector;

    PlayHead playHead;
    PlayHead cuePlayHead;
    PlayHead inMark;
    PlayHead outMark;
#if RFBUILD
    std::unique_ptr<juce::TextButton> denoiseButton;
#endif

    std::unique_ptr<Meter> meter;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ClipEditor)
};
