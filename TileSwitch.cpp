#include "Board.h"
#include "TileSwitch.h"

TileSwitch::TileSwitch(const Vector2u& position, Board& board, char charID, bool active) {
    _position = position;
    _direction = NORTH;
    _charID = charID;
    _active = active;
    board.redrawTile(this);
}

int TileSwitch::getTextureID() const {
    return 15 + _active;
}