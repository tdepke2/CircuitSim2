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

void TileGate::flip(bool acrossHorizontal, Board& board) {
    if ((!acrossHorizontal && _direction % 2 == 1) || (acrossHorizontal && _direction % 2 == 0)) {
        _direction = static_cast<Direction>((_direction + 2) % 4);
        board.redrawTile(this);
    }
}

Tile* TileGate::clone(const Vector2u& position, Board& board) {
    return new TileGate(position, board, _direction, _type, _active);
}