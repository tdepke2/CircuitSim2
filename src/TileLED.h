#ifndef _TILELED_H
#define _TILELED_H

class Board;

#include "Tile.h"
#include <SFML/Graphics.hpp>
#include <stack>
#include <vector>

using namespace std;
using namespace sf;

class TileLED : public Tile {
    public:
    static vector<TileLED*> traversedLEDs;
    static stack<TileLED*> LEDNodes;
    
    TileLED(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates = false, State state = LOW);
    ~TileLED();
    int getTextureID() const;
    State getState() const;
    void setState(State state);
    void addUpdate(bool isCosmetic = false, bool noAdjacentUpdates = false);
    void followWire(Direction direction, State state);
    Tile* clone(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates = false);
    
    private:
    State _state;
    unsigned int _updateTimestamp;
    
    void _addNextTile(Tile* nextTile, Direction direction, State* statePtr);
};

#endif