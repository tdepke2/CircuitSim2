#ifndef _TILEWIRE_H
#define _TILEWIRE_H

class Board;

#include "Tile.h"
#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;

class TileWire : public Tile {
    public:
    enum Type : int {
        STRAIGHT = 0, CORNER, TEE, JUNCTION, CROSSOVER
    };
    
    TileWire(Board* boardPtr, const Vector2u& position, Direction direction = NORTH, Type type = STRAIGHT, State state1 = LOW, State state2 = LOW);
    ~TileWire();
    int getTextureID() const;
    void setDirection(Direction direction, Board& board);
    void flip(bool acrossHorizontal, Board& board);
    State checkOutput(Direction direction) const;
    void addUpdate(bool isCosmetic = false);
    Tile* clone(Board* boardPtr, const Vector2u& position);
    bool isActive(Direction d) const;
    
    private:
    const bool CONNECTION_INFO[4][5][4][4] = {    // Checks for wire path given direction of object, type, direction of entry, and direction of exit.
        {{{1, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 0}},    // STRAIGHT   "│ "
         {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 1, 0, 0}, {1, 0, 0, 0}},    // CORNER     "└─"
         {{1, 1, 0, 0}, {0, 0, 0, 0}, {0, 1, 1, 0}, {1, 0, 1, 0}},    // TEE        "├─"
         {{1, 1, 0, 1}, {1, 1, 1, 0}, {0, 1, 1, 1}, {1, 0, 1, 1}},    // JUNCTION   "┼─"
         {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}},   // CROSSOVER  "│─"
        
        {{{0, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 1}},    // STRAIGHT   "──"
         {{0, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 1, 0}},    // CORNER     "┌─"
         {{0, 1, 0, 1}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 1, 1}},    // TEE        "┬─"
         {{1, 1, 0, 1}, {1, 1, 1, 0}, {0, 1, 1, 1}, {1, 0, 1, 1}},    // JUNCTION   "┼─"
         {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}},   // CROSSOVER  "│─"
        
        {{{1, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 0}},    // STRAIGHT   "│ "
         {{0, 0, 0, 1}, {0, 0, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},    // CORNER     "┐ "
         {{1, 0, 0, 1}, {1, 0, 1, 0}, {0, 0, 1, 1}, {0, 0, 0, 0}},    // TEE        "┤ "
         {{1, 1, 0, 1}, {1, 1, 1, 0}, {0, 1, 1, 1}, {1, 0, 1, 1}},    // JUNCTION   "┼─"
         {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}},   // CROSSOVER  "│─"
        
        {{{0, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 1}},    // STRAIGHT   "──"
         {{0, 0, 0, 0}, {1, 0, 0, 0}, {0, 0, 0, 1}, {0, 0, 0, 0}},    // CORNER     "┘ "
         {{0, 0, 0, 0}, {1, 1, 0, 0}, {0, 1, 0, 1}, {1, 0, 0, 1}},    // TEE        "┴─"
         {{1, 1, 0, 1}, {1, 1, 1, 0}, {0, 1, 1, 1}, {1, 0, 1, 1}},    // JUNCTION   "┼─"
         {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}},   // CROSSOVER  "│─"
    };
    
    
    Type _type;
    State _state1, _state2;
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