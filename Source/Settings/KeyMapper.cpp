/*
  ==============================================================================

    KeyMapper.cpp
    Created: 1 Feb 2022 1:46:49pm
    Author:  DPR

  ==============================================================================
*/

#include <JuceHeader.h>
#include "KeyMapper.h"

//==============================================================================
KeyMapper::KeyMapper(Settings* s)
{
    setSize(600, 400);

    addKeyListener(this);

    settings = s;

    table.reset(new juce::TableListBox);
    addAndMakeVisible(table.get());
    table->setModel(this);
    table->setBounds(getBounds());
    table->getHeader().addColumn("Action", 1, 400);
    table->getHeader().addColumn("Shortcut", 2, 190);
    table->getHeader().setColour(juce::TableHeaderComponent::ColourIds::backgroundColourId, juce::Colour(40, 134, 189));
}

KeyMapper::~KeyMapper()
{
}

void KeyMapper::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
}

void KeyMapper::resized()
{
    int saveButtonHeight = 25;
    //table->setSize(600, 400 - saveButtonHeight);
}

int KeyMapper::getNumRows()
{
    return keyMapCommands.size();
}

void KeyMapper::paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected)
{
    auto alternateColour = getLookAndFeel().findColour(juce::ListBox::backgroundColourId)
        .interpolatedWith(getLookAndFeel().findColour(juce::ListBox::textColourId), 0.03f);
    if (rowIsSelected)
        g.fillAll(juce::Colour(40, 134, 189));
    else if (rowNumber % 2)
        g.fillAll(alternateColour);
}

void KeyMapper::paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
    g.setColour(getLookAndFeel().findColour(juce::ListBox::textColourId));
    if (rowNumber == table->getSelectedRow())
        g.setColour(juce::Colours::black);

    if (columnId == 1)
    {
        g.drawText(keyMapCommands[rowNumber], 2, 0, width - 4, height, juce::Justification::centredLeft, true);
    }
    if (columnId == 2)
    {
        juce::String key = juce::KeyPress(keyMapping[rowNumber]).getTextDescription();
        if (wantsKeyPress && table->getSelectedRow() == rowNumber)
        {
            g.setColour(juce::Colours::red);
            g.drawText("Press a key...", 2, 0, width - 4, height, juce::Justification::centredLeft, true);
        }
        else
        g.drawText(key, 2, 0, width - 4, height, juce::Justification::centredLeft, true);
    }
}

void KeyMapper::cellClicked(int rowNumber, int columnID, const juce::MouseEvent& e)
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

juce::Array<int> KeyMapper::getKeyMapping()
{
    return keyMapping;
}

int KeyMapper::getKeyMapping(int command)
{
    return keyMapping[command];
}

void KeyMapper::setKeyMapping(juce::Array<int> m)
{
    keyMapping = m;
}

bool KeyMapper::keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent)
{
    if (wantsKeyPress)
    {
        keyMapping.set(table->getSelectedRow(), key.getKeyCode());
        wantsKeyPress = false;
        table->updateContent();
        repaint();
        settings->setKeyMapping(keyMapping, keyMapCommands);
    }
    return false;
}

bool KeyMapper::getWantsKeyPress()
{
    return wantsKeyPress;
}

void KeyMapper::loadKeyMapping()
{
    keyMapping = settings->getKeyMapping(keyMapCommands);
    if (keyMapping[0] == 0 && keyMapping[1] == 0)
        keyMapping = defaultMapping;
}