#include "Board.h"
#include "TileWire.h"

TileWire::TileWire(Board* boardPtr, const Vector2u& position, Direction direction, Type type, State state1, State state2) : Tile(boardPtr, position, true) {
    if (type != JUNCTION && type != CROSSOVER) {
        if (type == STRAIGHT) {
            _direction = static_cast<Direction>(direction % 2);
        } else {
            _direction = direction;
        }
    }
    _type = type;
    _state1 = state1;
    _state2 = state2;
    addUpdate();
}

TileWire::~TileWire() {
    _boardPtr->wireUpdates.erase(this);
}

int TileWire::getTextureID() const {
    return 1 + _type * 2 + _state1 - 1 + (_state2 - 1) * 2;
}

void TileWire::setDirection(Direction direction, Board& board) {
    if (_type != JUNCTION) {
        if (_type != CROSSOVER) {
            if (_type == STRAIGHT) {
                _direction = static_cast<Direction>(direction % 2);
            } else {
                _direction = direction;
            }
            addUpdate();
        } else if (direction % 2 == 1) {    // If direction odd, assume 1 or 3 rotations were made to the crossover wire.
            State tempState = _state1;
            _state1 = _state2;
            _state2 = tempState;
            addUpdate();
        }
    }
}

void TileWire::flip(bool acrossHorizontal, Board& board) {
    if (_type == CORNER) {
        if (!acrossHorizontal) {
            _direction = static_cast<Direction>(3 - _direction);
        } else if (_direction % 2 == 0) {
            _direction = static_cast<Direction>(_direction + 1);
        } else {
            _direction = static_cast<Direction>(_direction - 1);
        }
        addUpdate();
    } else if (_type == TEE && ((!acrossHorizontal && _direction % 2 == 0) || (acrossHorizontal && _direction % 2 == 1))) {
        _direction = static_cast<Direction>((_direction + 2) % 4);
        addUpdate();
    }
}

State TileWire::checkConnection(Direction direction) const {
    const bool* exitDirections = CONNECTION_INFO[_direction][_type][direction];
    if (exitDirections[0] || exitDirections[1] || exitDirections[2] || exitDirections[3]) {
        if (_type == CROSSOVER && direction % 2 == 1) {
            return _state2;
        } else {
            return _state1;
        }
    } else {
        return DISCONNECTED;
    }
}

void TileWire::addUpdate(bool isCosmetic) {
    if (isCosmetic) {
        _boardPtr->cosmeticUpdates.insert(this);
    } else {
        _boardPtr->wireUpdates.insert(this);
    }
}

Tile* TileWire::clone(Board* boardPtr, const Vector2u& position) {
    return new TileWire(boardPtr, position, _direction, _type, _state1, _state2);
}

bool TileWire::isActive(Direction d) const {
    return false;
}