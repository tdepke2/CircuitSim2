#include "Board.h"
#include "TileButton.h"

TileButton::TileButton(const Vector2u& position, Board& board, char charID, bool active) {
    _position = position;
    _direction = NORTH;
    _charID = charID;
    _active = active;
    board.redrawTile(this);
}

int TileButton::getTextureID() const {
    return 15 + _active;
}