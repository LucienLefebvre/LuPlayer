#include "EnveloppeEditor.h"
#include <random>
#define BLUE juce::Colour(40, 134, 189)
#define ORANGE juce::Colour(229, 149, 0)

//==============================================================================
EnveloppeEditor::EnveloppeEditor()
{
    constrainer.setMinimumOnscreenAmounts(2, 2, 2, 2);

    soundColour = BLUE;

    juce::Timer::startTimerHz(30);

    setBufferedToImage(true);
}

EnveloppeEditor::~EnveloppeEditor()
{
    setNullPlayer();
}

//==============================================================================
void EnveloppeEditor::paint(juce::Graphics& g)
{
    DBG("paint");
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    if (editedPlayer != nullptr)
    {
        if (editedPlayer->getColourHasChanged())
        {
            g.setColour(editedPlayer->getPlayerColour().darker(1.0));
            g.setOpacity(0.2);
            g.fillRoundedRectangle(getLocalBounds().toFloat(), 15);
        }

        //Draw X axis
        g.setColour(juce::Colours::white);
        g.drawLine(0, getHeight() / 2, getWidth(), getHeight() / 2, 2);

        //Draw Thumbnail
        juce::Array<float> gainValue;
        for (int i = 0; i < thumbnailBounds.getWidth(); i++)
        {
            if (editedPlayer->isEnveloppeEnabled())
            {
                float value = juce::Decibels::decibelsToGain(editedPlayer->getEnveloppeValue(getXValue(i), *editedPlayer->getEnveloppePath()) * 24);
                gainValue.set(i, value);
            }
            else
            {
                gainValue.set(i, 1.0f);
            }
        }
        thumbnail->setGainValues(gainValue);

        g.setColour(soundColour);
        thumbnail->drawChannels(g, thumbnailBounds,
            juce::jlimit<double>(0, thumbnail->getTotalLength(), thumbnailRange.getStart() * thumbnail->getTotalLength()),
            juce::jlimit<double>(0, thumbnail->getTotalLength(), thumbnailRange.getEnd() * thumbnail->getTotalLength()),
            juce::Decibels::decibelsToGain(editedPlayer->getTrimVolume()));

        //Draw dB lines
        /*g.setColour(juce::Colours::lightgrey);
        for (auto i = 0; i < dBLines.size(); i++)
        {
            auto yPosition = gainToY(dBLines[i]);
            g.setOpacity(0.05);
            g.drawHorizontalLine(yPosition, 0, getWidth());
            g.setOpacity(0.5);
            g.drawText(juce::String(dBLines[i] * 24) << "dB", getWidth() - 40, yPosition - 16,
                40, 15, juce::Justification::centredRight);
        }*/
        
        //Draw time lines
        g.setColour(juce::Colours::white);
        for (int i = 1; i < timeLines.size(); i++)
        {
            if (getRangeInSeconds(thumbnailRange) > 500)
            {
                drawTimeLines(g, i * 300);
            }
            else if (getRangeInSeconds(thumbnailRange) > 100)
            {
                drawTimeLines(g, i * 60);
            }
            else if (getRangeInSeconds(thumbnailRange) > 30)
            {
                drawTimeLines(g, i * 10);
            }
            else if (getRangeInSeconds(thumbnailRange) < 30)
            {
                drawTimeLines(g, i);
            }
        }

        //Draw enveloppe line
        g.setOpacity(1);
        if (editedPlayer->isEnveloppeEnabled())
            g.setColour(juce::Colours::orange);
        else
            g.setColour(juce::Colours::grey);
        for (int i = 0; i < myPoints.size() - 1; i++)
        {
            juce::Point<int> startPoint = getPointPosition(*myPoints[i]);
            juce::Point<int> endPoint = getPointPosition(*myPoints[i + 1]);
            g.drawLine(startPoint.x, startPoint.y, endPoint.x, endPoint.y, 2);
        }

        //Draw Point info
        if (drawPointInfo == true)
        {
            //Position of the rectangle
            int xPos = myPoints[pointInfoToDraw]->getRight() + 4;
            int yPos = myPoints[pointInfoToDraw]->getBottom() + 4;
            if (xPos > getWidth() - 52)
                xPos = getWidth() - 52;
            if (yPos > getHeight() - 40)
                yPos = getHeight() - 40;
            g.setColour(juce::Colours::white);
            g.setOpacity(0.15);
            g.fillRoundedRectangle(xPos, yPos,
                50, 25, 5);

            g.setColour(juce::Colours::white);
            auto roundedGain = std::ceil(myPoints[pointInfoToDraw]->getYPos() * (float)scale * 10.0) / 10.0;
            g.drawText(juce::String(roundedGain) << "dB",
                xPos, yPos, 50, 25, juce::Justification::centred);
        }
    }
}

