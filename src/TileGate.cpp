#include "Board.h"
#include "TileGate.h"
#include "TileWire.h"

//#include <iostream>

TileGate::TileGate(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates, Direction direction, Type type, State state) : Tile(boardPtr, position, true, true) {
    _direction = direction;
    _type = type;
    _state = state;
    _nextState = state;
    _connectorChange = false;
    _leftConnector = false;
    _rightConnector = false;
    addUpdate(false, noAdjacentUpdates);
}

TileGate::~TileGate() {
    getBoardPtr()->gateUpdates.erase(this);
}

TileGate::Type TileGate::getType() const {
    return _type;
}

State TileGate::getState() const {
    return _state;
}

State TileGate::getNextState() const {
    return _nextState;
}

void TileGate::setDirection(Direction direction, bool noAdjacentUpdates) {
    _direction = direction;
    addUpdate(false, noAdjacentUpdates);
    getBoardPtr()->changesMade = true;
}

void TileGate::setState(State state) {
    _state = state;
    addUpdate();
}

void TileGate::flip(bool acrossHorizontal, bool noAdjacentUpdates) {
    if ((!acrossHorizontal && _direction % 2 == 1) || (acrossHorizontal && _direction % 2 == 0)) {
        _direction = static_cast<Direction>((_direction + 2) % 4);
        addUpdate(false, noAdjacentUpdates);
        getBoardPtr()->changesMade = true;
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
    getBoardPtr()->cosmeticUpdates.insert(this);
    if (!isCosmetic) {
        getBoardPtr()->gateUpdates.insert(this);
        if (!noAdjacentUpdates) {
            _updateAdjacentTiles();
        }
        _connectorChange = true;
    }
}

bool TileGate::updateNextState() {
    State adjacentStates[4];
    adjacentStates[0] = getPosition().y > 0 ? getBoardPtr()->getTile(Vector2u(getPosition().x, getPosition().y - 1))->checkOutput(NORTH) : DISCONNECTED;
    adjacentStates[1] = getPosition().x < getBoardPtr()->getSize().x - 1 ? getBoardPtr()->getTile(Vector2u(getPosition().x + 1, getPosition().y))->checkOutput(EAST) : DISCONNECTED;
    adjacentStates[2] = getPosition().y < getBoardPtr()->getSize().y - 1 ? getBoardPtr()->getTile(Vector2u(getPosition().x, getPosition().y + 1))->checkOutput(SOUTH) : DISCONNECTED;
    adjacentStates[3] = getPosition().x > 0 ? getBoardPtr()->getTile(Vector2u(getPosition().x - 1, getPosition().y))->checkOutput(WEST) : DISCONNECTED;
    
    int numInputs = 0, numHigh = 0, numMiddle = 0;
    for (int i = 0; i < 4; ++i) {
        if (i != _direction && adjacentStates[i] != DISCONNECTED) {
            ++numInputs;
            if (adjacentStates[i] == HIGH) {
                ++numHigh;
            } else if (adjacentStates[i] == MIDDLE) {
                ++numMiddle;
            }
        }
    }
    if (!Board::enableExtraLogicStates) {
        numMiddle = 0;
    }
    if (_connectorChange) {
        _rightConnector = (adjacentStates[(_direction + 1) % 4] != DISCONNECTED);
        _leftConnector = (adjacentStates[(_direction + 3) % 4] != DISCONNECTED);
        if (_type < AND && _rightConnector && _leftConnector) {
            _rightConnector = false;
            _leftConnector = false;
        }
        _connectorChange = false;
        addUpdate(true);
    }
    
    if (_type == DIODE) {
        if (numInputs == 1) {
            if (numHigh == 1) {
                _nextState = HIGH;
            } else if (numMiddle == 1) {
                _nextState = MIDDLE;
            } else {
                _nextState = LOW;
            }
        } else {
            _nextState = LOW;
        }
    } else if (_type == BUFFER) {
        _nextState = _findNextStateBuffer(adjacentStates, numInputs, numHigh, numMiddle);
    } else if (_type == NOT) {
        _nextState = _complementState(_findNextStateBuffer(adjacentStates, numInputs, numHigh, numMiddle));
    } else if (_type == AND) {
        _nextState = _findNextStateAND(adjacentStates, numInputs, numHigh, numMiddle);
    } else if (_type == NAND) {
        _nextState = _complementState(_findNextStateAND(adjacentStates, numInputs, numHigh, numMiddle));
    } else if (_type == OR) {
        _nextState = _findNextStateOR(adjacentStates, numInputs, numHigh, numMiddle);
    } else if (_type == NOR) {
        _nextState = _complementState(_findNextStateOR(adjacentStates, numInputs, numHigh, numMiddle));
    } else if (_type == XOR) {
        _nextState = _findNextStateXOR(adjacentStates, numInputs, numHigh, numMiddle);
    } else if (_type == XNOR) {
        _nextState = _complementState(_findNextStateXOR(adjacentStates, numInputs, numHigh, numMiddle));
    }
    
    //cout << "Gate at (" << getPosition().x << ", " << getPosition().y << ") checked for state change:" << endl;
    //cout << "  aS = [" << adjacentStates[0] << ", " << adjacentStates[1] << ", " << adjacentStates[2] << ", " << adjacentStates[3] << "]" << endl;
    //cout << "  state = " << _state << ", next = " << _nextState << endl;
    
    return _nextState != _state;
}

void TileGate::updateOutput() {
    //cout << "Gate at (" << getPosition().x << ", " << getPosition().y << ") updated:" << endl;
    getBoardPtr()->gateUpdates.erase(this);    // Remove this gate update and transition to next state.
    _state = _nextState;
    addUpdate(true);
    
    Vector2u targetPosition;    // Start followWire at the output (the object at output could be anything, not just a wire).
    if (_direction == NORTH) {
        targetPosition = Vector2u(getPosition().x, getPosition().y - 1);
    } else if (_direction == EAST) {
        targetPosition = Vector2u(getPosition().x + 1, getPosition().y);
    } else if (_direction == SOUTH) {
        targetPosition = Vector2u(getPosition().x, getPosition().y + 1);
    } else {
        targetPosition = Vector2u(getPosition().x - 1, getPosition().y);
    }
    if (targetPosition.x < getBoardPtr()->getSize().x && targetPosition.y < getBoardPtr()->getSize().y) {
        getBoardPtr()->getTile(targetPosition)->followWire(_direction, _state);
    }
}

void TileGate::followWire(Direction direction, State state) {
    if ((_direction + 2) % 4 != direction) {
        Board::endpointGates.push_back(this);
    }
}

void TileGate::redrawTile() const {
    int textureID;
    if (_type < AND) {
        textureID = 28 + _type * 9 + _state - 1 + _rightConnector * 3 + _leftConnector * 6;
    } else {
        textureID = 19 + _type * 12 + _state - 1 + _rightConnector * 3 + _leftConnector * 6;
    }
    getBoardPtr()->redrawTileVertices(textureID, getPosition(), _direction, getHighlight());
}

string TileGate::toString() const {
    return GATE_SYMBOL_TABLE[_type * 12 + _direction * 3 + _state - 1];
}

bool TileGate::alternativeTile() {
    if (_type != DIODE) {
        if (_type % 2 == 1) {
            _type = static_cast<Type>(_type + 1);
        } else {
            _type = static_cast<Type>(_type - 1);
        }
        addUpdate();
        getBoardPtr()->changesMade = true;
    }
    return false;
}

Tile* TileGate::clone(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates) {
    return new TileGate(boardPtr, position, noAdjacentUpdates, _direction, _type, _state);
}

State TileGate::_complementState(State state) const {
    if (state == LOW) {
        return HIGH;
    } else if (state == HIGH) {
        return LOW;
    } else {
        return state;
    }
}

State TileGate::_findNextStateBuffer(State adjacentStates[4], int numInputs, int numHigh, int numMiddle) const {
    if (numInputs == 1) {
        if (numHigh == 1) {
            return HIGH;
        } else if (numMiddle == 1) {
            return MIDDLE;
        }
    } else if (numInputs == 2 && Board::enableExtraLogicStates) {
        State inputState = adjacentStates[(_direction + 2) % 4];
        if (inputState != DISCONNECTED) {
            State controlState = adjacentStates[(_direction + 1) % 4];
            if (controlState == DISCONNECTED) {
                controlState = adjacentStates[(_direction + 3) % 4];
            }
            if (controlState == HIGH) {
                return inputState;
            } else {
                return MIDDLE;
            }
        }
    }
    return LOW;
}

State TileGate::_findNextStateAND(State adjacentStates[4], int numInputs, int numHigh, int numMiddle) const {
    if (numInputs >= 2) {
        if (numHigh == numInputs) {
            return HIGH;
        } else if (numHigh + numMiddle == numInputs) {
            return MIDDLE;
        }
    }
    return LOW;
}

State TileGate::_findNextStateOR(State adjacentStates[4], int numInputs, int numHigh, int numMiddle) const {
    if (numInputs >= 2) {
        if (numHigh >= 1) {
            return HIGH;
        } else if (numMiddle >= 1) {
            return MIDDLE;
        }
    }
    return LOW;
}

State TileGate::_findNextStateXOR(State adjacentStates[4], int numInputs, int numHigh, int numMiddle) const {
    if (numInputs >= 2) {
        if (numMiddle == 0) {
            if (numHigh % 2 == 1) {
                return HIGH;
            }
        } else {
            return MIDDLE;
        }
    }
    return LOW;
}