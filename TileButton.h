#ifndef _TILEBUTTON_H
#define _TILEBUTTON_H

class Board;

#include "Tile.h"
#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;

class TileButton : public Tile {
    public:
    TileButton(const Vector2u& position, Board& board, char charID = '\0', bool active = false);
    int getTextureID() const;
    Tile* clone(const Vector2u& position, Board& board);
    
    private:
    char _charID;
    bool _active;
};

#endif