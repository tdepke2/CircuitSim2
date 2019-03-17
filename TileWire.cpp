#include "Board.h"
#include "TileWire.h"

TileWire::TileWire(const Vector2u& position, Board& board, Direction direction, Type type, bool active1, bool active2) {
    _position = position;
    if (type == STRAIGHT) {
        _direction = static_cast<Direction>(direction % 2);
    } else {
        _direction = direction;
    }
    _type = type;
    _active1 = active1;
    _active2 = active2;
    board.redrawTile(this);
}

int TileWire::getTextureID() const {
    return 1 + _type * 2 + _active1 + _active2 * 2;
}

void TileWire::setActive(Direction d, bool state) {
    
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