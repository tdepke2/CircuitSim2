#include "Board.h"
#include "TileButton.h"

TileButton::TileButton(Board* boardPtr, const Vector2u& position, char charID, State state) : Tile(boardPtr, position, true) {
    _charID = charID;
    _state = state;
    addUpdate();
}

TileButton::~TileButton() {
    _boardPtr->buttonUpdates.erase(this);
}

int TileButton::getTextureID() const {
    return 15 + (_state == HIGH);
}

State TileButton::checkConnection(Direction direction) const {
    return _state;
}

void TileButton::addUpdate(bool isCosmetic) {
    if (isCosmetic) {
        _boardPtr->cosmeticUpdates.insert(this);
    } else {
        _boardPtr->buttonUpdates.insert(this);
    }
}

Tile* TileButton::clone(Board* boardPtr, const Vector2u& position) {
    return new TileButton(boardPtr, position, _charID, _state);
}