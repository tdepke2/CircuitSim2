#include "Board.h"
#include "Tile.h"

unsigned int Tile::currentUpdateTime = 1;

Tile::Tile() {}

Tile::Tile(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates, bool suppressUpdate) {
    _boardPtr = boardPtr;
    _position = position;
    _direction = NORTH;
    _highlight = false;
    if (!suppressUpdate) {
        addUpdate(false, noAdjacentUpdates);
    }
}

Tile::~Tile() {
    _boardPtr->cosmeticUpdates.erase(this);
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

State Tile::getState() const {
    return DISCONNECTED;
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

void Tile::setDirection(Direction direction) {}

void Tile::setHighlight(bool highlight) {
    if (_highlight != highlight) {
        _highlight = highlight;
        addUpdate(true);
    }
}

void Tile::setState(State state) {}

void Tile::flip(bool acrossHorizontal) {}

State Tile::checkOutput(Direction direction) const {
    return DISCONNECTED;
}

void Tile::addUpdate(bool isCosmetic, bool noAdjacentUpdates) {
    _boardPtr->cosmeticUpdates.insert(this);
    if (!isCosmetic && !noAdjacentUpdates) {
        _updateAdjacentTiles();
    }
}

void Tile::followWire(Direction direction, State state) {}

void Tile::redrawTile() {
    _boardPtr->redrawTileVertices(0, _position, _direction, _highlight);
}

Tile* Tile::clone(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates) {
    return new Tile(boardPtr, position, noAdjacentUpdates);
}

void Tile::_updateAdjacentTiles() {
    if (_position.y > 0) {
        _boardPtr->getTile(Vector2u(_position.x, _position.y - 1))->addUpdate(false, true);
    }
    if (_position.x < _boardPtr->getSize().x - 1) {
        _boardPtr->getTile(Vector2u(_position.x + 1, _position.y))->addUpdate(false, true);
    }
    if (_position.y < _boardPtr->getSize().y - 1) {
        _boardPtr->getTile(Vector2u(_position.x, _position.y + 1))->addUpdate(false, true);
    }
    if (_position.x > 0) {
        _boardPtr->getTile(Vector2u(_position.x - 1, _position.y))->addUpdate(false, true);
    }
}