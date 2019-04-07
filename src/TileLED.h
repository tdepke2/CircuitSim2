#ifndef _TILELED_H
#define _TILELED_H

class Board;

#include "Tile.h"
#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;

class TileLED : public Tile {
    public:
    TileLED(Board* boardPtr, const Vector2u& position, bool active = false);
    int getTextureID() const;
    Tile* clone(Board* boardPtr, const Vector2u& position);
    
    private:
    bool _active;
};

#endif