/*
  ==============================================================================

    Settings.cpp
    Created: 12 Feb 2021 10:47:21am
    Author:  Lucien

  ==============================================================================
*/

#include <JuceHeader.h>
#include "Settings.h"


//==============================================================================
Settings::Settings(Playlist* p, Playlist* pbis, juce::OSCSender* sender) : settingsFile(options)
{
    //OPTIONS
    options.applicationName = ProjectInfo::projectName;
    options.filenameSuffix = ".settings";
    options.folderName = juce::File::getSpecialLocation(juce::File::SpecialLocationType::userApplicationDataDirectory).getChildFile("MultiPlayer").getFullPathName();
    options.storageFormat = juce::PropertiesFile::storeAsXML;
    properties.setStorageParameters(options);



    //LOAD PROPERTIES
    FFmpegPath = properties.getUserSettings()->getValue("FFmpeg Path");
    exiftoolPath = properties.getUserSettings()->getValue("Exiftool Path");
    convertedSoundsPath = properties.getUserSettings()->getValue("Converted Sound Path");
    skewFactor = properties.getUserSettings()->getValue("Skew Factor").getFloatValue();
    maxFaderValue = properties.getUserSettings()->getValue("Max Fader Value").getIntValue();
    preferedMidiDeviceIndex = properties.getUserSettings()->getValue("Midi Device").getIntValue();
    midiShift = properties.getUserSettings()->getValue("Midi Shift").getIntValue();
    adress1 = properties.getUserSettings()->getValue("IP Adress 1");
    adress2 = properties.getUserSettings()->getValue("IP Adress 2");
    adress3 = properties.getUserSettings()->getValue("IP Adress 3");
    adress4 = properties.getUserSettings()->getValue("IP Adress 4");
    inOscPort = properties.getUserSettings()->getValue("In OSC Port").getIntValue();
    outOscPort = properties.getUserSettings()->getValue("Out OSC Port").getIntValue();

    //SAVE & CLOSE BUTTON
    saveButton.setBounds(250, 400, 100, 50);
    addAndMakeVisible(saveButton);
    saveButton.setButtonText("Save & Close");
    saveButton.onClick = [this, p, pbis, sender] { setOptions(p, pbis);
                                    connectOSC(sender); saveOptions(); };
    

    //MAX FADER LEVEL
    maxFaderValueSlider.setBounds(150, 0, 450, 25);
    maxFaderValueSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    maxFaderValueSlider.setRange(0, 12, 1);
    maxFaderValueSlider.addListener(this);
    maxFaderValueSlider.setValue(maxFaderValue);
    addAndMakeVisible(maxFaderValueSlider);
    maxFaderValueSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 50, 100);

    

    maxFaderValueLabel.setBounds(0, 0, 150, 25);
    addAndMakeVisible(maxFaderValueLabel);
    maxFaderValueLabel.setText("Fader Maximum Value", juce::NotificationType::dontSendNotification);

    //SkEW FACTOR
    skewFactorSlider.setBounds(200, 50, 300, 25);
    skewFactorSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    skewFactorSlider.setRange(0.1, 1.0, 0.1);
    skewFactorSlider.addListener(this);
    skewFactorSlider.setValue(skewFactor);
    skewFactorSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(skewFactorSlider);


    skewFactorLabel.setBounds(0, 50, 130, 25);
    addAndMakeVisible(skewFactorLabel);
    skewFactorLabel.setText("Fader Acceleration", juce::NotificationType::dontSendNotification);
    skewFactorLabelLeft.setBounds(130, 50, 70, 25);
    addAndMakeVisible(skewFactorLabelLeft);
    skewFactorLabelLeft.setText("Logarithmic", juce::NotificationType::dontSendNotification);
    skewFactorLabelLeft.setJustificationType(juce::Justification::right);
    skewFactorLabelRight.setBounds(500, 50, 100, 25);
    addAndMakeVisible(skewFactorLabelRight);
    skewFactorLabelRight.setText("Linear", juce::NotificationType::dontSendNotification);

    //FMPEG PATH
    ffmpegPathLabel.setBounds(200, 300, 400, 25);
    addAndMakeVisible(ffmpegPathLabel);

    selectFFmpegButton.setBounds(0, 300, 200, 25);
    selectFFmpegButton.setButtonText("Select FFmpeg.exe");
    addAndMakeVisible(selectFFmpegButton);
    selectFFmpegButton.onClick = [this, p, pbis] { selectFFmpeg(p, pbis); };
    ffmpegPathLabel.setText(FFmpegPath, juce::NotificationType::dontSendNotification);

    //EXIFTOOL PATH
    exiftoolLabel.setBounds(200, 350, 400, 25);
    addAndMakeVisible(exiftoolLabel);

    selectExiftoolButton.setBounds(0, 350, 200, 25);
    selectExiftoolButton.setButtonText("Select Exiftool.exe");
    addAndMakeVisible(selectExiftoolButton);
    selectExiftoolButton.onClick = [this, p, pbis] { selectExiftool(p, pbis); };
    exiftoolLabel.setText(exiftoolPath, juce::NotificationType::dontSendNotification);

    //CONVERTED SOUNDS PATH 
    convertedSoundsLabel.setBounds(200, 250, 400, 25);
    addAndMakeVisible(convertedSoundsLabel);
    convertedSoundsLabel.setText(convertedSoundsPath, juce::NotificationType::dontSendNotification);

    convertedSoundsButtons.setBounds(0, 250, 200, 25);
    convertedSoundsButtons.setButtonText("Select converted sounds folder");
    addAndMakeVisible(convertedSoundsButtons);
    convertedSoundsButtons.onClick = [this, p, pbis] { selectSoundsFolder(p, pbis); };

    //MIDI SHIFT
    midiShiftLabel.setBounds(0, 100, 200, 25);
    midiShiftLabel.setText("Midi Channel Shift", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(midiShiftLabel);
    midiShiftValue.setBounds(200, 100, 200, 25);
    midiShiftValue.setText(juce::String(midiShift), juce::NotificationType::dontSendNotification);
    addAndMakeVisible(midiShiftValue);
    midiShiftValue.setEditable(true, true, false);
    midiShiftValue.addListener(this);


    //OSC Ports
    addAndMakeVisible(oscPorts);
    oscPorts.setText("OSC Ports", juce::NotificationType::dontSendNotification);
    oscPorts.setBounds(0, 150, 100, 25);

    //OUT
    addAndMakeVisible(oscOutPortLabel);
    oscOutPortLabel.setText("Outgoing", juce::NotificationType::dontSendNotification);
    oscOutPortLabel.setBounds(200, 150, 100, 25);

    addAndMakeVisible(oscOutPort);
    oscOutPort.setText(juce::String(outOscPort), juce::NotificationType::dontSendNotification);
    oscOutPort.setBounds(300, 150, 100, 25);
    oscOutPort.setEditable(true, true, false);
    oscOutPort.addListener(this);

    //IN
    addAndMakeVisible(oscInPortLabel);
    oscInPortLabel.setText("Ingoing", juce::NotificationType::dontSendNotification);
    oscInPortLabel.setBounds(400, 150, 100, 25);

    addAndMakeVisible(oscInPort);
    oscInPort.setText(juce::String(inOscPort), juce::NotificationType::dontSendNotification);
    oscInPort.setBounds(500, 150, 100, 25);
    oscInPort.setEditable(true, true, false);
    oscInPort.addListener(this);

    //OSC IP Adress Destination
    addAndMakeVisible(ipAdressLabel);
    ipAdressLabel.setBounds(0, 200, 200, 25);
    ipAdressLabel.setText("OSC device Ip Adress", juce::NotificationType::dontSendNotification);

    addAndMakeVisible(ipAdress1);
    ipAdress1.setBounds(200, 200, 50, 25);
    ipAdress1.setText(adress1, juce::NotificationType::sendNotification);
    ipAdress1.setEditable(true, true, false);
    ipAdress1.addListener(this);
    addAndMakeVisible(ipAdress2);
    ipAdress2.setBounds(250, 200, 50, 25);
    ipAdress2.setText(adress2, juce::NotificationType::sendNotification);
    ipAdress2.setEditable(true, true, false);
    ipAdress2.addListener(this);
    addAndMakeVisible(ipAdress3);
    ipAdress3.setBounds(300, 200, 50, 25);
    ipAdress3.setText(adress3, juce::NotificationType::sendNotification);
    ipAdress3.setEditable(true, true, false);
    ipAdress3.addListener(this);
    addAndMakeVisible(ipAdress4);
    ipAdress4.setBounds(350, 200, 50, 25);
    ipAdress4.setText(adress4, juce::NotificationType::sendNotification);
    ipAdress4.setEditable(true, true, false);
    ipAdress4.addListener(this);

    connectOSC(sender);


    setOptions(p, pbis);
    }

