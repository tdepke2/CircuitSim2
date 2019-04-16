#include "Board.h"
#include "TileLED.h"

TileLED::TileLED(Board* boardPtr, const Vector2u& position, bool active) : Tile(boardPtr, position, true) {
    _active = active;
    addUpdate();
}

TileLED::~TileLED() {
    _boardPtr->LEDUpdates.erase(this);
}

int TileLED::getTextureID() const {
    return 17 + _active;
}

void TileLED::addUpdate(bool isCosmetic) {
    if (isCosmetic) {
        _boardPtr->cosmeticUpdates.insert(this);
    } else {
        _boardPtr->LEDUpdates.insert(this);
    }
}

Tile* TileLED::clone(Board* boardPtr, const Vector2u& position) {
    return new TileLED(boardPtr, position, _active);
}