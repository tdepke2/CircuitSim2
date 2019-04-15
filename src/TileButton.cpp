#include "Board.h"
#include "TileButton.h"

TileButton::TileButton(Board* boardPtr, const Vector2u& position, char charID, bool active) : Tile(boardPtr, position, true) {
    _charID = charID;
    _active = active;
    _boardPtr->addUpdate(this, true);
}

int TileButton::getTextureID() const {
    return 15 + _active;
}

Tile* TileButton::clone(Board* boardPtr, const Vector2u& position) {
    return new TileButton(boardPtr, position, _charID, _active);
}