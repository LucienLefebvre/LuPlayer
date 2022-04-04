/*
  ==============================================================================

    MidiMapper.cpp
    Created: 3 Feb 2022 10:58:09pm
    Author:  Lucien Lefebvre

  ==============================================================================
*/

#include <JuceHeader.h>
#include "MidiMapper.h"

//==============================================================================
MidiMapper::MidiMapper()
{
    setSize(600, 400);

    table.reset(new juce::TableListBox);
    addAndMakeVisible(table.get());
    table->setModel(this);
    table->setBounds(getBounds());
    table->getHeader().addColumn("Action", 1, 400);
    table->getHeader().addColumn("Midi Control Change", 2, 190);
    table->getHeader().setColour(juce::TableHeaderComponent::ColourIds::backgroundColourId, juce::Colour(40, 134, 189));
    table->setWantsKeyboardFocus(false);
    table->setMouseClickGrabsKeyboardFocus(false);

    initializeDefaultMapping();
}

MidiMapper::~MidiMapper()
{
}

void MidiMapper::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
}

void MidiMapper::resized()
{


}

int MidiMapper::getNumRows()
{
    DBG("num rows : " << midiMappings.size());
    return midiMappings.size();
}

void MidiMapper::paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected)
{
    auto alternateColour = getLookAndFeel().findColour(juce::ListBox::backgroundColourId)
        .interpolatedWith(getLookAndFeel().findColour(juce::ListBox::textColourId), 0.03f);
    if (rowIsSelected)
        g.fillAll(juce::Colour(40, 134, 189));
    else if (rowNumber % 2)
        g.fillAll(alternateColour);
}

void MidiMapper::paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
    g.setColour(getLookAndFeel().findColour(juce::ListBox::textColourId));
    if (rowNumber == table->getSelectedRow())
        g.setColour(juce::Colours::black);

    if (columnId == 1)
    {
        juce::String cellstring = midiMappings[rowNumber].commandDescription;
        g.drawText(cellstring, 2, 0, width - 4, height, juce::Justification::centredLeft, true);
    }
    if (columnId == 2)
    {
        if (wantsKeyPress && table->getSelectedRow() == rowNumber)
        {
            g.setColour(juce::Colours::red);
            g.drawText("Press a key...", 2, 0, width - 4, height, juce::Justification::centredLeft, true);
        }
        else
        {
            if (midiMappings[rowNumber].cc != -1)
            {
            juce::String cellstring = juce::String(midiMappings[rowNumber].cc);
            g.drawText(cellstring, 2, 0, width - 4, height, juce::Justification::centredLeft, true);
            }
        }

    }
}

void MidiMapper::cellClicked(int rowNumber, int columnID, const juce::MouseEvent& e)
{
    if (e.getNumberOfClicks() == 2)
    {
        if (!wantsKeyPress)
        {
            wantsKeyPress = true;
        }
        else
        {
            wantsKeyPress = false;
        }
        repaint();
    }
}

void MidiMapper::handleMidiMessage(const juce::MidiMessage& message)
{
    if (wantsKeyPress)
    {
        const juce::MessageManagerLock mmLock;
        wantsKeyPress = false;
        MidiCommandMapping m;
        m.command = midiMappings[table->getSelectedRow()].command;
        m.commandDescription = midiMappings[table->getSelectedRow()].commandDescription;
        m.cc = message.getControllerNumber();

        for (int i = 0; i < midiMappings.size(); ++i)
        {
            if (midiMappings[i].cc == message.getControllerNumber())
            {//delete previous mapping on this cc
                MidiCommandMapping d;
                d.command = midiMappings[i].command;
                d.commandDescription = midiMappings[i].commandDescription;
                d.cc = -1;
                d.faderNumber = -1;
                midiMappings.set(i, d);
            }
        }
        midiMappings.set(table->getSelectedRow(), m);
        table->updateContent();
        repaint();

        saveMidiMapping();
        /*std::unique_ptr<juce::XmlElement> xml = commandManager->getKeyMappings()->createXml(false);
        juce::File keyboardMappingFile(juce::File::getCurrentWorkingDirectory().getChildFile("keyMapping.xml"));
        xml->writeTo(keyboardMappingFile);*/
    }
}

