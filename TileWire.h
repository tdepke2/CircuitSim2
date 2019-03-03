#ifndef _TILEWIRE_H
#define _TILEWIRE_H

class Board;

#include "Tile.h"
#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;

class TileWire : public Tile {
    public:
    TileWire(const Vector2u& position, Board& board, int typeID, bool active1, bool active2);
    int getTextureID() const;
    void setActive(Direction d, bool state);
    bool isActive(Direction d) const;
    bool connectsNorth(Direction d) const;
    bool connectsEast(Direction d) const;
    bool connectsSouth(Direction d) const;
    bool connectsWest(Direction d) const;
    
    private:
    const bool CONNECTION_INFO_NORTH[12][4] = {
        {1, 0, 0, 0}, {0, 0, 0, 0},
        {0, 0, 0, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 1, 0, 0},
        {1, 0, 0, 1}, {0, 0, 0, 0}, {1, 1, 0, 0}, {0, 1, 0, 1},
        {1, 1, 0, 1},
        {1, 0, 0, 0}
    };
    const bool CONNECTION_INFO_EAST[12][4] = {
        {0, 0, 0, 0}, {0, 1, 0, 0},
        {0, 0, 1, 0}, {1, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0},
        {1, 0, 1, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 1, 1, 0},
        {1, 1, 1, 0},
        {0, 1, 0, 0}
    };
    const bool CONNECTION_INFO_SOUTH[12][4] = {
        {0, 0, 1, 0}, {0, 0, 0, 0},
        {0, 0, 0, 0}, {0, 0, 0, 1}, {0, 1, 0, 0}, {0, 0, 0, 0},
        {0, 0, 1, 1}, {0, 1, 0, 1}, {0, 1, 1, 0}, {0, 0, 0, 0},
        {0, 1, 1, 1},
        {0, 0, 1, 0}
    };
    const bool CONNECTION_INFO_WEST[12][4] = {
        {0, 0, 0, 0}, {0, 0, 0, 1},
        {0, 0, 0, 0}, {0, 0, 0, 0}, {1, 0, 0, 0}, {0, 0, 1, 0},
        {0, 0, 0, 0}, {1, 0, 0, 1}, {1, 0, 1, 0}, {0, 0, 1, 1},
        {1, 0, 1, 1},
        {0, 0, 0, 1}
    };
    int _typeID;
    bool _active1, _active2;
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