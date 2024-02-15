#ifndef _TILEGATE_H
#define _TILEGATE_H

class Board;

#include "Tile.h"
#include <SFML/Graphics.hpp>
#include <string>

using namespace std;
using namespace sf;

class TileGate : public Tile {
    public:
    enum Type : int {
        DIODE = 0, BUFFER, NOT, AND, NAND, OR, NOR, XOR, XNOR
    };
    
    TileGate(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates = false, Direction direction = NORTH, Type type = DIODE, State state = LOW);    // Construct a gate tile, noAdjacentUpdates stops updates to adjacent tiles.
    ~TileGate();
    Type getType() const;
    State getState() const;
    State getNextState() const;
    void setDirection(Direction direction, bool noAdjacentUpdates = false);
    void setState(State state);
    void flip(bool acrossHorizontal, bool noAdjacentUpdates = false);    // Flips the tile across the vertical/horizontal axis.
    State checkOutput(Direction direction) const;    // Check for output from this tile on the side that is closest when traveling the given direction towards the tile.
    void addUpdate(bool isCosmetic = false, bool noAdjacentUpdates = false);    // Add an update to this tile into the corresponding hash set, isCosmetic disables the state update part, noAdjacentUpdates stops updates to adjacent tiles.
    bool updateNextState();    // Checks adjacent tile states (only inputs) and sets the next state of this gate, returns true if state changed. Does not set the actual state of this gate.
    void updateOutput();    // Updates this gate and starts wire traversal on the output tile.
    void followWire(Direction direction, State state);    // Used in wire path following algorithm, just adds this gate to the list of endpoints if the gate does not point back to the source in the direction.
    void redrawTile() const;
    string toString() const;
    bool alternativeTile();
    Tile* clone(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates = false);    // Make a copy of this tile, the new tile needs its own board and position.
    
    private:
    Type _type;
    State _state, _nextState;
    bool _connectorChange, _rightConnector, _leftConnector;
    
    State _complementState(State state) const;    // Converts LOW state to HIGH and HIGH to LOW.
    State _findNextStateBuffer(State adjacentStates[4], int numInputs, int numHigh, int numMiddle) const;    // Determines the next gate state (buffer logic).
    State _findNextStateAND(State adjacentStates[4], int numInputs, int numHigh, int numMiddle) const;    // Determines the next gate state (AND logic).
    State _findNextStateOR(State adjacentStates[4], int numInputs, int numHigh, int numMiddle) const;    // Determines the next gate state (OR logic).
    State _findNextStateXOR(State adjacentStates[4], int numInputs, int numHigh, int numMiddle) const;    // Determines the next gate state (XOR logic).
};

#endif