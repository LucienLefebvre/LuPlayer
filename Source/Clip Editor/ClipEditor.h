/*
  ==============================================================================

    ClipEditor.h
    Created: 24 Jan 2022 2:47:44pm
    Author:  DPR

  ==============================================================================
*/

#pragma once
#define BLUE juce::Colour(40, 134, 189)
#define ORANGE juce::Colour(229, 149, 0)
#include <JuceHeader.h>
#include "EnveloppeEditor.h"
#include "../Mixer/Meter.h"
//==============================================================================
/*
*/
class ClipEditor  : public juce::Component,
    public juce::ChangeListener,
    public juce::Slider::Listener,
    public juce::Timer,
    public juce::Button::Listener,
    public juce::Label::Listener,
    public juce::ComponentListener
{
public:
    ClipEditor()
    {
        juce::Timer::startTimerHz(30);
        grabFocusBroadcaster.reset(new juce::ChangeBroadcaster());

        addAndMakeVisible(&enveloppeEditor);

        //Toolbar
        clipToolBar.reset(new juce::Toolbar);
        addAndMakeVisible(clipToolBar.get());
        clipToolBar->addDefaultItems(factory);
        for (auto b : factory.broadcasters)
            b->addChangeListener(this);

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
        normButton->onClick = [this] {if (editedPlayer != nullptr) editedPlayer->normButtonClicked(); };
        //Denoise button
        denoiseButton.reset(new juce::TextButton());
        addAndMakeVisible(denoiseButton.get());
        denoiseButton->setButtonText("Denoiser");
        denoiseButton->addListener(this);
        denoiseButton->onClick = [this] {denoiseButtonClicked(); };
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
        //Meter
        meter.reset(new Meter(Meter::Mode::Stereo));
        addAndMakeVisible(meter.get());
        enableButttons(false);
    }

    ~ClipEditor() override
    {
        for (auto b : factory.broadcasters)
            b->removeChangeListener(this);
        clipToolBar->clear();
        factory.broadcasters.clear();
        setNullPlayer();
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
        g.setColour(BLUE);
        g.fillRect(volumeSliderWidth, toolBarHeight, dividerBarWidth, getHeight());
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
        clipToolBar->setBounds(0, 0, getWidth() - meterWidth - dividerBarWidth - 2, toolBarHeight);
        inOutButtonWidth = getWidth() / 24;
        enveloppeEditor.setBounds(volumeSliderWidth + dividerBarWidth, clipToolBar->getBottom(), getWidth() - volumeSliderWidth - meterWidth - dividerBarWidth * 2 - 2, getHeight() - toolBarHeight - bottomButtonHeight - 2);
        meter->setBounds(getWidth() - meterWidth, 0, meterWidth, getHeight());

        nameLabel->setBounds(enveloppeEditor.getX(), getHeight() - nameLabelHeight - 3, enveloppeEditor.getWidth(), nameLabelHeight);

        cueButton->setBounds(buttonsSpacer, clipToolBar->getBottom() + 6, volumeSliderWidth - 2 * buttonsSpacer, 40);
        trimVolumeSlider->setSize(volumeSliderWidth, volumeSliderWidth);
        trimVolumeSlider->setCentrePosition(volumeSliderWidth / 2, cueButton->getBottom() + volumeSliderWidth / 2);

        inButton->setBounds(buttonsSpacer, trimVolumeSlider->getBottom() + 5, volumeSliderWidth - 2 * buttonsSpacer, 20);
        inLabel->setBounds(buttonsSpacer, inButton->getBottom() + 5, volumeSliderWidth - 2 * buttonsSpacer, 20);
        outButton->setBounds(buttonsSpacer, inLabel->getBottom() + 5, volumeSliderWidth - 2 * buttonsSpacer, 20);
        outLabel->setBounds(buttonsSpacer, outButton->getBottom() + 5, volumeSliderWidth - 2 * buttonsSpacer, 20);
        loopButton->setBounds(buttonsSpacer, outLabel->getBottom() + 5, volumeSliderWidth - 2 * buttonsSpacer, 20);
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
            int result = -1;
            for (int i = 0; i < factory.broadcasters.size(); i++)
            {
                if (source == factory.broadcasters[i])
                {
                    result = i + 1;
                }
            }
            switch (result)
            {
                case 1:
                    editedPlayer->openButtonClicked();
                    break;
                case 2:
                    editedPlayer->deleteFile();
                    break;
                case 3:
                    colourButtonClicked();
                    break;
                case 4:
                    if (factory.rightClickDown)
                    {
                        editedPlayer->setEnveloppeEnabled(!editedPlayer->isEnveloppeEnabled());
                        factory.rightClickDown = false;
                    }
                    else if (factory.commandDown)
                    {
                        editedPlayer->createDefaultEnveloppePath();
                        editedPlayer->setEnveloppeEnabled(false, true, true);
                        updateInfos();
                        factory.commandDown = false;
                    }
                    else
                        editedPlayer->envButtonClicked();
                    break;
                case 5:
                    if (factory.rightClickDown)
                    {
                        editedPlayer->bypassFX(editedPlayer->isFxEnabled(), false);
                        updateInfos();
                        factory.rightClickDown = false;
                    }
                    else
                        editedPlayer->fxButtonClicked();
                    break;
                case 6:
                    editedPlayer->normButtonClicked();
                    break;
                case 7:
                    if (factory.rightClickDown)
                    {
                        editedPlayer->setDenoisedFile(!editedPlayer->getDenoisedFileLoaded());
                        updateInfos();
                    }
                    else
                        editedPlayer->denoiseButtonClicked();
                    break;
            }
        }
    }

    void updateInfos()
    {
        if (editedPlayer != nullptr)
        {

            //Name
            nameLabel->setText(editedPlayer->getName(), juce::NotificationType::dontSendNotification);
            

            //Tril
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
                enveloppeButton->setColour(juce::TextButton::ColourIds::buttonColourId, BLUE);
                factory.labels[TItemFactory::ToolBarItemIDs::Enveloppe - 1]->setTextColour(juce::Colours::green);
            }

            else
            {
                enveloppeButton->setColour(juce::TextButton::ColourIds::buttonColourId,
                    getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
                factory.labels[TItemFactory::ToolBarItemIDs::Enveloppe - 1]->setTextColour(juce::Colours::white);
            }


            //FX
            if (editedPlayer->isFxEnabled())
            {
                fxButton->setColour(juce::TextButton::ColourIds::buttonColourId, BLUE);
                factory.labels[TItemFactory::ToolBarItemIDs::Effects - 1]->setTextColour(juce::Colours::green);
            }

            else
            {
                fxButton->setColour(juce::TextButton::ColourIds::buttonColourId,
                    getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
                factory.labels[TItemFactory::ToolBarItemIDs::Effects - 1]->setTextColour(juce::Colours::white);
            }

            //Denoiser
            if (editedPlayer->getDenoisedFileLoaded())
            {
                denoiseButton->setColour(juce::TextButton::ColourIds::buttonColourId, BLUE);
                factory.labels[TItemFactory::ToolBarItemIDs::Denoise - 1]->setTextColour(juce::Colours::green);
            }
            else
            {
                denoiseButton->setColour(juce::TextButton::ColourIds::buttonColourId,
                    getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
                factory.labels[TItemFactory::ToolBarItemIDs::Denoise - 1]->setTextColour(juce::Colours::white);
            }

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
            if (rightClickDown)
            {
                editedPlayer->setEnveloppeEnabled(!editedPlayer->isEnveloppeEnabled());
                updateInfos();
            }
            else
                editedPlayer->envButtonClicked();
        }
        rightClickDown = false;
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
            juce::CallOutBox::launchAsynchronously(std::move(cs), factory.labels[TItemFactory::ToolBarItemIDs::Select_Colour - 1]->getScreenBounds(), this);
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
        denoiseButton->setEnabled(isEnabled);
        openButton->setEnabled(isEnabled);
        deleteButton->setEnabled(isEnabled);
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
    void componentBeingDeleted(juce::Component& component)
    {
        grabFocusBroadcaster->sendChangeMessage();
    }
    std::unique_ptr<juce::ChangeBroadcaster> grabFocusBroadcaster;
private:
    class TItemFactory : public juce::ToolbarItemFactory,
        public juce::Button::Listener
    {
    public:
        enum ToolBarItemIDs
        {
            Open_Sound = 1,
            Delete_Sound,
            Select_Colour,
            Enveloppe,
            Effects,
            Normalize,
            Denoise,
            NUM_ITEMS
        };

        TItemFactory()
        {
            for (int i = 0; i < NUM_ITEMS; i++)
            {
                labels.add(new CustomToolbarLabel(i+1));
                labels[i]->addListener(this);
            }

            for (int i = 0; i < NUM_ITEMS; i++)
            {
                broadcasters.add(new juce::ChangeBroadcaster());
            }

        }

        ~TItemFactory()
        {
            //labels.clear(false);
            //for (int i = 0; i < NUM_ITEMS; i++)
            //{
            //    labels[i]->removeListener(this);
            //}
            //labels.clear(false);
            //broadcasters.clear();
        }

        void getAllToolbarItemIds(juce::Array<int>& ids) override
        {
            ids.add(Open_Sound);
            ids.add(Delete_Sound);
            ids.add(Select_Colour);
            ids.add(separatorBarId);
            ids.add(Enveloppe);
            ids.add(Effects);
            ids.add(Normalize);
            ids.add(Denoise);
            ids.add(separatorBarId);
            ids.add(spacerId);
        }

        void getDefaultItemSet(juce::Array<int>& ids) override
        {
            ids.add(Open_Sound);
            ids.add(Delete_Sound);
            ids.add(Select_Colour);
            ids.add(separatorBarId);
            ids.add(spacerId);
            ids.add(separatorBarId);
            ids.add(Enveloppe);
            ids.add(Effects);
            ids.add(Normalize);
            ids.add(Denoise);
        }
        
        void buttonStateChanged(juce::Button* b)
        {
            auto& modifiers = juce::ModifierKeys::getCurrentModifiers();
            if (modifiers.isRightButtonDown())
                rightClickDown = true;
            else if (modifiers.isCommandDown())
                commandDown = true;
        }

        void buttonClicked(juce::Button* b)
        {
            for (int i = 0; i < labels.size(); i++)
            {
                if (b == labels[i])
                {
                    broadcasters[i]->sendChangeMessage();
                    labels[i]->resetColour();
                }
            }
        }
        juce::ToolbarItemComponent* createItem(int itemId) override
        {
            switch (itemId)
            {
                case Open_Sound:
                {   
                    return labels[0];
                }
                case Delete_Sound:
                {
                    return labels[1];
                }
                case Select_Colour:
                {
                    return labels[2];
                }
                case Enveloppe:
                {
                    return labels[3];
                }
                case Effects:
                {
                    return labels[4];
                }
                case Normalize:
                {
                    return labels[5];
                }
                case Denoise:
                {
                    return labels[6];
                }
            }
        }
        bool rightClickDown = false;
        bool commandDown = false;
        class CustomToolbarLabel : public juce::ToolbarItemComponent
        {
        public:
            CustomToolbarLabel(const int toolbarItemId)
                : ToolbarItemComponent(toolbarItemId, "Custom Toolbar Item", true)
            {
                tLabel.reset(new juce::Label);
                addAndMakeVisible(tLabel.get());

                switch (toolbarItemId)
                {
                case 1 :
                    tLabel->setText("Open Sound", juce::dontSendNotification);
                    break;
                case 2:
                    tLabel->setText("Delete Sound", juce::dontSendNotification);
                    break;
                case 3:
                    tLabel->setText("Sound Colour", juce::dontSendNotification);
                    break;
                case 4:
                    tLabel->setText("Enveloppe", juce::dontSendNotification);
                    break;
                case 5:
                    tLabel->setText("Effects", juce::dontSendNotification);
                    break;
                case 6:
                    tLabel->setText("Normalize", juce::dontSendNotification);
                    break;
                case 7:
                    tLabel->setText("Denoise", juce::dontSendNotification);
                    break;
                }
                tLabel->setJustificationType(juce::Justification::centred);
                tLabel->setInterceptsMouseClicks(false, false);
            }

            bool getToolbarItemSizes(int /*toolbarDepth*/, bool isVertical,
                int& preferredSize, int& minSize, int& maxSize) override
            {
                if (isVertical)
                    return false;

                preferredSize = 100;
                minSize = 100;
                maxSize = 100;
                return true;
            }

            void paintButtonArea(juce::Graphics&, int, int, bool, bool) override
            {
            }

            void contentAreaChanged(const juce::Rectangle<int>& newArea) override
            {
                tLabel->setSize(newArea.getWidth(), newArea.getHeight());

                tLabel->setCentrePosition(newArea.getCentreX(), newArea.getCentreY());
            }

            void mouseEnter(const juce::MouseEvent& event)
            {
                tLabel->setColour(juce::Label::ColourIds::backgroundColourId, getLookAndFeel().findColour(juce::PopupMenu::ColourIds::highlightedBackgroundColourId));
            }            
            
            void mouseExit(const juce::MouseEvent& event)
            {
                tLabel->setColour(juce::Label::ColourIds::backgroundColourId,
                    getLookAndFeel().findColour(juce::Toolbar::backgroundColourId));
            }

            void resetColour()
            {
                tLabel->setColour(juce::Label::ColourIds::backgroundColourId,
                    getLookAndFeel().findColour(juce::Toolbar::backgroundColourId));
            }

            void setTextColour(juce::Colour c)
            {
                tLabel->setColour(juce::Label::ColourIds::textColourId, c);
            }

            //void mouseDown(const juce::MouseEvent& event)
            //{
            //    if (event.mods.isCommandDown())
            //        commandClicked = true;
            //}
            //void mouseUp(const juce::MouseEvent& event)
            //{
            //    commandClicked = false;
            //}
            //bool commandClicked = false;
        private:
            std::unique_ptr<juce::Label> tLabel;
        };
        juce::OwnedArray<CustomToolbarLabel> labels;
        juce::OwnedArray<juce::ChangeBroadcaster> broadcasters;
    };

    TItemFactory factory;
    EnveloppeEditor enveloppeEditor;
    Player* editedPlayer = nullptr;

    bool rightClickDown = false;

    int dividerBarWidth = 4;
    int volumeSliderWidth = 80;
    int inOutButtonWidth = 50;
    int meterWidth = 50;
    int toolBarHeight = 25;
    int bottomButtonHeight = 30;
    int nameLabelHeight = 25;
    int buttonsSpacer = 8;

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
    std::unique_ptr<juce::TextButton> denoiseButton;
    std::unique_ptr<juce::TextButton> openButton;
    std::unique_ptr<juce::TextButton> deleteButton;
    std::unique_ptr<juce::Toolbar> clipToolBar;
    std::unique_ptr<Meter> meter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ClipEditor)
};