void EnveloppeEditor::drawTimeLines(juce::Graphics& g, int multiplier)
{
    g.setOpacity(0.1);
    if ((multiplier % 60) == 0)
        g.setOpacity(0.3);
    else if ((multiplier % 10) == 0)
        g.setOpacity(0.2);
    int xPos = timeToX(timeLines[multiplier] / editedPlayer->getLenght());
    g.drawVerticalLine(xPos, 0, getHeight());
    g.setOpacity(0.5);
    g.drawText(secondsToMMSS(timeLines[multiplier]), xPos + 2, 0,
        50, 15, juce::Justification::centredLeft);
}

void EnveloppeEditor::resized()
{
    for (auto* point : myPoints)
    {
        point->setCentrePosition(getPointPosition(*point));
    }
    fixFirstAndLastPoint();
    cuePlayHead.setSize(2, getHeight());
    playHead.setSize(2, getHeight());
    inMark.setSize(2, getHeight());
    outMark.setSize(2, getHeight());
    thumbnailBounds.setBounds(0, 0, getWidth(), getHeight());
    scaleButton.setBounds(getWidth() - scaleButtonWidth, getHeight() - scaleButtonHeight, scaleButtonWidth, scaleButtonHeight);
    DBG("resized");
    repaint();
}

void EnveloppeEditor::setEditedPlayer(Player* p)
{
    if (editedPlayer != nullptr)
    {
        editedPlayer->trimValueChangedBroacaster->removeChangeListener(this);
        editedPlayer->soundEditedBroadcaster->removeChangeListener(this);
        thumbnail->removeChangeListener(this);
        thumbnail = nullptr;
    }
    myPoints.clear();
    editedPlayer = p;
    thumbnail = &editedPlayer->getAudioThumbnail();
    thumbnail->addChangeListener(this);
    editedPlayer->trimValueChangedBroacaster->addChangeListener(this);
    editedPlayer->soundEditedBroadcaster->addChangeListener(this);
    thumbnailHorizontalZoom = 1.0;
    thumbnailRange = thumbnailRange.withStartAndLength(0.0, 1.0);
    createPointsFromPath(editedPlayer->getEnveloppePath());
    createTimeLines();
    resized();
}

void EnveloppeEditor::timerCallback()
{
    if (editedPlayer != nullptr)
    {
        if (myPoints[1] != nullptr)
        {//generate random position for debugging
            /*std::random_device rd;
            std::mt19937 mt(rd());
            std::uniform_real_distribution<double> dist(0.0, 1.0);
            std::uniform_real_distribution<double> disty(-1.0, 1.0);
            myPoints[1]->setXpos(dist(mt));
            myPoints[1]->setYpos(disty(mt));
            resized();*/
        }
    }
}

void EnveloppeEditor::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == thumbnail)
        repaint();
    if (editedPlayer != nullptr)
    {
        if (source == editedPlayer->trimValueChangedBroacaster)
            resized();
        else if (source == editedPlayer->soundEditedBroadcaster)
        {
            setPointsColour(editedPlayer->isEnveloppeEnabled() ? juce::Colours::red : juce::Colours::grey);
            resized();
        }
    }
}

