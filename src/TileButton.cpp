#include "Board.h"
#include "TileButton.h"

TileButton::TileButton(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates, char charID, State state) : Tile(boardPtr, position, true, true) {
    _charID = charID;
    _state = state;
    addUpdate(false, noAdjacentUpdates);
}

TileButton::~TileButton() {
    _boardPtr->buttonUpdates.erase(this);
}

int TileButton::getTextureID() const {
    return 15 + (_state == HIGH);
}

State TileButton::checkOutput(Direction direction) const {
    return _state;
}

void TileButton::addUpdate(bool isCosmetic, bool noAdjacentUpdates) {
    _boardPtr->cosmeticUpdates.insert(this);
    if (!isCosmetic) {
        _boardPtr->buttonUpdates.insert(this);
        if (!noAdjacentUpdates) {
            _updateAdjacentTiles();
        }
    }
}

Tile* TileButton::clone(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates) {
    return new TileButton(boardPtr, position, noAdjacentUpdates, _charID, _state);
}