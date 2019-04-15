#include "Board.h"
#include "TileLED.h"

TileLED::TileLED(Board* boardPtr, const Vector2u& position, bool active) : Tile(boardPtr, position, true) {
    _active = active;
    _boardPtr->addUpdate(this, true);
}

int TileLED::getTextureID() const {
    return 17 + _active;
}

Tile* TileLED::clone(Board* boardPtr, const Vector2u& position) {
    return new TileLED(boardPtr, position, _active);
}