juce::Point<int> EnveloppeEditor::getPointPosition(EnveloppePoint& point)
{
    juce::Point<int> p;
    p.x =  getWidth() * (float)((point.getXPos() - thumbnailRange.getStart()) / thumbnailRange.getLength());
    p.y = (getHeight() * (1 - point.getYPos())) / 2;
    return p;
}

juce::Point<float> EnveloppeEditor::getPointValue(juce::Point<int> position)
{
    juce::Point<float> p;
    p.x = float(position.x) / float(getWidth()) * (thumbnailRange.getLength()) + thumbnailRange.getStart();
    p.y = 1 - 2 * ((position.y) / float(getHeight()));
    return p;
}

float EnveloppeEditor::getXValue(int x)
{//give a value between 0 and 1 for a x position
    return float(x) / float(getWidth()) * (thumbnailRange.getLength()) + thumbnailRange.getStart();
}

int EnveloppeEditor::timeToX(float time)
{//give a x position for a value between 0 and 1
    return getWidth() * (float)((time - thumbnailRange.getStart()) / thumbnailRange.getLength());
}

float EnveloppeEditor::getYValue(int y)
{
    return 1 - 2 * ((y) / float(getHeight()));
}

int EnveloppeEditor::gainToY(float gain)
{
    return (getHeight() * (1 - gain)) / 2;
}

void EnveloppeEditor::mouseDown(const juce::MouseEvent& e)
{
    if (e.getNumberOfClicks() == 2)
    {
        thumbnailHorizontalZoom = 1.0;
        thumbnailRange = thumbnailRange.withStartAndLength(0.0, 1.0);
        resized();
    }
    draggedComponent = nullptr;
    thumbnailDragStart = getMouseXYRelative().getX();
    juce::Point<int> mousePos = e.getEventRelativeTo(this).getPosition();
    for (int i = 0; i < myPoints.size(); i++)
    {
        if (myPoints[i]->getBounds().contains(mousePos))
        {
            if (e.mods.isRightButtonDown())
            {
                if (myPoints[i]->getCanMoveTime())
                {
                    deletePoint(myPoints[i]);
                    break;
                }
            }
            else
                draggedComponent = myPoints[i];
        }
    }

    if (draggedComponent)
        myDragger.startDraggingComponent(draggedComponent, e);
    else if (e.mods.isCtrlDown())
    {
        juce::Point<float> p = getPointValue(mousePos);
        draggedComponent = addPoint(p);
        if (!editedPlayer->isEnveloppeEnabled())
            editedPlayer->setEnveloppeEnabled(true);
    }
    else if (!e.mods.isShiftDown() && !e.mods.isRightButtonDown())
    {
        if (editedPlayer != nullptr)
        {
            double position = editedPlayer->cueTransport.getLengthInSeconds() * getXValue(mousePos.getX());
            editedPlayer->cueTransport.setPosition(position);
            lastPositionClickedorDragged = editedPlayer->cueTransport.getCurrentPosition() / editedPlayer->cueTransport.getLengthInSeconds();
            editedPlayer->updateCuePlayHeadPosition(true);
        }
    }
}

