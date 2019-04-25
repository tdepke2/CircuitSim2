#include "Board.h"
#include "TileSwitch.h"

TileSwitch::TileSwitch(Board* boardPtr, const Vector2u& position, char charID, State state) : Tile(boardPtr, position, true) {
    _charID = charID;
    _state = state;
    addUpdate();
}

TileSwitch::~TileSwitch() {
    _boardPtr->switchUpdates.erase(this);
}

int TileSwitch::getTextureID() const {
    return 13 + (_state == HIGH);
}

State TileSwitch::checkOutput(Direction direction) const {
    return _state;
}

void TileSwitch::addUpdate(bool isCosmetic) {
    _boardPtr->cosmeticUpdates.insert(this);
    if (!isCosmetic) {
        _boardPtr->switchUpdates.insert(this);
    }
}

Tile* TileSwitch::clone(Board* boardPtr, const Vector2u& position) {
    return new TileSwitch(boardPtr, position, _charID, _state);
}