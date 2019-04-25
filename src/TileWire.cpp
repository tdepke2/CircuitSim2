#include "Board.h"
#include "TileWire.h"
#include <cassert>
#include <typeinfo>

#include <iostream>

unsigned int TileWire::currentUpdateTime = 1;
vector<TileWire*> TileWire::traversedWires;
stack<pair<TileWire*, Direction>> TileWire::wireNodes;
vector<pair<Tile*, Direction>> TileWire::endpointTiles;

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
    _updateTimestamp = 0;
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
    assert(endpointTiles.empty());
    assert(traversedWires.empty());
    assert(wireNodes.empty());
    
    cout << "Follow wire started at (" << _position.x << ", " << _position.y << ")." << endl;
    
    wireNodes.push(pair<TileWire*, Direction>(this, direction));
    while (!wireNodes.empty()) {
        TileWire* currentWire = wireNodes.top().first;
        Direction currentDirection = wireNodes.top().second;
        wireNodes.pop();
        
        cout << "  currently at (" << currentWire->_position.x << ", " << currentWire->_position.y << ") going direction " << currentDirection << endl;
        
        if (currentWire->_updateTimestamp != currentUpdateTime || currentWire->_type == CROSSOVER) {    // If wire not traversed yet or its a crossover.
            currentWire->_updateTimestamp = currentUpdateTime;
            traversedWires.push_back(currentWire);
            _boardPtr->wireUpdates.erase(currentWire);    // ###################################################################### may be too costly need to optimize
            
            const bool* exitDirections = CONNECTION_INFO[currentWire->_direction][currentWire->_type][currentDirection];
            if (exitDirections[0] && currentWire->_position.y > 0) {
                _addNextTile(_boardPtr->getTile(Vector2u(currentWire->_position.x, currentWire->_position.y - 1)), NORTH);
            }
            if (exitDirections[1] && currentWire->_position.x < _boardPtr->getSize().x - 1) {
                _addNextTile(_boardPtr->getTile(Vector2u(currentWire->_position.x + 1, currentWire->_position.y)), EAST);
            }
            if (exitDirections[2] && currentWire->_position.y < _boardPtr->getSize().y - 1) {
                _addNextTile(_boardPtr->getTile(Vector2u(currentWire->_position.x, currentWire->_position.y + 1)), SOUTH);
            }
            if (exitDirections[3] && currentWire->_position.x > 0) {
                _addNextTile(_boardPtr->getTile(Vector2u(currentWire->_position.x - 1, currentWire->_position.y)), WEST);
            }
        }
    }
    
    cout << "Finished, endpoints found:" << endl;
    for (auto e : endpointTiles) {
        cout << "  (" << e.first->getPosition().x << ", " << e.first->getPosition().y << ") with direction " << e.second << endl;
    }
    endpointTiles.clear();
    traversedWires.clear();
}

Tile* TileWire::clone(Board* boardPtr, const Vector2u& position) {
    return new TileWire(boardPtr, position, _direction, _type, _state1, _state2);
}

void TileWire::_addNextTile(Tile* nextTile, Direction direction) {
    if (typeid(*nextTile) == typeid(TileWire)) {
        wireNodes.push(pair<TileWire*, Direction>(static_cast<TileWire*>(nextTile), direction));
    } else {
        endpointTiles.push_back(pair<Tile*, Direction>(nextTile, direction));
    }
}