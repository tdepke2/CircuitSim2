#include "Board.h"
#include "TileGate.h"

TileGate::TileGate(Board* boardPtr, const Vector2u& position, Direction direction, Type type, bool active) : Tile(boardPtr, position, true) {
    _direction = direction;
    _type = type;
    _active = active;
    addUpdate();
}

TileGate::~TileGate() {
    _boardPtr->gateUpdates.erase(this);
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
    addUpdate();
}

void TileGate::flip(bool acrossHorizontal, Board& board) {
    if ((!acrossHorizontal && _direction % 2 == 1) || (acrossHorizontal && _direction % 2 == 0)) {
        _direction = static_cast<Direction>((_direction + 2) % 4);
        addUpdate();
    }
}

void TileGate::addUpdate(bool isCosmetic) {
    if (isCosmetic) {
        _boardPtr->cosmeticUpdates.insert(this);
    } else {
        _boardPtr->gateUpdates.insert(this);
    }
}

Tile* TileGate::clone(Board* boardPtr, const Vector2u& position) {
    return new TileGate(boardPtr, position, _direction, _type, _active);
}