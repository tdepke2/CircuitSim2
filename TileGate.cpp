#include "Board.h"
#include "TileGate.h"

TileGate::TileGate(const Vector2u& position, Direction direction, Board& board, Type type, bool active) {
    _position = position;
    _direction = direction;
    _type = type;
    _active = active;
    board.redrawTile(this);
}

int TileGate::getTextureID() const {
    bool connectRight = false, connectLeft = false;
    if (_type < 3) {
        return 21 + _type * 6 + connectRight * 2 + connectLeft * 4 + _active;
    } else {
        return 15 + _type * 8 + connectRight * 2 + connectLeft * 4 + _active;
    }
}