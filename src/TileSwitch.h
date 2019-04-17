#ifndef _TILESWITCH_H
#define _TILESWITCH_H

class Board;

#include "Tile.h"
#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;

class TileSwitch : public Tile {
    public:
    TileSwitch(Board* boardPtr, const Vector2u& position, char charID = '\0', State state = LOW);
    ~TileSwitch();
    int getTextureID() const;
    State checkConnection(Direction direction) const;
    void addUpdate(bool isCosmetic = false);
    Tile* clone(Board* boardPtr, const Vector2u& position);
    
    private:
    char _charID;
    State _state;
};

#endif