void EnveloppeEditor::mouseDrag(const juce::MouseEvent& e)
{
    juce::Point<int> mousePos = e.getEventRelativeTo(this).getPosition();
    int mouseXPos = mousePos.getX();
    int mouseYPos = mousePos.getY();
    if (editedPlayer != nullptr)
    {
        if (draggedComponent)
        {
            EnveloppePoint* draggedPoint = dynamic_cast<EnveloppePoint*>(draggedComponent);
            int indexOfDraggedPoint = myPoints.indexOf(draggedPoint);
            drawPointInfo = true;
            pointInfoToDraw = indexOfDraggedPoint;
            if (myPoints[indexOfDraggedPoint + 1] != nullptr && myPoints[indexOfDraggedPoint - 1] != nullptr)
            {

                int previousPointX = myPoints[indexOfDraggedPoint - 1]->getX() + draggedPoint->getRadius();
                int nextPointX = myPoints[indexOfDraggedPoint + 1]->getX() + draggedPoint->getRadius();


                if (mouseXPos >= previousPointX && mouseXPos <= nextPointX)
                {
                    draggedPoint->setPosition(getPointValue(mousePos));
                }
                else if (mouseXPos < previousPointX)
                {
                    draggedPoint->setPosition(getXValue(previousPointX), getYValue(mouseYPos));
                }
                else if (mouseXPos > nextPointX)
                {
                    draggedPoint->setPosition(getXValue(nextPointX), getYValue(mouseYPos));
                }
                createEnveloppePath();
                resized();
            }
        }
        else if (e.mods.isShiftDown())
        {
            setMouseCursor(juce::MouseCursor::DraggingHandCursor);
            int dragValue = thumbnailDragStart - getMouseXYRelative().getX();
            thumbnailDragStart = getMouseXYRelative().getX();
            float rangeDragValue = (float)dragValue / (float)getWidth() * thumbnailRange.getLength();
            float thumbnailRangeStart = thumbnailRange.getStart() + rangeDragValue;

            thumbnailRange = thumbnailRange.movedToStartAt(juce::jlimit<double>(0.0, 1.0, thumbnailRangeStart));
            if (thumbnailRange.getEnd() > 1.0)
                thumbnailRange.setEnd(1.0);
            resized();
        }
        else
        {
            int playHeadXPos = juce::jlimit<int>(0, getWidth(), mousePos.getX());
            float playHeadPosition = float(playHeadXPos) / getWidth();
            float valueInDB = getEnveloppeValue(playHeadPosition, *editedPlayer->getEnveloppePath()) * (float)scale;
            infoLabel.setText("Position : " + juce::String(playHeadPosition)
                + " Value : " + juce::String(valueInDB) + "dB"
                , juce::NotificationType::dontSendNotification);

            if (editedPlayer != nullptr)
            {
                double position = editedPlayer->cueTransport.getLengthInSeconds() * getXValue(mousePos.getX());
                lastPositionClickedorDragged = editedPlayer->cueTransport.getCurrentPosition() / editedPlayer->cueTransport.getLengthInSeconds();
                editedPlayer->cueTransport.setPosition(juce::jlimit<double>(0.0, editedPlayer->cueTransport.getLengthInSeconds(), position));
                editedPlayer->updateCuePlayHeadPosition(true);
            }
        }
    }
    mouseIsDragged = true;
}

void EnveloppeEditor::mouseMove(const juce::MouseEvent& e)
{
    juce::Point<int> mousePos = e.getEventRelativeTo(this).getPosition();
    if (editedPlayer != nullptr)
    {
        for (int i = 0; i < myPoints.size(); i++)
        {
            if (myPoints[i]->getBounds().contains(mousePos))
            {
                drawPointInfo = true;
                pointInfoToDraw = i;
                if (myPoints[i]->getPos() != myPoints[i]->getLastPointPosition())
                {
                    DBG("point repaint");
                    repaint();
                }

                myPoints[i]->setLastPointPosition(myPoints[i]->getPos());
                return;
            }
            else if (drawPointInfo == true)
            {
                drawPointInfo = false;
                repaint();
            }
        }
    }
    lastMousePos = mousePos;
}

void EnveloppeEditor::mouseUp(const juce::MouseEvent& e)
{
    if (editedPlayer != nullptr)
    {
        setMouseCursor(juce::MouseCursor::NormalCursor);
        thumbnailDragStart = 0;
        drawPointInfo = false;
        mouseIsDragged = false;
        createEnveloppePath();
        resized();
    }
}

