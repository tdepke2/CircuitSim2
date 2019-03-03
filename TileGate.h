#ifndef _TILEGATE_H
#define _TILEGATE_H

class Board;

#include "Tile.h"
#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;

class TileGate : public Tile {
    public:
    TileGate(const Vector2u& position, Board& board);
    int getTextureID() const;
};

#endif