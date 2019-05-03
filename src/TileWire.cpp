#include "Board.h"
#include "TileGate.h"
#include "TileLED.h"
#include "TileWire.h"
#include <cassert>
#include <typeinfo>

#include <iostream>

unsigned int TileWire::currentUpdateTime = 1;
vector<pair<TileWire*, Direction>> TileWire::traversedWires;
stack<pair<TileWire*, Direction>> TileWire::wireNodes;
vector<Tile*> TileWire::endpointTiles;

void TileWire::updateEndpointTiles() {
    cout << "Endpoints list:" << endl;
    for (const Tile* tile : endpointTiles) {
        cout << "  (" << tile->getPosition().x << ", " << tile->getPosition().y << ")" << endl;
    }
    endpointTiles.clear();
}

TileWire::TileWire(Board* boardPtr, const Vector2u& position, Direction direction, Type type, State state1, State state2) : Tile(boardPtr, position, true) {
    if (type != JUNCTION && type != CROSSOVER) {
        if (type == STRAIGHT) {
            _direction = static_cast<Direction>(direction % 2);
        } else {
            _direction = direction;
        }
    }
    _type = type;
    _state1 = state1;
    _state2 = state2;
    _updateTimestamp1 = 0;
    _updateTimestamp2 = 0;
    addUpdate();
}

TileWire::~TileWire() {
    _boardPtr->wireUpdates.erase(this);
}

int TileWire::getTextureID() const {
    return 1 + _type * 2 + _state1 - 1 + (_state2 - 1) * 2;
}

void TileWire::setDirection(Direction direction) {
    if (_type != JUNCTION) {
        if (_type != CROSSOVER) {
            if (_type == STRAIGHT) {
                _direction = static_cast<Direction>(direction % 2);
            } else {
                _direction = direction;
            }
            addUpdate();
        } else if (direction % 2 == 1) {    // If direction odd, assume 1 or 3 rotations were made to the crossover wire.
            State tempState = _state1;
            _state1 = _state2;
            _state2 = tempState;
            addUpdate();
        }
    }
}

void TileWire::flip(bool acrossHorizontal) {
    if (_type == CORNER) {
        if (!acrossHorizontal) {
            _direction = static_cast<Direction>(3 - _direction);
        } else if (_direction % 2 == 0) {
            _direction = static_cast<Direction>(_direction + 1);
        } else {
            _direction = static_cast<Direction>(_direction - 1);
        }
        addUpdate();
    } else if (_type == TEE && ((!acrossHorizontal && _direction % 2 == 0) || (acrossHorizontal && _direction % 2 == 1))) {
        _direction = static_cast<Direction>((_direction + 2) % 4);
        addUpdate();
    }
}

State TileWire::checkOutput(Direction direction) const {
    const bool* exitDirections = CONNECTION_INFO[_direction][_type][direction];
    if (exitDirections[0] || exitDirections[1] || exitDirections[2] || exitDirections[3]) {
        if (_type == CROSSOVER && direction % 2 == 1) {
            return _state2;
        } else {
            return _state1;
        }
    } else {
        return DISCONNECTED;
    }
}

void TileWire::addUpdate(bool isCosmetic) {
    _boardPtr->cosmeticUpdates.insert(this);
    if (!isCosmetic) {
        _boardPtr->wireUpdates.insert(this);
    }
}

