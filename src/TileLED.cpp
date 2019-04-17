#include "Board.h"
#include "TileLED.h"

TileLED::TileLED(Board* boardPtr, const Vector2u& position, State state) : Tile(boardPtr, position, true) {
    _state = state;
    addUpdate();
}

TileLED::~TileLED() {
    _boardPtr->LEDUpdates.erase(this);
}

int TileLED::getTextureID() const {
    return 17 + (_state == HIGH);
}

State TileLED::checkConnection(Direction direction) const {
    return _state;
}

void TileLED::addUpdate(bool isCosmetic) {
    if (isCosmetic) {
        _boardPtr->cosmeticUpdates.insert(this);
    } else {
        _boardPtr->LEDUpdates.insert(this);
    }
}

Tile* TileLED::clone(Board* boardPtr, const Vector2u& position) {
    return new TileLED(boardPtr, position, _state);
}