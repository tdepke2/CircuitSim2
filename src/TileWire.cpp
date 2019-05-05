#include "Board.h"
#include "TileGate.h"
#include "TileLED.h"
#include "TileWire.h"
#include <cassert>
#include <typeinfo>

//#include <iostream>

vector<pair<TileWire*, Direction>> TileWire::traversedWires;
stack<pair<TileWire*, Direction>> TileWire::wireNodes;
vector<Tile*> TileWire::endpointTiles;

void TileWire::updateEndpointTiles(Board* boardPtr) {
    //cout << "Endpoints list:" << endl;
    for (Tile* tile : endpointTiles) {
        //cout << "  (" << tile->getPosition().x << ", " << tile->getPosition().y << ")" << endl;
        if (typeid(*tile) == typeid(TileGate)) {
            boardPtr->gateUpdates.insert(static_cast<TileGate*>(tile));
        } else if (typeid(*tile) == typeid(TileLED)) {
            tile->followWire(NORTH, LOW);
        } else {
            assert(false);
        }
    }
    endpointTiles.clear();
}

TileWire::TileWire(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates, Direction direction, Type type, State state1, State state2) : Tile(boardPtr, position, true, true) {
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
    addUpdate(false, noAdjacentUpdates);
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

void TileWire::addUpdate(bool isCosmetic, bool noAdjacentUpdates) {
    _boardPtr->cosmeticUpdates.insert(this);
    if (!isCosmetic) {
        _boardPtr->wireUpdates.insert(this);
        if (!noAdjacentUpdates) {
            _updateAdjacentTiles();
        }
    }
}

void TileWire::followWire(Direction direction, State state) {
    assert(traversedWires.empty());
    assert(wireNodes.empty());
    
    //cout << "Follow wire started at (" << _position.x << ", " << _position.y << ")." << endl;
    
    _addNextTile(this, direction, &state);
    if (wireNodes.empty()) {
        return;
    }
    wireNodes.pop();
    addUpdate(true);
    if (!_boardPtr->wireUpdates.empty() && (_type != CROSSOVER || _updateTimestamp1 == _updateTimestamp2)) {
        _boardPtr->wireUpdates.erase(this);
    }
    
    const bool* exitDirections = CONNECTION_INFO[_direction][_type][direction];    // Check for connection with all adjacent tiles.
    if (exitDirections[0] && _position.y > 0) {
        _addNextTile(_boardPtr->getTile(Vector2u(_position.x, _position.y - 1)), NORTH, &state);
    }
    if (exitDirections[1] && _position.x < _boardPtr->getSize().x - 1) {
        _addNextTile(_boardPtr->getTile(Vector2u(_position.x + 1, _position.y)), EAST, &state);
    }
    if (exitDirections[2] && _position.y < _boardPtr->getSize().y - 1) {
        _addNextTile(_boardPtr->getTile(Vector2u(_position.x, _position.y + 1)), SOUTH, &state);
    }
    if (exitDirections[3] && _position.x > 0) {
        _addNextTile(_boardPtr->getTile(Vector2u(_position.x - 1, _position.y)), WEST, &state);
    }
    
    while (!wireNodes.empty()) {
        TileWire* currentWire = wireNodes.top().first;
        Direction currentDirection = wireNodes.top().second;
        wireNodes.pop();
        //cout << "  currently at (" << currentWire->_position.x << ", " << currentWire->_position.y << ") going direction " << currentDirection << endl;
        currentWire->addUpdate(true);
        if (!_boardPtr->wireUpdates.empty() && (currentWire->_type != CROSSOVER || currentWire->_updateTimestamp1 == currentWire->_updateTimestamp2)) {
            _boardPtr->wireUpdates.erase(currentWire);
        }
        
        const bool* exitDirections = CONNECTION_INFO[currentWire->_direction][currentWire->_type][currentDirection];    // Check for connection with adjacent tiles that do not point back to source wire.
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
    traversedWires.clear();
}

Tile* TileWire::clone(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates) {
    return new TileWire(boardPtr, position, noAdjacentUpdates, _direction, _type, _state1, _state2);
}

void TileWire::updateWire(State state) {
    if (_type == CROSSOVER) {
        if (_updateTimestamp1 != Tile::currentUpdateTime) {
            followWire(NORTH, state);
        } else {
            followWire(EAST, state);
        }
    } else {
        for (int i = 0; i < 4; ++i) {
            const bool* exitDirections = CONNECTION_INFO[_direction][_type][i];
            for (int j = 0; j < 4; ++j) {
                if (exitDirections[j] == true) {
                    followWire(static_cast<Direction>(i), state);
                    return;
                }
            }
        }
    }
}

void TileWire::_addNextTile(Tile* nextTile, Direction direction, State* statePtr) {
    if (typeid(*nextTile) == typeid(TileWire)) {
        TileWire* nextWire = static_cast<TileWire*>(nextTile);
        if (CONNECTION_INFO[nextWire->_direction][nextWire->_type][direction][(direction + 2) % 4]) {
            if (nextWire->_type == CROSSOVER && direction % 2 == 1) {    // Update state of the wire and timestamp if this wire has not been traversed yet.
                if (nextWire->_updateTimestamp2 != Tile::currentUpdateTime) {
                    nextWire->_state2 = *statePtr;
                    nextWire->_updateTimestamp2 = Tile::currentUpdateTime;
                    traversedWires.push_back(pair<TileWire*, Direction>(nextWire, direction));
                    wireNodes.push(pair<TileWire*, Direction>(nextWire, direction));
                }
            } else if (nextWire->_updateTimestamp1 != Tile::currentUpdateTime) {
                nextWire->_state1 = *statePtr;
                nextWire->_updateTimestamp1 = Tile::currentUpdateTime;
                traversedWires.push_back(pair<TileWire*, Direction>(nextWire, direction));
                wireNodes.push(pair<TileWire*, Direction>(nextWire, direction));
            }
        }
    } else if (typeid(*nextTile) == typeid(TileGate)) {
        TileGate* nextGate = static_cast<TileGate*>(nextTile);
        State gateState = (nextGate->getDirection() + 2) % 4 == direction ? nextGate->getNextState() : DISCONNECTED;
        if (gateState != DISCONNECTED) {
            if (gateState == HIGH && *statePtr == LOW) {
                //cout << "  Found a state conflict." << endl;
                *statePtr = HIGH;
                for (pair<TileWire*, Direction>& wire : traversedWires) {
                    if (wire.first->_type == CROSSOVER && wire.second % 2 == 1) {
                        wire.first->_state2 = *statePtr;
                    } else {
                        wire.first->_state1 = *statePtr;
                    }
                }
            }
        } else {
            endpointTiles.push_back(nextTile);
        }
    } else if (typeid(*nextTile) == typeid(TileLED)) {
        endpointTiles.push_back(nextTile);
    }
}