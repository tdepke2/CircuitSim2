#include "Board.h"
#include "Tile.h"

unsigned int Tile::currentUpdateTime = 1;

Tile::Tile() {}

Tile::Tile(Board* boardPtr, const Vector2u& position, bool suppressUpdate) {
    _boardPtr = boardPtr;
    _position = position;
    _direction = NORTH;
    _highlight = false;
    if (!suppressUpdate) {
        addUpdate();
    }
}

Tile::~Tile() {
    _boardPtr->cosmeticUpdates.erase(this);
}

int Tile::getTextureID() const {
    return 0;
}

const Vector2u& Tile::getPosition() const {
    return _position;
}

Direction Tile::getDirection() const {
    return _direction;
}

bool Tile::getHighlight() const {
    return _highlight;
}

void Tile::setPosition(const Vector2u& position, bool keepOverwrittenTile) {
    if (!keepOverwrittenTile) {
        if (_boardPtr->getTile(position) != this) {
            delete _boardPtr->getTile(position);
            _boardPtr->setTile(position, this);
            _boardPtr->setTile(_position, new Tile(_boardPtr, _position));
            _position = position;
            addUpdate();
        }
    } else {
        _boardPtr->setTile(position, this);
        _position = position;
        addUpdate();
    }
}

void Tile::setHighlight(bool highlight) {
    if (_highlight != highlight) {
        _highlight = highlight;
        addUpdate(true);
    }
}

void Tile::setDirection(Direction direction) {}

void Tile::flip(bool acrossHorizontal) {}

State Tile::checkOutput(Direction direction) const {
    return DISCONNECTED;
}

pair<State, Tile*> Tile::checkState(Direction direction) const {
    if (direction == NORTH) {
        if (_position.y == 0) {
            return pair<State, Tile*>(DISCONNECTED, nullptr);
        } else {
            Tile* targetTile = _boardPtr->getTile(_position);
            return pair<State, Tile*>(targetTile->checkOutput(direction), targetTile);
        }
    } else {
        return pair<State, Tile*>(DISCONNECTED, nullptr);
    }
}

void Tile::addUpdate(bool isCosmetic) {
    _boardPtr->cosmeticUpdates.insert(this);
}

void Tile::followWire(Direction direction, State state) {}

Tile* Tile::clone(Board* boardPtr, const Vector2u& position) {
    return new Tile(boardPtr, position);
}