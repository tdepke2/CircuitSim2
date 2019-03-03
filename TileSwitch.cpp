#include "Board.h"
#include "TileSwitch.h"

TileSwitch::TileSwitch(const Vector2u& position, Board& board) {
    _position = position;
    board.redrawTile(this);
}

int TileSwitch::getTextureID() const {
    return 27;
}