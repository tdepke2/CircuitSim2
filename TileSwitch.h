#ifndef _TILESWITCH_H
#define _TILESWITCH_H

class Board;

#include "Tile.h"
#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;

class TileSwitch : public Tile {
    public:
    TileSwitch(const Vector2u& position, Board& board, char charID = '\0', bool active = false);
    int getTextureID() const;
    
    private:
    char _charID;
    bool _active;
};

#endif