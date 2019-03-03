#include "Board.h"
#include "TileLED.h"

TileLED::TileLED(const Vector2u& position, Board& board, bool active) {
    _position = position;
    _active = active;
    board.redrawTile(this);
}

int TileLED::getTextureID() const {
    return 31 + _active;
}