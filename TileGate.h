#ifndef _TILEGATE_H
#define _TILEGATE_H

class Board;

#include "Tile.h"
#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;

enum LogicGate : int {
    DIODE = 0, NOT, AND, OR, XOR, NAND, NOR, XNOR    // Should change this later to d, n, a, b, o, p, x, y when textures changed ################################################################################
};

class TileGate : public Tile {
    public:
    TileGate(const Vector2u& position, Board& board, LogicGate type, Direction direction, bool active);
    int getTextureID() const;
    
    private:
    LogicGate _type;
    Direction _direction;
    bool _active;
};

#endif