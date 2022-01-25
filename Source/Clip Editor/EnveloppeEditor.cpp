#include "EnveloppeEditor.h"
#define BLUE juce::Colour(40, 134, 189)
#define ORANGE juce::Colour(229, 149, 0)
//==============================================================================
EnveloppeEditor::EnveloppeEditor()
{
    /*addPoint(juce::Point<float>(0.0, 0.0));
    myPoints.getLast()->setCanMoveTime(false);
    addPoint(juce::Point<float>(1.0, 0.0));
    myPoints.getLast()->setCanMoveTime(false);*/

    setSize(600, 400);
    constrainer.setMinimumOnscreenAmounts(2, 2, 2, 2);

    addAndMakeVisible(&playHead);
    playHead.setColour(juce::Colours::green);

    addAndMakeVisible(&cuePlayHead);
    cuePlayHead.setColour(juce::Colours::black);

    //addAndMakeVisible(&infoLabel);
    //infoLabel.setBounds(0, 0, 200, 25);

    juce::Timer::startTimerHz(60);
}

EnveloppeEditor::~EnveloppeEditor()
{
}

//==============================================================================
void EnveloppeEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    if (editedPlayer != nullptr)
    {
        //Draw Thumbnail
        g.setColour(BLUE);
        thumbnail->drawChannels(g, thumbnailBounds,
            juce::jlimit<double>(0, thumbnail->getTotalLength(), thumbnailRange.getStart() * thumbnail->getTotalLength()),
            juce::jlimit<double>(0, thumbnail->getTotalLength(), thumbnailRange.getEnd() * thumbnail->getTotalLength()),
            juce::Decibels::decibelsToGain(editedPlayer->getTrimVolume()));

        //Draw X axis
        g.setColour(juce::Colours::white);
        g.drawLine(0, getHeight() / 2, getWidth(), getHeight() / 2, 2);

        //Draw enveloppe line
        g.setColour(juce::Colours::orange);
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
                40, 25, 5);

            g.setColour(juce::Colours::white);
            auto roundedGain = std::ceil(myPoints[pointInfoToDraw]->getYPos() * 12.0 * 10.0) / 10.0;
            g.drawText(juce::String(roundedGain) << "dB",
                xPos, yPos, 40, 25, juce::Justification::centred);
        }
    }
}

void EnveloppeEditor::resized()
{
    for (auto* point : myPoints)
    {
        point->setCentrePosition(getPointPosition(*point));
    }
    cuePlayHead.setSize(2, getHeight());
    playHead.setSize(2, getHeight());
    thumbnailBounds.setBounds(0, 0, getWidth(), getHeight());
    repaint();
}

void EnveloppeEditor::setEditedPlayer(Player* p)
{
    myPoints.clear();
    editedPlayer = p;
    thumbnail = &editedPlayer->getAudioThumbnail();
    thumbnail->addChangeListener(this);
    editedPlayer->trimValueChangedBroacaster->addChangeListener(this);
    createPointsFromPath(editedPlayer->getEnveloppePath());
    resized();
}

bool EnveloppeEditor::keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent)
{
    if (key == juce::KeyPress::spaceKey && editedPlayer != nullptr)
    {
        editedPlayer->cueButtonClicked();
    }
    return true;
}

void EnveloppeEditor::timerCallback()
{//Cue PlayHead
    if (editedPlayer != nullptr)
    {   //transport playhead
        juce::AudioTransportSource* transport = &editedPlayer->transport;
        auto audioPosition = (float)transport->getCurrentPosition();
        auto transportLenght = transport->getLengthInSeconds();
        auto drawPosition = (timeToX(audioPosition / transportLenght));
        playHead.setTopLeftPosition(drawPosition, 0);

        //cue transport playhead
        juce::AudioTransportSource* cueTransport = &editedPlayer->cueTransport;
        auto cueaudioPosition = (float)cueTransport->getCurrentPosition();
        auto cuetransportLenght = cueTransport->getLengthInSeconds();
        auto cuedrawPosition = (timeToX(cueaudioPosition / cuetransportLenght));
        cuePlayHead.setTopLeftPosition(cuedrawPosition, 0);


    }
}

void EnveloppeEditor::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == thumbnail)
        repaint();
    else if (source == editedPlayer->trimValueChangedBroacaster)
        resized();
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

void EnveloppeEditor::mouseDown(const juce::MouseEvent& e)
{
    draggedComponent = nullptr;
    thumbnailDragStart = getMouseXYRelative().getX();
    juce::Point<int> mousePos = e.getEventRelativeTo(this).getPosition();
    for (int i = 0; i < myPoints.size(); i++)
    {
        if (myPoints[i]->getBounds().contains(mousePos))
        {
            if (e.mods.isRightButtonDown())
            {
                deletePoint(myPoints[i]);
                break;
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
    }
    else if (!e.mods.isShiftDown() && !e.mods.isRightButtonDown())
    {
        if (editedPlayer != nullptr)
        {
            double position = editedPlayer->cueTransport.getLengthInSeconds() * getXValue(mousePos.getX());
            editedPlayer->cueTransport.setPosition(position);
            lastPositionClickedorDragged = editedPlayer->cueTransport.getCurrentPosition() / editedPlayer->cueTransport.getLengthInSeconds();
        }
    }
}

void EnveloppeEditor::mouseDrag(const juce::MouseEvent& e)
{
    juce::Point<int> mousePos = e.getEventRelativeTo(this).getPosition();
    int mouseXPos = mousePos.getX();
    int mouseYPos = mousePos.getY();
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
        float valueInDB = getEnveloppeValue(playHeadPosition, *editedPlayer->getEnveloppePath()) * 12.0;
        infoLabel.setText("Position : " + juce::String(playHeadPosition)
            + " Value : " + juce::String(valueInDB) + "dB"
            , juce::NotificationType::dontSendNotification);

        if (editedPlayer != nullptr)
        {
            double position = editedPlayer->cueTransport.getLengthInSeconds() * getXValue(mousePos.getX());
            lastPositionClickedorDragged = editedPlayer->cueTransport.getCurrentPosition() / editedPlayer->cueTransport.getLengthInSeconds();
            editedPlayer->cueTransport.setPosition(juce::jlimit<double>(0.0, editedPlayer->cueTransport.getLengthInSeconds(), position));
        }
    }
    mouseIsDragged = true;
}

void EnveloppeEditor::mouseMove(const juce::MouseEvent& e)
{
    juce::Point<int> mousePos = e.getEventRelativeTo(this).getPosition();

    for (int i = 0; i < myPoints.size(); i++)
    {
        if (myPoints[i]->getBounds().contains(mousePos))
        {
            drawPointInfo = true;
            pointInfoToDraw = i;
            repaint();
            return;
        }
        else
        {
            drawPointInfo = false;
            repaint();
        }
    }
    for (int i = 0; i < linesArray.size(); i++)
    {

    }

}

void EnveloppeEditor::mouseUp(const juce::MouseEvent& e)
{
    setMouseCursor(juce::MouseCursor::NormalCursor);
    thumbnailDragStart = 0;
    drawPointInfo = false;
    mouseIsDragged = false;
    createEnveloppePath();
    resized();
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
    editedPlayer->getEnveloppePath()->clear();
    editedPlayer->getEnveloppePath()->startNewSubPath(0, 0);
    for (auto* point : myPoints)
    {
        editedPlayer->getEnveloppePath()->lineTo(point->getPos());
    }
    editedPlayer->getEnveloppePath()->closeSubPath();

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
            resized();
            return;
        }
    }
}