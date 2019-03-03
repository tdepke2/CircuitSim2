#include "Board.h"
#include "TileLED.h"

TileLED::TileLED(const Vector2u& position, Board& board) {
    _position = position;
    board.redrawTile(this);
}

int TileLED::getTextureID() const {
    return 31;
}