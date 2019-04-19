#ifndef _TILELED_H
#define _TILELED_H

class Board;

#include "Tile.h"
#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;

class TileLED : public Tile {
    public:
    TileLED(Board* boardPtr, const Vector2u& position, State state = LOW);
    ~TileLED();
    int getTextureID() const;
    State checkOutput(Direction direction) const;
    void addUpdate(bool isCosmetic = false);
    Tile* clone(Board* boardPtr, const Vector2u& position);
    
    private:
    State _state;
};

#endif