void EnveloppeEditor::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel)
{
    thumbnailHorizontalZoom = juce::jlimit<float>(1, 1000, thumbnailHorizontalZoom * (2 + wheel.deltaY) / 2);
    if (editedPlayer != nullptr)
    {
        middleRangePoint = editedPlayer->cueTransport.getCurrentPosition() / editedPlayer->cueTransport.getLengthInSeconds();
        middleRangePoint = lastPositionClickedorDragged;
    }
    thumbnailRange.setStart(middleRangePoint - (middleRangePoint / thumbnailHorizontalZoom));
    thumbnailRange.setEnd(middleRangePoint + (middleRangePoint / thumbnailHorizontalZoom));
    thumbnailRange.setLength(1.0 / thumbnailHorizontalZoom);

    resized();
}

EnveloppePoint* EnveloppeEditor::addPoint(juce::Point<float> position, bool createPath)
{
    EnveloppePoint* p;
    myPoints.add(new EnveloppePoint(position.x, position.y));
    addAndMakeVisible(myPoints.getLast());
    myPoints.getLast()->setCentrePosition(getPointPosition(*myPoints.getLast()));
    myPoints.getLast()->addMouseListener(this, false);
    p = myPoints.getLast();
    myPoints.sort(comparator, true);
    if (createPath)
        createEnveloppePath();
    resized();
    return p;
}

void EnveloppeEditor::deletePoint(EnveloppePoint* point)
{
    if (point->getCanMoveTime())
    {
        point->removeMouseListener(this);
        myPoints.removeObject(point);
        createEnveloppePath();
        resized();
    }
}

void EnveloppeEditor::sortPoints()
{//sort points in the owned array by x position
    myPoints.sort(comparator, true);

    createEnveloppePath();
    resized();
}

std::atomic<float> EnveloppeEditor::getEnveloppeValue(float x, juce::Path& p)
{//get the enveloppe value (between 0 and 1) for a given x position (between 0 and 1)
    std::atomic<float> value = 0.0;

    juce::Path::Iterator iterator(p);
    //creates an array of lines representing the enveloppe
    float lineStartX = 0.0;
    float lineStartY = 0.0;
    float lineEndX = 0.0;
    float lineEndY = 0.0;
    linesArray.clear();
    while (iterator.next())
    {
        if (iterator.elementType == juce::Path::Iterator::PathElementType::lineTo)
        {
            lineEndX = iterator.x1;
            lineEndY = iterator.y1;
            linesArray.add(juce::Line<float>(lineStartX, lineStartY, lineEndX, lineEndY));
            lineStartX = lineEndX;
            lineStartY = lineEndY;
        }
    }

    juce::Line<float> valueLine(x, 1.0, x, -1.0); //playhead line

    for (int i = 0; i < linesArray.size(); i++)
    {//for each line in the array, check if it intersect with the playhead
        if (linesArray[i].intersects(valueLine))
        {
            //If yes, get the y value corresponding
            juce::Point<float> intersectPoint = linesArray[i].getIntersection(valueLine);
            value.store(intersectPoint.getY());
        }
    }
    return value.load();
}

void EnveloppeEditor::createEnveloppePath()
{//creates a path from the enveloppe points
    if (editedPlayer != nullptr)
    {
        editedPlayer->getEnveloppePath()->clear();
        editedPlayer->getEnveloppePath()->startNewSubPath(0, 0);
        for (auto* point : myPoints)
        {
            editedPlayer->getEnveloppePath()->lineTo(point->getPos());
        }
        editedPlayer->getEnveloppePath()->closeSubPath();
        editedPlayer->enveloppePathChangedBroadcaster->sendChangeMessage();
        editedPlayer->repaint();
    }

    fixFirstAndLastPoint();
    /*if (editedPlayer != nullptr)
        editedPlayer->setEnveloppePath(enveloppePath);*/
}

