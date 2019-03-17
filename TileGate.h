#ifndef _TILEGATE_H
#define _TILEGATE_H

class Board;

#include "Tile.h"
#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;



class TileGate : public Tile {
    public:
    enum Type : int {
        DIODE = 0, BUFFER, NOT, AND, NAND, OR, NOR, XOR, XNOR
    };
    
    TileGate(const Vector2u& position, Board& board, Direction direction = NORTH, Type type = DIODE, bool active = false);
    int getTextureID() const;
    
    private:
    Type _type;
    bool _active;
};

#endif