void TileWire::followWire(Direction direction, State state) {
    assert(traversedWires.empty());
    assert(wireNodes.empty());
    
    cout << "Follow wire started at (" << _position.x << ", " << _position.y << ")." << endl;
    
    const State originalState = state;
    wireNodes.push(pair<TileWire*, Direction>(this, direction));
    while (!wireNodes.empty()) {
        TileWire* currentWire = wireNodes.top().first;
        Direction currentDirection = wireNodes.top().second;
        wireNodes.pop();
        
        bool traverseWire = false;
        if (currentWire->_type == CROSSOVER && currentDirection % 2 == 1) {
            if (currentWire->_updateTimestamp2 != currentUpdateTime) {
                currentWire->_state2 = state;
                currentWire->_updateTimestamp2 = currentUpdateTime;
                traverseWire = true;
            }
        } else if (currentWire->_updateTimestamp1 != currentUpdateTime) {
            currentWire->_state1 = state;
            currentWire->_updateTimestamp1 = currentUpdateTime;
            traverseWire = true;
        }
        
        if (traverseWire) {
            cout << "  currently at (" << currentWire->_position.x << ", " << currentWire->_position.y << ") going direction " << currentDirection << endl;
            currentWire->addUpdate(true);
            traversedWires.push_back(pair<TileWire*, Direction>(currentWire, currentDirection));
            if (!_boardPtr->wireUpdates.empty() && (currentWire->_type != CROSSOVER || currentWire->_updateTimestamp1 == currentWire->_updateTimestamp2)) {
                _boardPtr->wireUpdates.erase(currentWire);
            }
            
            const bool* exitDirections = CONNECTION_INFO[currentWire->_direction][currentWire->_type][currentDirection];
            if (exitDirections[0] && currentDirection != SOUTH && currentWire->_position.y > 0) {
                _addNextTile(_boardPtr->getTile(Vector2u(currentWire->_position.x, currentWire->_position.y - 1)), NORTH, &state);
            }
            if (exitDirections[1] && currentDirection != WEST && currentWire->_position.x < _boardPtr->getSize().x - 1) {
                _addNextTile(_boardPtr->getTile(Vector2u(currentWire->_position.x + 1, currentWire->_position.y)), EAST, &state);
            }
            if (exitDirections[2] && currentDirection != NORTH && currentWire->_position.y < _boardPtr->getSize().y - 1) {
                _addNextTile(_boardPtr->getTile(Vector2u(currentWire->_position.x, currentWire->_position.y + 1)), SOUTH, &state);
            }
            if (exitDirections[3] && currentDirection != EAST && currentWire->_position.x > 0) {
                _addNextTile(_boardPtr->getTile(Vector2u(currentWire->_position.x - 1, currentWire->_position.y)), WEST, &state);
            }
        }
    }
    traversedWires.clear();
}

Tile* TileWire::clone(Board* boardPtr, const Vector2u& position) {
    return new TileWire(boardPtr, position, _direction, _type, _state1, _state2);
}

void TileWire::_addNextTile(Tile* nextTile, Direction direction, State* statePtr) {
    if (typeid(*nextTile) == typeid(TileWire)) {
        TileWire* nextWire = static_cast<TileWire*>(nextTile);
        if (CONNECTION_INFO[nextWire->_direction][nextWire->_type][direction][(direction + 2) % 4]) {
            wireNodes.push(pair<TileWire*, Direction>(nextWire, direction));
        }
    } else if (typeid(*nextTile) == typeid(TileGate)) {
        TileGate* nextGate = static_cast<TileGate*>(nextTile);
        State gateState = (nextGate->getDirection() + 2) % 4 == direction ? nextGate->getNextState() : DISCONNECTED;
        if (gateState != DISCONNECTED) {
            if (gateState == HIGH && *statePtr == LOW) {
                cout << "Found a state conflict." << endl;
                *statePtr = HIGH;
                for (pair<TileWire*, Direction>& wire : traversedWires) {
                    if (wire.first->_type == CROSSOVER && wire.second % 2 == 1) {
                        wire.first->_state2 = *statePtr;
                    } else {
                        wire.first->_state1 = *statePtr;
                    }
                }
            }
            _boardPtr->gateUpdates.erase(nextGate);
        } else {
            endpointTiles.push_back(nextTile);
        }
    } else if (typeid(*nextTile) == typeid(TileLED)) {
        endpointTiles.push_back(nextTile);
    }
}