void EnveloppeEditor::createPointsFromPath(juce::Path* p)
{
    juce::Path::Iterator iterator(*p);

    while (iterator.next())
    {
        if (iterator.elementType == juce::Path::Iterator::PathElementType::startNewSubPath)
        {
            juce::Point<float> point(iterator.x1, iterator.y1);
            addPoint(point, false);
        }
        else if (iterator.elementType == juce::Path::Iterator::PathElementType::lineTo)
        {
            juce::Point<float> point(iterator.x1, iterator.y1);
            addPoint(point, false);
        }
        else if (iterator.elementType == juce::Path::Iterator::PathElementType::closePath)
        {
            //fixFirstAndLastPoint();
            resized();
            return;
        }
    }

}

void EnveloppeEditor::setNullPlayer()
{
    editedPlayer = nullptr;
    myPoints.clear();
    outMark.setVisible(false);
    inMark.setVisible(false);
    playHead.setTopLeftPosition(0, 0);
    cuePlayHead.setTopLeftPosition(0, 0);
    resized();
}

void EnveloppeEditor::createDefaultEnveloppePath()
{
    enveloppePath.clear();
    enveloppePath.startNewSubPath(0.0, 0.0);
    enveloppePath.lineTo(1.0, 0.0);
    enveloppePath.closeSubPath();
}

void EnveloppeEditor::scaleButtonClicked()
{
    switch (scale)
    {
    case 6:
        scale = 12;
        scaleButton.setButtonText("+-12dB");
        break;
    case 12 : 
        scale = 24;
        scaleButton.setButtonText("+-24dB");
        break;
    case 24:
        scale = 6;
        scaleButton.setButtonText("+-6dB");
        break;
    }
    resized();
}

void EnveloppeEditor::spaceBarPressed()
{
    if (editedPlayer != nullptr)
        editedPlayer->cueButtonClicked();
}

juce::Array<int> EnveloppeEditor::createTimeLines()
{
    if (editedPlayer != nullptr)
    {
        auto lenght = editedPlayer->getLenght();
        timeLines.clear();
        int i = 0;
        while (i < lenght)
        {
            timeLines.add(i);
            i += 1;
        }
    }
    return timeLines;
}

juce::String EnveloppeEditor::secondsToMMSS(int seconds)
{
    int timeSeconds = seconds % 60;
    int timeMinuts = trunc(seconds / 60);
    juce::String timeString;
    if (timeSeconds < 10)
        timeString = juce::String(timeMinuts) + ":0" + juce::String(timeSeconds);
    else
        timeString = juce::String(timeMinuts) + ":" + juce::String(timeSeconds);
    return timeString;
}

float EnveloppeEditor::getRangeInSeconds(juce::Range<double>& r)
{
    return r.getLength() * editedPlayer->getLenght();
}

void EnveloppeEditor::setPointsColour(juce::Colour c)
{
    for (auto* p : myPoints)
    {
        p->setPointColour(c);
    }
}

void EnveloppeEditor::setOrDeleteStart(bool setOrDelete)
{
    if (editedPlayer != nullptr)
    {
        if (setOrDelete)
            editedPlayer->setStart();
        else
            editedPlayer->deleteStart();
    }
}

void EnveloppeEditor::setOrDeleteStop(bool setOrDelete)
{
    if (editedPlayer != nullptr)
    {
        if (setOrDelete)
            editedPlayer->setStop();
        else
            editedPlayer->deleteStop();
    }
}

void EnveloppeEditor::setSoundColour(juce::Colour c)
{
    soundColour = c;
    repaint();
}

void EnveloppeEditor::fixFirstAndLastPoint()
{
    for (auto* point : myPoints)
        point->setFixedPoint(false);
    if (myPoints.getFirst() != nullptr)
        myPoints.getFirst()->setFixedPoint(true);
    if (myPoints.getLast() != nullptr)
        myPoints.getLast()->setFixedPoint(true);
}