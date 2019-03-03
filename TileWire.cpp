#include "Board.h"
#include "TileWire.h"

TileWire::TileWire(const Vector2u& position, Board& board) {
    _position = position;
    _typeID = 0;
    _active1 = false;
    _active2 = false;
    board.redrawTile(this);
}

int TileWire::getTextureID() const {
    if (_typeID <= 10) {
        return _typeID * 2 + _active1 + 1;
    } else {
        return 23 + _active1 + _active2 * 2;
    }
}

void TileWire::setActive(Direction d, bool state) {
    
}

bool TileWire::isActive(Direction d) const {
    return false;
}

bool TileWire::connectsNorth(Direction d) const {
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
}