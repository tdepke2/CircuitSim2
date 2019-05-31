#include "Board.h"
#include "TileLED.h"
#include <cassert>
#include <typeinfo>

//#include <iostream>

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

void TileLED::updateLED(State state) {
    assert(traversedLEDs.empty());
    assert(LEDNodes.empty());
    
    //cout << "Update LED started at (" << _position.x << ", " << _position.y << ")." << endl;
    
    _addNextTile(this, NORTH, &state);
    while (!LEDNodes.empty()) {    // Start a depth-first traversal on the connected LEDs.
        TileLED* currentLED = LEDNodes.top();
        LEDNodes.pop();
        
        //cout << "  currently at (" << currentLED->_position.x << ", " << currentLED->_position.y << ")" << endl;
        
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

void TileLED::followWire(Direction direction, State state) {
    Board::endpointLEDs.push_back(this);
}

void TileLED::redrawTile() const {
    _boardPtr->redrawTileVertices(17 + (_state == HIGH), _position, _direction, _highlight);
}

string TileLED::toString() const {
    return OUTPUT_SYMBOL_TABLE[(_state == HIGH)];
}

void TileLED::fixUpdateTime() {
    _updateTimestamp = 0;
}

Tile* TileLED::clone(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates) {
    return new TileLED(boardPtr, position, noAdjacentUpdates, _state);
}

void TileLED::_addNextTile(Tile* nextTile, Direction direction, State* statePtr) {
    if (typeid(*nextTile) == typeid(TileLED)) {    // Check if nextTile is an LED (most common case).
        TileLED* nextLED = static_cast<TileLED*>(nextTile);
        if (nextLED->_updateTimestamp != Tile::currentUpdateTime) {
            nextLED->_state = *statePtr;
            nextLED->_updateTimestamp = Tile::currentUpdateTime;
            nextLED->addUpdate(true);
            traversedLEDs.push_back(nextLED);
            LEDNodes.push(nextLED);
            if (!_boardPtr->LEDUpdates.empty()) {
                _boardPtr->LEDUpdates.erase(nextLED);
            }
        }
    } else if (*statePtr == LOW) {
        if (nextTile->checkOutput(direction) == HIGH) {
            //cout << "  Conflict detected, fixing LEDs." << endl;
            *statePtr = HIGH;
            for (TileLED* LED : traversedLEDs) {
                LED->_state = *statePtr;
            }
        }
    }
}