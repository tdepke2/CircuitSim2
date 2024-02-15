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
    getBoardPtr()->LEDUpdates.erase(this);
}

State TileLED::getState() const {
    return _state;
}

void TileLED::setState(State state) {
    _state = state;
    addUpdate();
}

void TileLED::addUpdate(bool isCosmetic, bool noAdjacentUpdates) {
    getBoardPtr()->cosmeticUpdates.insert(this);
    if (!isCosmetic) {
        getBoardPtr()->LEDUpdates.insert(this);
        if (!noAdjacentUpdates) {
            _updateAdjacentTiles();
        }
    }
}

void TileLED::updateLED(State state) {
    assert(traversedLEDs.empty());
    assert(LEDNodes.empty());
    
    //cout << "Update LED started at (" << getPosition().x << ", " << getPosition().y << ")." << endl;
    
    _addNextTile(this, NORTH, state);
    while (!LEDNodes.empty()) {    // Start a depth-first traversal on the connected LEDs.
        TileLED* currentLED = LEDNodes.top();
        LEDNodes.pop();
        
        //cout << "  currently at (" << currentLED->getPosition().x << ", " << currentLED->getPosition().y << ")" << endl;
        
        if (currentLED->getPosition().y > 0) {
            _addNextTile(getBoardPtr()->getTile(Vector2u(currentLED->getPosition().x, currentLED->getPosition().y - 1)), NORTH, state);
        }
        if (currentLED->getPosition().x < getBoardPtr()->getSize().x - 1) {
            _addNextTile(getBoardPtr()->getTile(Vector2u(currentLED->getPosition().x + 1, currentLED->getPosition().y)), EAST, state);
        }
        if (currentLED->getPosition().y < getBoardPtr()->getSize().y - 1) {
            _addNextTile(getBoardPtr()->getTile(Vector2u(currentLED->getPosition().x, currentLED->getPosition().y + 1)), SOUTH, state);
        }
        if (currentLED->getPosition().x > 0) {
            _addNextTile(getBoardPtr()->getTile(Vector2u(currentLED->getPosition().x - 1, currentLED->getPosition().y)), WEST, state);
        }
    }
    traversedLEDs.clear();
}

void TileLED::followWire(Direction direction, State state) {
    Board::endpointLEDs.push_back(this);
}

void TileLED::redrawTile() const {
    getBoardPtr()->redrawTileVertices(26 + _state - 1, getPosition(), _direction, getHighlight());
}

string TileLED::toString() const {
    return OUTPUT_SYMBOL_TABLE[_state - 1];
}

void TileLED::fixUpdateTime() {
    _updateTimestamp = 0;
}

Tile* TileLED::clone(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates) {
    return new TileLED(boardPtr, position, noAdjacentUpdates, _state);
}

void TileLED::_addNextTile(Tile* nextTile, Direction direction, State& state) const {
    if (typeid(*nextTile) == typeid(TileLED)) {    // Check if nextTile is an LED (most common case).
        TileLED* nextLED = static_cast<TileLED*>(nextTile);
        if (nextLED->_updateTimestamp != Tile::currentUpdateTime) {
            nextLED->_state = state;
            nextLED->_updateTimestamp = Tile::currentUpdateTime;
            nextLED->addUpdate(true);
            traversedLEDs.push_back(nextLED);
            LEDNodes.push(nextLED);
            if (!getBoardPtr()->LEDUpdates.empty()) {
                getBoardPtr()->LEDUpdates.erase(nextLED);
            }
        }
    } else if (state == LOW) {
        if (nextTile->checkOutput(direction) == HIGH) {
            state = HIGH;
            for (TileLED* LED : traversedLEDs) {
                LED->_state = state;
            }
        }
    }
}