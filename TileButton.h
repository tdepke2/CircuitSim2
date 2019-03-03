#ifndef _TILEBUTTON_H
#define _TILEBUTTON_H

class Board;

#include "Tile.h"
#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;

class TileButton : public Tile {
    public:
    TileButton(const Vector2u& position, Board& board);
    int getTextureID() const;
};

#endif