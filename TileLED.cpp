#include "Board.h"
#include "TileLED.h"

TileLED::TileLED(const Vector2u& position, Board& board, bool active) {
    _position = position;
    _direction = NORTH;
    _active = active;
    board.redrawTile(this);
}

int TileLED::getTextureID() const {
    return 19 + _active;
}