#include "Board.h"
#include "TileSwitch.h"

TileSwitch::TileSwitch(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates, char charID, State state) : Tile(boardPtr, position, true, true) {
    _charID = charID;
    _state = state;
    addUpdate(false, noAdjacentUpdates);
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

void TileSwitch::addUpdate(bool isCosmetic, bool noAdjacentUpdates) {
    _boardPtr->cosmeticUpdates.insert(this);
    if (!isCosmetic) {
        _boardPtr->switchUpdates.insert(this);
        if (!noAdjacentUpdates) {
            _updateAdjacentTiles();
        }
    }
}

Tile* TileSwitch::clone(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates) {
    return new TileSwitch(boardPtr, position, noAdjacentUpdates, _charID, _state);
}