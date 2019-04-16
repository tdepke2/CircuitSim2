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
    
    TileGate(Board* boardPtr, const Vector2u& position, Direction direction = NORTH, Type type = DIODE, bool active = false);
    ~TileGate();
    int getTextureID() const;
    void setDirection(Direction direction, Board& board);
    void flip(bool acrossHorizontal, Board& board);
    void addUpdate(bool isCosmetic = false);
    Tile* clone(Board* boardPtr, const Vector2u& position);
    
    private:
    Type _type;
    bool _active;
};

#endif