void MidiMapper::addMidiCommand(juce::CommandID command, int midiCC, juce::String description, int faderNumber)
{
    MidiCommandMapping m;
    m.command = command;
    m.cc = midiCC;
    m.commandDescription = description;
    midiMappings.add(m);
}

void MidiMapper::initializeDefaultMapping()
{
    midiMappings.clear();
    addMidiCommand(Fader1Level, 0, "Fader 1 level", 1);
    addMidiCommand(Fader2Level, 1, "Fader 2 level", 2);
    addMidiCommand(Fader3Level, 2, "Fader 3 level", 3);
    addMidiCommand(Fader4Level, 3, "Fader 4 level", 4);
    addMidiCommand(Fader5Level, 4, "Fader 5 level", 5);
    addMidiCommand(Fader6Level, 5, "Fader 6 level", 6);
    addMidiCommand(Fader7Level, 6, "Fader 7 level", 7);
    addMidiCommand(Fader8Level, 7, "Fader 8 level", 8);
    addMidiCommand(Fader1Trim, 16, "Fader 1 trim", 1);
    addMidiCommand(Fader2Trim, 17, "Fader 2 trim", 2);
    addMidiCommand(Fader3Trim, 18, "Fader 3 trim", 3);
    addMidiCommand(Fader4Trim, 19, "Fader 4 trim", 4);
    addMidiCommand(Fader5Trim, 20, "Fader 5 trim", 5);
    addMidiCommand(Fader6Trim, 21, "Fader 6 trim", 6);
    addMidiCommand(Fader7Trim, 22, "Fader 7 trim", 7);
    addMidiCommand(Fader8Trim, 23, "Fader 8 trim", 8);    
    addMidiCommand(Fader1TrimR, 54, "Fader 1 trim relative", 1);
    addMidiCommand(Fader2TrimR, 55, "Fader 2 trim relative", 2);
    addMidiCommand(Fader3TrimR, 56, "Fader 3 trim relative", 3);
    addMidiCommand(Fader4TrimR, 57, "Fader 4 trim relative", 4);
    addMidiCommand(Fader5TrimR, 58, "Fader 5 trim relative", 5);
    addMidiCommand(Fader6TrimR, 59, "Fader 6 trim relative", 6);
    addMidiCommand(Fader7TrimR, 60, "Fader 7 trim relative", 7);
    addMidiCommand(Fader8TrimR, 61, "Fader 8 trim relative", 8);
    addMidiCommand(KeyMap1, -1, "Keymap mode : launch player 1 (1st row)");
    addMidiCommand(KeyMap2, -1, "Keymap mode : launch player 2 (1st row)");
    addMidiCommand(KeyMap3, -1, "Keymap mode : launch player 3 (1st row)");
    addMidiCommand(KeyMap4, -1, "Keymap mode : launch player 4 (1st row)");
    addMidiCommand(KeyMap5, -1, "Keymap mode : launch player 5 (1st row)");
    addMidiCommand(KeyMap6, -1, "Keymap mode : launch player 6 (1st row)");
    addMidiCommand(KeyMap7, -1, "Keymap mode : launch player 7 (1st row)");
    addMidiCommand(KeyMap8, -1, "Keymap mode : launch player 8 (1st row)");
    addMidiCommand(KeyMap9, -1, "Keymap mode : launch player 9 (1st row)");
    addMidiCommand(KeyMap10, -1, "Keymap mode : launch player 10 (1st row)");   
    addMidiCommand(KeyMap11, -1, "Keymap mode : launch player 1 (2nd row)");
    addMidiCommand(KeyMap12, -1, "Keymap mode : launch player 2 (2nd row)");
    addMidiCommand(KeyMap13, -1, "Keymap mode : launch player 3 (2nd row)");
    addMidiCommand(KeyMap14, -1, "Keymap mode : launch player 4 (2nd row)");
    addMidiCommand(KeyMap15, -1, "Keymap mode : launch player 5 (2nd row)");
    addMidiCommand(KeyMap16, -1, "Keymap mode : launch player 6 (2nd row)");
    addMidiCommand(KeyMap17, -1, "Keymap mode : launch player 7 (2nd row)");
    addMidiCommand(KeyMap18, -1, "Keymap mode : launch player 8 (2nd row)");
    addMidiCommand(KeyMap19, -1, "Keymap mode : launch player 9 (2nd row)");
    addMidiCommand(KeyMap20, -1, "Keymap mode : launch player 10 (2nd row)");    
    addMidiCommand(KeyMap21, -1, "Keymap mode : launch player 1 (3rd row)");
    addMidiCommand(KeyMap22, -1, "Keymap mode : launch player 2 (3rd row)");
    addMidiCommand(KeyMap23, -1, "Keymap mode : launch player 3 (3rd row)");
    addMidiCommand(KeyMap24, -1, "Keymap mode : launch player 4 (3rd row)");
    addMidiCommand(KeyMap25, -1, "Keymap mode : launch player 5 (3rd row)");
    addMidiCommand(KeyMap26, -1, "Keymap mode : launch player 6 (3rd row)");
    addMidiCommand(KeyMap27, -1, "Keymap mode : launch player 7 (3rd row)");
    addMidiCommand(KeyMap28, -1, "Keymap mode : launch player 8 (3rd row)");
    addMidiCommand(KeyMap29, -1, "Keymap mode : launch player 9 (3rd row)");
    addMidiCommand(KeyMap30, -1, "Keymap mode : launch player 10 (3rd row)");

    table->updateContent();
    repaint();
}

