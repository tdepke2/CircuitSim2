#ifndef _TILELED_H
#define _TILELED_H

class Board;

#include "Tile.h"
#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;

class TileLED : public Tile {
    public:
    TileLED(const Vector2u& position, Board& board);
    int getTextureID() const;
};

#endif