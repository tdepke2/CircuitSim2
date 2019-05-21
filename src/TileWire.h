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
    
    static vector<pair<TileWire*, Direction>> traversedWires;    // Vector of currently travelled wires in one instance of followWire, used to fix wire states in the case of a state conflict.
    static stack<pair<TileWire*, Direction>> wireNodes;    // Stack of wire nodes used in DFS algorithm.
    
    TileWire(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates = false, Direction direction = NORTH, Type type = STRAIGHT, State state1 = LOW, State state2 = LOW);    // Construct a wire tile, noAdjacentUpdates stops updates to adjacent tiles.
    ~TileWire();
    State getState() const;
    void setDirection(Direction direction);
    void setState(State state);
    void flip(bool acrossHorizontal);    // Flips the tile across the vertical/horizontal axis.
    State checkOutput(Direction direction) const;    // Check for output from this tile on the side that is closest when travelling the given direction towards the tile.
    void addUpdate(bool isCosmetic = false, bool noAdjacentUpdates = false);    // Add an update to this tile into the corresponding hash set, isCosmetic disables the state update part, noAdjacentUpdates stops updates to adjacent tiles.
    void updateWire(State state);    // Attempts to update this wire and all connected wires with a given predicted state, travels all paths through the wire.
    void followWire(Direction direction, State state);    // Used in wire path following algorithm, traverses a wire using DFS and marks locations of endpoints to be updated later.
    void redrawTile() const;
    string toString() const;
    Tile* clone(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates = false);    // Make a copy of this tile, the new tile needs its own board and position.
    
    private:
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
    
    void _addNextTile(Tile* nextTile, Direction direction, State* statePtr);    // Adds a tile to current wire traversal in followWire.
    void _fixTraversedWires(State state);    // Sets the states of previously traversed wires to the new state.
};

#endif

/* List of type IDs
 0  "| "  "│ "
 1  "--"  "──"
 2  "'-"  "└─"
 3  ",-"  "┌─"
 4  ", "  "┐ "
 5  "' "  "┘ "
 6  ">-"  "├─"
 7  "v-"  "┬─"
 8  "< "  "┤ "
 9  "^-"  "┴─"
10  "+-"  "┼─"
11  "|-"  "│─"

 0  "s"
 1  "t"

 0  ".."  "░░"

 0  "d"
 1  "n"
 2  "a"
 3  "o"
 4  "x"
 5  "b"
 6  "p"
 7  "y"
*/