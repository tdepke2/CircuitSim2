#include "Board.h"
#include "TileSwitch.h"

TileSwitch::TileSwitch(Board* boardPtr, const Vector2u& position, char charID, bool active) : Tile(boardPtr, position, true) {
    _charID = charID;
    _active = active;
    _boardPtr->addUpdate(this, true);
}

int TileSwitch::getTextureID() const {
    return 13 + _active;
}

Tile* TileSwitch::clone(Board* boardPtr, const Vector2u& position) {
    return new TileSwitch(boardPtr, position, _charID, _active);
}