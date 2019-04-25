#include "Board.h"
#include "TileGate.h"

#include <iostream>

TileGate::TileGate(Board* boardPtr, const Vector2u& position, Direction direction, Type type, State state) : Tile(boardPtr, position, true) {
    _direction = direction;
    _type = type;
    _state = state;
    _nextState = state;
    addUpdate();
}

TileGate::~TileGate() {
    _boardPtr->gateUpdates.erase(this);
}

int TileGate::getTextureID() const {
    bool connectRight = false, connectLeft = false;
    if (_type < 3) {
        return 19 + _type * 6 + connectRight * 2 + connectLeft * 4 + (_state == HIGH);
    } else {
        return 13 + _type * 8 + connectRight * 2 + connectLeft * 4 + (_state == HIGH);
    }
}

bool TileGate::getState() const {
    return _state;
}

bool TileGate::getNextState() const {
    return _nextState;
}

void TileGate::setDirection(Direction direction) {
    _direction = direction;
    addUpdate();
}

void TileGate::flip(bool acrossHorizontal) {
    if ((!acrossHorizontal && _direction % 2 == 1) || (acrossHorizontal && _direction % 2 == 0)) {
        _direction = static_cast<Direction>((_direction + 2) % 4);
        addUpdate();
    }
}

bool TileGate::updateNextState() {
    State adjacentStates [4];
    adjacentStates[0] = _position.y > 0 ? _boardPtr->getTile(Vector2u(_position.x, _position.y - 1))->checkOutput(static_cast<Direction>(0)) : DISCONNECTED;
    adjacentStates[1] = _position.x < _boardPtr->getSize().x - 1 ? _boardPtr->getTile(Vector2u(_position.x + 1, _position.y))->checkOutput(static_cast<Direction>(1)) : DISCONNECTED;
    adjacentStates[2] = _position.y < _boardPtr->getSize().y - 1 ? _boardPtr->getTile(Vector2u(_position.x, _position.y + 1))->checkOutput(static_cast<Direction>(2)) : DISCONNECTED;
    adjacentStates[3] = _position.x > 0 ? _boardPtr->getTile(Vector2u(_position.x - 1, _position.y))->checkOutput(static_cast<Direction>(3)) : DISCONNECTED;
    
    int numInputs = 0, numHigh = 0;
    for (int i = 0; i < 4; ++i) {
        if (i != _direction && adjacentStates[i] != DISCONNECTED) {
            ++numInputs;
            if (adjacentStates[i] == HIGH) {
                ++numHigh;
            }
        }
    }
    
    if (_type == DIODE || _type == BUFFER) {
        if (numInputs == 1 && numHigh == 1) {
            _nextState = HIGH;
        } else {
            _nextState = LOW;
        }
    } else if (_type == NOT) {
        if (numInputs == 1 && numHigh == 1) {
            _nextState = LOW;
        } else {
            _nextState = HIGH;
        }
    } else if (_type == AND) {
        if (numInputs >= 2 && numHigh == numInputs) {
            _nextState = HIGH;
        } else {
            _nextState = LOW;
        }
    } else if (_type == NAND) {
        if (numInputs >= 2 && numHigh == numInputs) {
            _nextState = LOW;
        } else {
            _nextState = HIGH;
        }
    } else if (_type == OR) {
        if (numInputs >= 2 && numHigh >= 1) {
            _nextState = HIGH;
        } else {
            _nextState = LOW;
        }
    } else if (_type == NOR) {
        if (numInputs >= 2 && numHigh >= 1) {
            _nextState = LOW;
        } else {
            _nextState = HIGH;
        }
    } else if (_type == XOR) {
        if (numInputs >= 2 && numHigh % 2 == 1) {
            _nextState = HIGH;
        } else {
            _nextState = LOW;
        }
    } else if (_type == XNOR) {
        if (numInputs >= 2 && numHigh % 2 == 1) {
            _nextState = LOW;
        } else {
            _nextState = HIGH;
        }
    }
    
    cout << "Gate of type " << _type << " updated:" << endl;
    cout << "  aS = [" << adjacentStates[0] << ", " << adjacentStates[1] << ", " << adjacentStates[2] << ", " << adjacentStates[3] << "]" << endl;
    cout << "  current = " << _state << ", next = " << _nextState << endl;
    
    return _nextState != _state;
}

State TileGate::checkOutput(Direction direction) const {
    if ((_direction + 2) % 4 == direction) {
        return _state;
    } else {
        return DISCONNECTED;
    }
}

void TileGate::addUpdate(bool isCosmetic) {
    _boardPtr->cosmeticUpdates.insert(this);
    if (!isCosmetic) {
        _boardPtr->gateUpdates.insert(this);
    }
}

void TileGate::followWire(Direction direction, State state) {
    addUpdate();
    // nah its not that easy
}

Tile* TileGate::clone(Board* boardPtr, const Vector2u& position) {
    return new TileGate(boardPtr, position, _direction, _type, _state);
}