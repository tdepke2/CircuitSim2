#ifndef _TILESWITCH_H
#define _TILESWITCH_H

class Board;

#include "Tile.h"
#include <SFML/Graphics.hpp>
#include <string>

using namespace std;
using namespace sf;

class TileSwitch : public Tile {
    public:
    TileSwitch(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates = false, char charID = ' ', State state = LOW);    // Construct a switch tile, noAdjacentUpdates stops updates to adjacent tiles.
    ~TileSwitch();
    State getState() const;
    void setCharID(char charID);
    void setState(State state);
    State checkOutput(Direction direction) const;    // Check for output from this tile on the side that is closest when travelling the given direction towards the tile.
    void addUpdate(bool isCosmetic = false, bool noAdjacentUpdates = false);    // Add an update to this tile into the corresponding hash set, isCosmetic disables the state update part, noAdjacentUpdates stops updates to adjacent tiles.
    void updateOutput();    // Updates this switch and starts wire traversal on connected tiles.
    void redrawTile() const;
    string toString() const;
    bool alternativeTile();
    Tile* clone(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates = false);    // Make a copy of this tile, the new tile needs its own board and position.
    
    private:
    char _charID;
    State _state;
};

#endif