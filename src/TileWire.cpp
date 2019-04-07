#include "Board.h"
#include "TileWire.h"

TileWire::TileWire(Board* boardPtr, const Vector2u& position, Direction direction, Type type, bool active1, bool active2) : Tile(boardPtr, position) {
    if (type != JUNCTION && type != CROSSOVER) {
        if (type == STRAIGHT) {
            _direction = static_cast<Direction>(direction % 2);
        } else {
            _direction = direction;
        }
    }
    _type = type;
    _active1 = active1;
    _active2 = active2;
}

int TileWire::getTextureID() const {
    return 1 + _type * 2 + _active1 + _active2 * 2;
}

void TileWire::setDirection(Direction direction, Board& board) {
    if (_type != JUNCTION) {
        if (_type != CROSSOVER) {
            if (_type == STRAIGHT) {
                _direction = static_cast<Direction>(direction % 2);
            } else {
                _direction = direction;
            }
            _boardPtr->addUpdate(this, true);
        } else if (direction % 2 == 1) {    // If direction odd, assume 1 or 3 rotations were made to the crossover wire.
            bool tempActive = _active1;
            _active1 = _active2;
            _active2 = tempActive;
            _boardPtr->addUpdate(this, true);
        }
    }
}

void TileWire::setActive(Direction d, bool state) {
    
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
        _boardPtr->addUpdate(this, true);
    } else if (_type == TEE && ((!acrossHorizontal && _direction % 2 == 0) || (acrossHorizontal && _direction % 2 == 1))) {
        _direction = static_cast<Direction>((_direction + 2) % 4);
        _boardPtr->addUpdate(this, true);
    }
}

Tile* TileWire::clone(Board* boardPtr, const Vector2u& position) {
    return new TileWire(boardPtr, position, _direction, _type, _active1, _active2);
}

bool TileWire::isActive(Direction d) const {
    return false;
}

/*bool TileWire::connectsNorth(Direction d) const {
    return CONNECTION_INFO_NORTH[_typeID][d];
}

bool TileWire::connectsEast(Direction d) const {
    return CONNECTION_INFO_EAST[_typeID][d];
}

bool TileWire::connectsSouth(Direction d) const {
    return CONNECTION_INFO_SOUTH[_typeID][d];
}

bool TileWire::connectsWest(Direction d) const {
    return CONNECTION_INFO_WEST[_typeID][d];
}*/