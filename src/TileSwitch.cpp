#include "Board.h"
#include "TileSwitch.h"

TileSwitch::TileSwitch(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates, char charID, State state) : Tile(boardPtr, position, true, true) {
    _charID = charID;
    _state = state;
    boardPtr->switchKeybinds[charID].push_back(this);
    addUpdate(false, noAdjacentUpdates);
}

TileSwitch::~TileSwitch() {
    _boardPtr->switchUpdates.erase(this);
    auto mapIter = _boardPtr->switchKeybinds.find(_charID);
    for (auto vectorIter = mapIter->second.begin(); vectorIter != mapIter->second.end(); ++vectorIter) {
        if (*vectorIter == this) {
            mapIter->second.erase(vectorIter);
            break;
        }
    }
}

int TileSwitch::getTextureID() const {
    return 13 + (_state == HIGH);
}

State TileSwitch::getState() const {
    return _state;
}

void TileSwitch::setCharID(char charID) {
    auto mapIter = _boardPtr->switchKeybinds.find(_charID);
    for (auto vectorIter = mapIter->second.begin(); vectorIter != mapIter->second.end(); ++vectorIter) {
        if (*vectorIter == this) {
            mapIter->second.erase(vectorIter);
            break;
        }
    }
    _charID = charID;
    _boardPtr->switchKeybinds[_charID].push_back(this);
    addUpdate(true);
}

void TileSwitch::setState(State state) {
    _state = state;
    addUpdate();
}

State TileSwitch::checkOutput(Direction direction) const {
    return _state;
}

void TileSwitch::addUpdate(bool isCosmetic, bool noAdjacentUpdates) {
    _boardPtr->cosmeticUpdates.insert(this);
    if (!isCosmetic) {
        _boardPtr->switchUpdates.insert(this);
        if (!noAdjacentUpdates) {
            _updateAdjacentTiles();
        }
    }
}

void TileSwitch::updateOutput() {
    _boardPtr->switchUpdates.erase(this);
    
    if (_position.y > 0) {
        _boardPtr->getTile(Vector2u(_position.x, _position.y - 1))->followWire(NORTH, _state);
    }
    if (_position.x < _boardPtr->getSize().x - 1) {
        _boardPtr->getTile(Vector2u(_position.x + 1, _position.y))->followWire(EAST, _state);
    }
    if (_position.y < _boardPtr->getSize().y - 1) {
        _boardPtr->getTile(Vector2u(_position.x, _position.y + 1))->followWire(SOUTH, _state);
    }
    if (_position.x > 0) {
        _boardPtr->getTile(Vector2u(_position.x - 1, _position.y))->followWire(WEST, _state);
    }
}

Tile* TileSwitch::clone(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates) {
    return new TileSwitch(boardPtr, position, noAdjacentUpdates, _charID, _state);
}