Settings::~Settings()
{

}

void Settings::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

}

void Settings::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}

void Settings::selectFFmpeg(Playlist* p, Playlist* pbis)
{
    juce::FileChooser chooser("Select the FFmpeg Executable", juce::File::getSpecialLocation(juce::File::currentApplicationFile), "*FFmpeg.exe");
    if (chooser.browseForFileToOpen())
    {
        juce::File ffmpegExe;
        ffmpegExe = chooser.getResult();

        FFmpegPath = ffmpegExe.getFullPathName();

    }
    ffmpegPathLabel.setText(FFmpegPath, juce::NotificationType::dontSendNotification);
    setOptions(p, pbis);
    //saveOptions();
    //DBG(FFmpegPath);
}

juce::String Settings::getFFmpegPath()
{

    return FFmpegPath;
}

void Settings::selectExiftool(Playlist* p, Playlist* pbis)
{
    juce::FileChooser chooser("Select the Exiftool Executable", juce::File::getSpecialLocation(juce::File::currentApplicationFile), "*exiftool.exe");
    if (chooser.browseForFileToOpen())
    {
        juce::File exiftoolexe;
        exiftoolexe = chooser.getResult();

        exiftoolPath = exiftoolexe.getFullPathName();

    }
    exiftoolLabel.setText(exiftoolPath, juce::NotificationType::dontSendNotification);
    setOptions(p, pbis);
    //saveOptions();
    //DBG(FFmpegPath);
}

