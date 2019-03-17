#include "Board.h"
#include "TileLED.h"

TileLED::TileLED(const Vector2u& position, Board& board, bool active) {
    _position = position;
    _direction = NORTH;
    _active = active;
    board.redrawTile(this);
}

int TileLED::getTextureID() const {
    return 17 + _active;
}

Tile* TileLED::clone(const Vector2u& position, Board& board) {
    return new TileLED(position, board, _active);
}