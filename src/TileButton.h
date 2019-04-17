#ifndef _TILEBUTTON_H
#define _TILEBUTTON_H

class Board;

#include "Tile.h"
#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;

class TileButton : public Tile {
    public:
    TileButton(Board* boardPtr, const Vector2u& position, char charID = '\0', State state = LOW);
    ~TileButton();
    int getTextureID() const;
    State checkConnection(Direction direction) const;
    void addUpdate(bool isCosmetic = false);
    Tile* clone(Board* boardPtr, const Vector2u& position);
    
    private:
    char _charID;
    State _state;
};

#endif