int MidiMapper::getMidiCCForCommand(juce::CommandID c)
{
    for (auto commandMapping : midiMappings)
    {
        if (commandMapping.command == c)
            return commandMapping.cc;
    }
}

int MidiMapper::getFaderNumberForCommand(juce::CommandID c)
{
    for (auto commandMapping : midiMappings)
    {
        if (commandMapping.command == c
            && commandMapping.faderNumber != -1)
            return commandMapping.faderNumber;
    }
}

bool MidiMapper::getWantsKeyPress()
{
    return wantsKeyPress;
}


void MidiMapper::setWantsKeyPress(bool b)
{
    wantsKeyPress = b;
}

void MidiMapper::setCommandManager(juce::ApplicationCommandManager* cm)
{
    commandManager = cm;
    juce::StringArray commandCategories = commandManager->getCommandCategories();
    juce::Array<int> commandUIDS;
    for (auto categorie : commandCategories)
    {
        commandUIDS.addArray(commandManager->getCommandsInCategory(categorie));
    }
    for (auto commandUID : commandUIDS)
    {
        juce::String des = commandManager->getCommandForID(commandUID)->description;
        juce::CommandID id = commandManager->getCommandForID(commandUID)->commandID;
        addMidiCommand(id, -1, des);
    }
}

void MidiMapper::saveMidiMapping()
{
    juce::XmlElement midiMapping("MIDIMAPPING");
    for (int i = 0; i < midiMappings.size(); ++i)
    {
        auto* command = new juce::XmlElement("CommandMapping");
        command->setAttribute("Description", midiMappings[i].commandDescription);
        command->setAttribute("CommandID", midiMappings[i].command);
        command->setAttribute("MidiCC", midiMappings[i].cc);
        if (command != nullptr)
            midiMapping.addChildElement(command);
    }

    juce::File midiMappingFile(juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("Multiplayer/midiMapping.xml"));
    midiMapping.writeTo(midiMappingFile);
    DBG("a");
}

void MidiMapper::loadMappingFile()
{
    juce::File midiMappingFile(juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("Multiplayer/midiMapping.xml"));
    if (midiMappingFile.exists())
    {
        midiMappings.clear();
        if (auto xml = parseXML(midiMappingFile))
        {
            if (xml->hasTagName("MIDIMAPPING"))
            {
                forEachXmlChildElement(*xml, n)
                {
                    if (n->hasTagName("CommandMapping"))
                    {
                        MidiCommandMapping m;
                        m.command = n->getIntAttribute("CommandID");
                        m.cc = n->getIntAttribute("MidiCC");
                        m.commandDescription = n->getStringAttribute("Description");
                        midiMappings.add(m);
                    }
                }
            }
        }
    }
}