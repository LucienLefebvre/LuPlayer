for (auto i = 0; i < myPlaylists[0]->players.size(); i++)
{
    if ((getMouseXYRelative().getX() < playlistViewport.getWidth())
        && (getMouseXYRelative().getX() > 0)
        && (getMouseXYRelative().getY() > (playlistViewport.getPosition().getY() - playlistViewport.getViewPositionY() + (105 * i) + dragZoneHeight)
            && (getMouseXYRelative().getY() < (playlistViewport.getPosition().getY() - playlistViewport.getViewPositionY() + (105 * (i + 1) - dragZoneHeight)))))
    {

        playerMouseDragUp = i;
        playlistDragDestination = 0;
        myPlaylists[0]->fileDragPaintRectangle = true;
        myPlaylists[0]->fileDragPaintLine = false;
        myPlaylists[0]->fileDragPlayerDestination = playerMouseDragUp;
        myPlaylists[0]->repaint();
        destinationPlayerFind = true;
    }
    else if (getMouseXYRelative().getY() > (playlistViewport.getPosition().getY() - playlistViewport.getViewPositionY() + (105 * i) + 100 - dragZoneHeight)
        && getMouseXYRelative().getY() < (playlistViewport.getPosition().getY() - playlistViewport.getViewPositionY() + (105 * i) + 100 + dragZoneHeight)
        && getMouseXYRelative().getX() < playlistViewport.getWidth()
        && (getMouseXYRelative().getX() > 0))
    {
        playerMouseDragUp = i;
        playlistDragDestination = 0;
        myPlaylists[0]->fileDragPaintLine = true;
        myPlaylists[0]->fileDragPaintRectangle = false;
        myPlaylists[0]->fileDragPlayerDestination = playerMouseDragUp;
        myPlaylists[0]->repaint();
        destinationPlayerFind = true;
    }
    else if (getMouseXYRelative().getY() < (playlistViewport.getPosition().getY() - playlistViewport.getViewPositionY() + dragZoneHeight)
        && getMouseXYRelative().getY() > (playlistViewport.getPosition().getY() - playlistViewport.getViewPositionY() - dragZoneHeight)
        && getMouseXYRelative().getX() < playlistViewport.getWidth()
        && (getMouseXYRelative().getX() > 0))
    {
        insertTop = true;
        playerMouseDragUp = -1;
        playlistDragDestination = 0;
        myPlaylists[0]->fileDragPaintLine = true;
        myPlaylists[0]->fileDragPaintRectangle = false;
        myPlaylists[0]->fileDragPlayerDestination = playerMouseDragUp;
        myPlaylists[0]->repaint();
        destinationPlayerFind = true;
    }
    else if (getMouseXYRelative().getY() < playlistViewport.getPosition().getY()
        || getMouseXYRelative().getY() > myPlaylists[0]->players.size() * 105
        || getMouseXYRelative().getX() > playlistViewport.getWidth()
        || (getMouseXYRelative().getX() < 0))
    {
        myPlaylists[0]->fileDragPaintRectangle = false;
        myPlaylists[0]->fileDragPaintLine = false;
        destinationPlayerFind = false;
        playerMouseDragUp = -1;
        playlistDragDestination = -1;
        myPlaylists[0]->repaint();
    }
}