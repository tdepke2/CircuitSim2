#include "Board.h"
#include "TileGate.h"

TileGate::TileGate(const Vector2u& position, Board& board, Direction direction, Type type, bool active) {
    _position = position;
    _direction = direction;
    _type = type;
    _active = active;
    board.redrawTile(this);
}

int TileGate::getTextureID() const {
    bool connectRight = false, connectLeft = false;
    if (_type < 3) {
        return 19 + _type * 6 + connectRight * 2 + connectLeft * 4 + _active;
    } else {
        return 13 + _type * 8 + connectRight * 2 + connectLeft * 4 + _active;
    }
}

void TileGate::setDirection(Direction direction, Board& board) {
    _direction = direction;
    board.redrawTile(this);
}

Tile* TileGate::clone(const Vector2u& position, Board& board) {
    return new TileGate(position, board, _direction, _type, _active);
}