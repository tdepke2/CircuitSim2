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
    
    TileGate(Board* boardPtr, const Vector2u& position, Direction direction = NORTH, Type type = DIODE, State state = LOW);
    ~TileGate();
    int getTextureID() const;
    bool getState() const;
    bool getNextState() const;
    void setDirection(Direction direction);
    void flip(bool acrossHorizontal);
    bool checkNextState();
    State checkOutput(Direction direction) const;
    void addUpdate(bool isCosmetic = false);
    Tile* clone(Board* boardPtr, const Vector2u& position);
    
    private:
    Type _type;
    State _state, _nextState;
};

#endif