juce::String Settings::getExiftoolPath()
{

    return exiftoolPath;
}

void Settings::selectSoundsFolder(Playlist* p, Playlist* pbis)
{
    juce::FileChooser chooser("Select the Directory for converted sounds", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory));
    if (chooser.browseForDirectory())
    {
        juce::File SoundsPath;
        SoundsPath = chooser.getResult();
        
        convertedSoundsPath = SoundsPath.getFullPathName();
    }
    convertedSoundsLabel.setText(convertedSoundsPath, juce::NotificationType::dontSendNotification);
    setOptions(p, pbis);
    //saveOptions();
    //DBG(FFmpegPath);
}

void Settings::setOptions(Playlist* p, Playlist* pbis)
{
    p->setOptions(FFmpegPath, exiftoolPath, convertedSoundsPath, skewFactor, maxFaderValue);
    pbis->setOptions(FFmpegPath, exiftoolPath, convertedSoundsPath, skewFactor, maxFaderValue);
    p->setMidiShift(midiShift);
    pbis->setMidiShift(midiShift);

}

void Settings::saveOptions()
{
    properties.getUserSettings()->setValue("FFmpeg Path", FFmpegPath);
    properties.getUserSettings()->setValue("Exiftool Path", exiftoolPath);
    properties.getUserSettings()->setValue("Converted Sound Path", convertedSoundsPath);
    properties.getUserSettings()->setValue("Skew Factor", skewFactor);
    properties.getUserSettings()->setValue("Max Fader Value", maxFaderValue);
    properties.getUserSettings()->setValue("Midi Device", preferedMidiDeviceIndex);
    properties.getUserSettings()->setValue("Midi Shift", midiShift);
    properties.getUserSettings()->setValue("IP Adress 1", adress1);
    properties.getUserSettings()->setValue("IP Adress 2", adress2);
    properties.getUserSettings()->setValue("IP Adress 3", adress3);
    properties.getUserSettings()->setValue("IP Adress 4", adress4);
    properties.getUserSettings()->setValue("In OSC Port", inOscPort);
    properties.getUserSettings()->setValue("Out OSC Port", outOscPort);
    properties.saveIfNeeded();
    settingsFile.save();
    getParentComponent()->exitModalState(0);
}


void Settings::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &skewFactorSlider)
    {
        skewFactor = skewFactorSlider.getValue();
        
    }
    else if (slider == &maxFaderValueSlider)
    {
        maxFaderValue = maxFaderValueSlider.getValue();
    }
}

void Settings::setPreferedMidiDevice(int midiDeviceIndex)
{
    DBG(midiDeviceIndex);
    preferedMidiDeviceIndex = midiDeviceIndex;
    properties.getUserSettings()->setValue("Midi Device", midiDeviceIndex);
    settingsFile.save();
}

int Settings::getPreferedMidiDevice()
{
    return preferedMidiDeviceIndex;
}

void Settings::labelTextChanged(juce::Label* labelThatHasChanged)
{
    if (labelThatHasChanged == &midiShiftValue)
    {
        midiShift = (midiShiftValue.getTextValue()).toString().getIntValue();
    }
    else if (labelThatHasChanged == &ipAdress1)
    {
        adress1 = ipAdress1.getText();
        properties.getUserSettings()->setValue("IP Adress 1", adress1);
    }
    else if (labelThatHasChanged == &ipAdress2)
    {
        adress2 = ipAdress2.getText();
        properties.getUserSettings()->setValue("IP Adress 2", adress2);
    }
    else if (labelThatHasChanged == &ipAdress3)
    {
        adress3 = ipAdress3.getText();
        properties.getUserSettings()->setValue("IP Adress 3", adress3);
    }
    else if (labelThatHasChanged == &ipAdress4)
    {
        adress4 = ipAdress4.getText();
        properties.getUserSettings()->setValue("IP Adress 4", adress4);
    }
    else if (labelThatHasChanged == &oscOutPort)
    {
        outOscPort = oscOutPort.getText().getIntValue();
        properties.getUserSettings()->setValue("Out OSC Port", outOscPort);
    }
    else if (labelThatHasChanged == &oscInPort)
    {
        inOscPort = oscInPort.getText().getIntValue();
        properties.getUserSettings()->setValue("In OSC Port", inOscPort);
    }
}

void Settings::connectOSC(juce::OSCSender* sender)
{
    ipAdress = juce::String(adress1 + "." + adress2 + "." + adress3 + "." + adress4);
    //DBG(ipAdress);
    sender->connect(ipAdress, outOscPort);
}

juce::String Settings::getIpAdress()
{

    return ipAdress;
}

int Settings::getOutPort()
{
    return outOscPort;
}

int Settings::getInPort()
{
    return inOscPort;
}