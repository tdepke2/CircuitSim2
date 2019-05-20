#ifndef _TILELED_H
#define _TILELED_H

class Board;

#include "Tile.h"
#include <SFML/Graphics.hpp>
#include <stack>
#include <vector>

using namespace std;
using namespace sf;

class TileLED : public Tile {
    public:
    static vector<TileLED*> traversedLEDs;    // Vector of currently travelled LEDs in one instance of updateLED, used to fix LED states in the case of a state conflict.
    static stack<TileLED*> LEDNodes;    // Stack of LED nodes used in DFS algorithm.
    
    TileLED(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates = false, State state = LOW);    // Construct an LED tile, noAdjacentUpdates stops updates to adjacent tiles.
    ~TileLED();
    State getState() const;
    void setState(State state);
    void addUpdate(bool isCosmetic = false, bool noAdjacentUpdates = false);    // Add an update to this tile into the corresponding hash set, isCosmetic disables the state update part, noAdjacentUpdates stops updates to adjacent tiles.
    void updateLED(State state);    // Starts LED path following through all connected LEDs given a predicted state, uses DFS traversal algorithm.
    void followWire(Direction direction, State state);    // Used in wire path following algorithm, just adds this LED to the list of endpoints.
    void redrawTile();
    Tile* clone(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates = false);    // Make a copy of this tile, the new tile needs its own board and position.
    
    private:
    State _state;
    unsigned int _updateTimestamp;
    
    void _addNextTile(Tile* nextTile, Direction direction, State* statePtr);    // Adds a tile to current LED traversal in updateLED.
};

#endif