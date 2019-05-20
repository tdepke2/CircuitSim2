#ifndef _TILEBUTTON_H
#define _TILEBUTTON_H

class Board;

#include "Tile.h"
#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;

class TileButton : public Tile {
    public:
    static void updateTransitioningButtons();
    TileButton(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates = false, char charID = '\0', State state = LOW);
    ~TileButton();
    int getTextureID() const;
    State getState() const;
    void setCharID(char charID);
    void setState(State state);
    State checkOutput(Direction direction) const;
    void addUpdate(bool isCosmetic = false, bool noAdjacentUpdates = false);
    void updateOutput();
    Tile* clone(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates = false);
    
    private:
    static vector<TileButton*> _transitioningButtons;
    char _charID;
    State _state;
};

#endif