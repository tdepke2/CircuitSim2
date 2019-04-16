#include "Board.h"
#include "TileSwitch.h"

TileSwitch::TileSwitch(Board* boardPtr, const Vector2u& position, char charID, bool active) : Tile(boardPtr, position, true) {
    _charID = charID;
    _active = active;
    addUpdate();
}

TileSwitch::~TileSwitch() {
    _boardPtr->switchUpdates.erase(this);
}

int TileSwitch::getTextureID() const {
    return 13 + _active;
}

void TileSwitch::addUpdate(bool isCosmetic) {
    if (isCosmetic) {
        _boardPtr->cosmeticUpdates.insert(this);
    } else {
        _boardPtr->switchUpdates.insert(this);
    }
}

Tile* TileSwitch::clone(Board* boardPtr, const Vector2u& position) {
    return new TileSwitch(boardPtr, position, _charID, _active);
}