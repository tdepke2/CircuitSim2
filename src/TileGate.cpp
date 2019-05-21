#include "Board.h"
#include "TileGate.h"
#include "TileWire.h"

//#include <iostream>

TileGate::TileGate(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates, Direction direction, Type type, State state) : Tile(boardPtr, position, true, true) {
    _direction = direction;
    _type = type;
    _state = state;
    _nextState = state;
    addUpdate(false, noAdjacentUpdates);
}

TileGate::~TileGate() {
    _boardPtr->gateUpdates.erase(this);
}

State TileGate::getState() const {
    return _state;
}

State TileGate::getNextState() const {
    return _nextState;
}

void TileGate::setDirection(Direction direction) {
    _direction = direction;
    addUpdate();
}

void TileGate::setState(State state) {
    _state = state;
    addUpdate();
}

void TileGate::flip(bool acrossHorizontal) {
    if ((!acrossHorizontal && _direction % 2 == 1) || (acrossHorizontal && _direction % 2 == 0)) {
        _direction = static_cast<Direction>((_direction + 2) % 4);
        addUpdate();
    }
}

State TileGate::checkOutput(Direction direction) const {
    if ((_direction + 2) % 4 == direction) {
        return _state;
    } else {
        return DISCONNECTED;
    }
}

void TileGate::addUpdate(bool isCosmetic, bool noAdjacentUpdates) {
    _boardPtr->cosmeticUpdates.insert(this);
    if (!isCosmetic) {
        _boardPtr->gateUpdates.insert(this);
        if (!noAdjacentUpdates) {
            _updateAdjacentTiles();
        }
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
    
    //cout << "Gate at (" << _position.x << ", " << _position.y << ") checked for state change:" << endl;
    //cout << "  aS = [" << adjacentStates[0] << ", " << adjacentStates[1] << ", " << adjacentStates[2] << ", " << adjacentStates[3] << "]" << endl;
    //cout << "  state = " << _state << ", next = " << _nextState << endl;
    
    return _nextState != _state;
}

void TileGate::updateOutput() {
    //cout << "Gate at (" << _position.x << ", " << _position.y << ") updated:" << endl;
    _boardPtr->gateUpdates.erase(this);    // Remove this gate update and transition to next state.
    _state = _nextState;
    addUpdate(true);
    
    Vector2u targetPosition;    // Start followWire at the output (the object at output could be anything, not just a wire).
    if (_direction == NORTH) {
        targetPosition = Vector2u(_position.x, _position.y - 1);
    } else if (_direction == EAST) {
        targetPosition = Vector2u(_position.x + 1, _position.y);
    } else if (_direction == SOUTH) {
        targetPosition = Vector2u(_position.x, _position.y + 1);
    } else {
        targetPosition = Vector2u(_position.x - 1, _position.y);
    }
    if (targetPosition.x < _boardPtr->getSize().x && targetPosition.y < _boardPtr->getSize().y) {
        _boardPtr->getTile(targetPosition)->followWire(_direction, _state);
    }
}

void TileGate::followWire(Direction direction, State state) {
    if ((_direction + 2) % 4 != direction) {
        Board::endpointGates.push_back(this);
    }
}

void TileGate::redrawTile() const {
    int textureID;
    bool connectRight = false, connectLeft = false;
    if (_type < 3) {
        textureID = 19 + _type * 6 + connectLeft * 4 + connectRight * 2 + (_state == HIGH);
    } else {
        textureID = 13 + _type * 8 + connectLeft * 4 + connectRight * 2 + (_state == HIGH);
    }
    _boardPtr->redrawTileVertices(textureID, _position, _direction, _highlight);
}

string TileGate::toString() const {
    return GATE_SYMBOL_TABLE[_type * 8 + _direction * 2 + (_state == HIGH)];
}

Tile* TileGate::clone(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates) {
    return new TileGate(boardPtr, position, noAdjacentUpdates, _direction, _type, _state);
}