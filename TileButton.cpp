#include "Board.h"
#include "TileButton.h"

TileButton::TileButton(const Vector2u& position, Board& board) {
    _position = position;
    board.redrawTile(this);
}

int TileButton::getTextureID() const {
    return 29;
}