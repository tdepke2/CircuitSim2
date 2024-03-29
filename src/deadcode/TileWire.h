#ifndef _TILEWIRE_H
#define _TILEWIRE_H

class Board;

#include "Tile.h"
#include <SFML/Graphics.hpp>
#include <stack>
#include <string>
#include <utility>
#include <vector>

using namespace std;
using namespace sf;

class TileWire : public Tile {
    public:
    enum Type : int {
        STRAIGHT = 0, CORNER, TEE, JUNCTION, CROSSOVER
    };
    
    static vector<pair<TileWire*, Direction>> traversedWires;    // Vector of currently traveled wires in one instance of followWire, used to fix wire states in the case of a state conflict.
    static stack<pair<TileWire*, Direction>> wireNodes;    // Stack of wire nodes used in DFS algorithm.
    
    TileWire(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates = false, Direction direction = NORTH, Type type = STRAIGHT, State state1 = LOW, State state2 = LOW);    // Construct a wire tile, noAdjacentUpdates stops updates to adjacent tiles.
    ~TileWire();
    Type getType() const;
    State getState() const;
    void setDirection(Direction direction, bool noAdjacentUpdates = false);
    void setState(State state);
    void flip(bool acrossHorizontal, bool noAdjacentUpdates = false);    // Flips the tile across the vertical/horizontal axis.
    State checkOutput(Direction direction) const;    // Check for output from this tile on the side that is closest when traveling the given direction towards the tile.
    void addUpdate(bool isCosmetic = false, bool noAdjacentUpdates = false);    // Add an update to this tile into the corresponding hash set, isCosmetic disables the state update part, noAdjacentUpdates stops updates to adjacent tiles.
    void updateWire(State state);    // Attempts to update this wire and all connected wires with a given predicted state, travels all paths through the wire.
    void followWire(Direction direction, State state);    // Used in wire path following algorithm, traverses a wire using DFS and marks locations of endpoints to be updated later.
    void redrawTile() const;
    string toString() const;
    void fixUpdateTime();
    bool alternativeTile();
    Tile* clone(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates = false);    // Make a copy of this tile, the new tile needs its own board and position.
    
    private:
    enum FollowWireStage : int {
        INITIAL_STAGE = 0, INPUT_FOUND, INVALID_STAGE
    };
    
    const bool CONNECTION_INFO[4][5][4][4] = {    // Checks for wire path given direction of object, type, direction of entry, and direction of exit.
        {{{1, 0, 1, 0}, {0, 0, 0, 0}, {1, 0, 1, 0}, {0, 0, 0, 0}},    // STRAIGHT   "│ "
         {{0, 0, 0, 0}, {0, 0, 0, 0}, {1, 1, 0, 0}, {1, 1, 0, 0}},    // CORNER     "└─"
         {{1, 1, 1, 0}, {0, 0, 0, 0}, {1, 1, 1, 0}, {1, 1, 1, 0}},    // TEE        "├─"
         {{1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}},    // JUNCTION   "┼─"
         {{1, 0, 1, 0}, {0, 1, 0, 1}, {1, 0, 1, 0}, {0, 1, 0, 1}}},   // CROSSOVER  "│─"
        
        {{{0, 0, 0, 0}, {0, 1, 0, 1}, {0, 0, 0, 0}, {0, 1, 0, 1}},    // STRAIGHT   "──"
         {{0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 1, 1, 0}},    // CORNER     "┌─"
         {{0, 1, 1, 1}, {0, 1, 1, 1}, {0, 0, 0, 0}, {0, 1, 1, 1}},    // TEE        "┬─"
         {{1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}},    // JUNCTION   "┼─"
         {{1, 0, 1, 0}, {0, 1, 0, 1}, {1, 0, 1, 0}, {0, 1, 0, 1}}},   // CROSSOVER  "│─"
        
        {{{1, 0, 1, 0}, {0, 0, 0, 0}, {1, 0, 1, 0}, {0, 0, 0, 0}},    // STRAIGHT   "│ "
         {{0, 0, 1, 1}, {0, 0, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}},    // CORNER     "┐ "
         {{1, 0, 1, 1}, {1, 0, 1, 1}, {1, 0, 1, 1}, {0, 0, 0, 0}},    // TEE        "┤ "
         {{1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}},    // JUNCTION   "┼─"
         {{1, 0, 1, 0}, {0, 1, 0, 1}, {1, 0, 1, 0}, {0, 1, 0, 1}}},   // CROSSOVER  "│─"
        
        {{{0, 0, 0, 0}, {0, 1, 0, 1}, {0, 0, 0, 0}, {0, 1, 0, 1}},    // STRAIGHT   "──"
         {{0, 0, 0, 0}, {1, 0, 0, 1}, {1, 0, 0, 1}, {0, 0, 0, 0}},    // CORNER     "┘ "
         {{0, 0, 0, 0}, {1, 1, 0, 1}, {1, 1, 0, 1}, {1, 1, 0, 1}},    // TEE        "┴─"
         {{1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}},    // JUNCTION   "┼─"
         {{1, 0, 1, 0}, {0, 1, 0, 1}, {1, 0, 1, 0}, {0, 1, 0, 1}}},   // CROSSOVER  "│─"
    };
    
    Type _type;
    State _state1, _state2;
    unsigned int _updateTimestamp1, _updateTimestamp2;
    
    void _addNextTile(Tile* nextTile, Direction direction, State& state, FollowWireStage& stage) const;    // Adds a tile to current wire traversal in followWire.
    void _fixTraversedWires(State state) const;    // Sets the states of previously traversed wires to the new state.
    void _checkForInvalidState(State targetState, State& state, FollowWireStage& stage) const;    // Checks for conflicting state errors during _addNextTile.
};

#endif