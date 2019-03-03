#include "Board.h"
#include "TileGate.h"

TileGate::TileGate(const Vector2u& position, Board& board) {
    _position = position;
    board.redrawTile(this);
}

int TileGate::getTextureID() const {
    return 33;
}