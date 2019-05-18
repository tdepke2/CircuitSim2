#include "Board.h"
#include "TileButton.h"

#include <iostream>

TileButton::TileButton(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates, char charID, State state) : Tile(boardPtr, position, true, true) {
    _charID = charID;
    _state = state;
    boardPtr->buttonKeybinds[charID].push_back(this);
    addUpdate(false, noAdjacentUpdates);
}

TileButton::~TileButton() {
    _boardPtr->buttonUpdates.erase(this);
    auto mapIter = _boardPtr->buttonKeybinds.find(_charID);
    for (auto vectorIter = mapIter->second.begin(); vectorIter != mapIter->second.end(); ++vectorIter) {
        if (*vectorIter == this) {
            mapIter->second.erase(vectorIter);
            break;
        }
    }
}

int TileButton::getTextureID() const {
    return 15 + (_state == HIGH);
}

State TileButton::getState() const {
    return _state;
}

void TileButton::setCharID(char charID) {
    auto mapIter = _boardPtr->buttonKeybinds.find(_charID);
    for (auto vectorIter = mapIter->second.begin(); vectorIter != mapIter->second.end(); ++vectorIter) {
        if (*vectorIter == this) {
            mapIter->second.erase(vectorIter);
            break;
        }
    }
    _charID = charID;
    _boardPtr->buttonKeybinds[_charID].push_back(this);
    addUpdate(true);
}

void TileButton::setState(State state) {
    _state = state;
    addUpdate();
}

State TileButton::checkOutput(Direction direction) const {
    return _state;
}

void TileButton::addUpdate(bool isCosmetic, bool noAdjacentUpdates) {
    _boardPtr->cosmeticUpdates.insert(this);
    if (!isCosmetic) {
        _boardPtr->buttonUpdates.insert(this);
        if (!noAdjacentUpdates) {
            _updateAdjacentTiles();
        }
    }
}

void TileButton::updateOutput() {
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
    
    if (_state == LOW) {
        _boardPtr->buttonUpdates.erase(this);
        cout << "Update removed." << endl;
    } else {
        _state = LOW;
    }
}

Tile* TileButton::clone(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates) {
    return new TileButton(boardPtr, position, noAdjacentUpdates, _charID, _state);
}