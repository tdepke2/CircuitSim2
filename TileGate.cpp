#include "Board.h"
#include "TileGate.h"

TileGate::TileGate(const Vector2u& position, Board& board, LogicGate type, Direction direction, bool active) {
    _position = position;
    _type = type;
    _direction = direction;
    _active = active;
    board.redrawTile(this);
}

int TileGate::getTextureID() const {
    return 33 + _type * 8 + _direction * 2 + _active;
}