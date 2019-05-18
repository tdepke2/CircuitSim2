#include "Board.h"
#include "TileLED.h"
#include <cassert>
#include <typeinfo>

#include <iostream>

vector<TileLED*> TileLED::traversedLEDs;
stack<TileLED*> TileLED::LEDNodes;

TileLED::TileLED(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates, State state) : Tile(boardPtr, position, true, true) {
    _state = state;
    _updateTimestamp = 0;
    addUpdate(false, noAdjacentUpdates);
}

TileLED::~TileLED() {
    _boardPtr->LEDUpdates.erase(this);
}

int TileLED::getTextureID() const {
    return 17 + (_state == HIGH);
}

State TileLED::getState() const {
    return _state;
}

void TileLED::setState(State state) {
    _state = state;
    addUpdate();
}

void TileLED::addUpdate(bool isCosmetic, bool noAdjacentUpdates) {
    _boardPtr->cosmeticUpdates.insert(this);
    if (!isCosmetic) {
        _boardPtr->LEDUpdates.insert(this);
        if (!noAdjacentUpdates) {
            _updateAdjacentTiles();
        }
    }
}

void TileLED::followWire(Direction direction, State state) {
    assert(traversedLEDs.empty());
    assert(LEDNodes.empty());
    
    cout << "Follow LED started at (" << _position.x << ", " << _position.y << ")." << endl;
    
    if (_updateTimestamp != Tile::currentUpdateTime) {
        LEDNodes.push(this);
    }
    while (!LEDNodes.empty()) {
        TileLED* currentLED = LEDNodes.top();
        LEDNodes.pop();
        currentLED->_state = state;
        currentLED->_updateTimestamp = Tile::currentUpdateTime;
        //cout << "  currently at (" << currentLED->_position.x << ", " << currentLED->_position.y << ")" << endl;
        currentLED->addUpdate(true);
        traversedLEDs.push_back(currentLED);
        if (!_boardPtr->LEDUpdates.empty()) {
            _boardPtr->LEDUpdates.erase(currentLED);
        }
        
        if (currentLED->_position.y > 0) {
            _addNextTile(_boardPtr->getTile(Vector2u(currentLED->_position.x, currentLED->_position.y - 1)), NORTH, &state);
        }
        if (currentLED->_position.x < _boardPtr->getSize().x - 1) {
            _addNextTile(_boardPtr->getTile(Vector2u(currentLED->_position.x + 1, currentLED->_position.y)), EAST, &state);
        }
        if (currentLED->_position.y < _boardPtr->getSize().y - 1) {
            _addNextTile(_boardPtr->getTile(Vector2u(currentLED->_position.x, currentLED->_position.y + 1)), SOUTH, &state);
        }
        if (currentLED->_position.x > 0) {
            _addNextTile(_boardPtr->getTile(Vector2u(currentLED->_position.x - 1, currentLED->_position.y)), WEST, &state);
        }
    }
    traversedLEDs.clear();
}

Tile* TileLED::clone(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates) {
    return new TileLED(boardPtr, position, noAdjacentUpdates, _state);
}

void TileLED::_addNextTile(Tile* nextTile, Direction direction, State* statePtr) {
    if (typeid(*nextTile) == typeid(TileLED)) {
        TileLED* nextLED = static_cast<TileLED*>(nextTile);
        if (nextLED->_updateTimestamp != Tile::currentUpdateTime) {
            LEDNodes.push(nextLED);
        }
    } else if (*statePtr == LOW) {
        //cout << "  Found an output from a tile at (" << nextTile->getPosition().x << ", " << nextTile->getPosition().y << "), state = " << tileState << endl;
        if (nextTile->checkOutput(direction) == HIGH) {
            //cout << "  Conflict detected, fixing LEDs." << endl;
            *statePtr = HIGH;
            for (TileLED* LED : traversedLEDs) {
                LED->_state = *statePtr;
            }
        }
    }
}