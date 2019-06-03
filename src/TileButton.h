#ifndef _TILEBUTTON_H
#define _TILEBUTTON_H

class Board;

#include "Tile.h"
#include <SFML/Graphics.hpp>
#include <string>

using namespace std;
using namespace sf;

class TileButton : public Tile {
    public:
    static void updateTransitioningButtons();    // Updates the states of all buttons in _transitioningButtons.
    TileButton(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates = false, char charID = ' ', State state = LOW);    // Construct a button tile, noAdjacentUpdates stops updates to adjacent tiles.
    ~TileButton();
    State getState() const;
    void setPosition(const Vector2u& position, bool noAdjacentUpdates = false, bool keepOverwrittenTile = false);    // Moves the tile to a new location, if keepOverwrittenTile is true then the tile that occupies the specified position will not be deleted.
    void setCharID(char charID);
    void setState(State state);
    State checkOutput(Direction direction) const;    // Check for output from this tile on the side that is closest when travelling the given direction towards the tile.
    void addUpdate(bool isCosmetic = false, bool noAdjacentUpdates = false);    // Add an update to this tile into the corresponding hash set, isCosmetic disables the state update part, noAdjacentUpdates stops updates to adjacent tiles.
    void updateOutput();    // Updates this button and starts wire traversal on connected tiles (does not change state).
    void redrawTile() const;
    string toString() const;
    bool alternativeTile();
    Tile* clone(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates = false);    // Make a copy of this tile, the new tile needs its own board and position.
    
    private:
    static vector<TileButton*> _transitioningButtons;    // Vector of buttons that are transitioning from HIGH to LOW and need a state update later.
    char _charID;
    State _state;
};

#endif