#include "Board.h"
#include "TileButton.h"
#include "TileGate.h"
#include "TileLED.h"
#include "TileSwitch.h"
#include "TileWire.h"
#include <cassert>
#include <iostream>
#include <typeinfo>

vector<pair<TileWire*, Direction>> TileWire::traversedWires;
stack<pair<TileWire*, Direction>> TileWire::wireNodes;

TileWire::TileWire(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates, Direction direction, Type type, State state1, State state2) : Tile(boardPtr, position, true, true) {
    if (type == STRAIGHT) {
        _direction = static_cast<Direction>(direction % 2);
    } else if (type != JUNCTION && type != CROSSOVER) {
        _direction = direction;
    }
    _type = type;
    _state1 = state1;
    if (type == CROSSOVER) {
        _state2 = state2;
    } else {
        _state2 = LOW;
    }
    _updateTimestamp1 = 0;
    _updateTimestamp2 = 0;
    addUpdate(false, noAdjacentUpdates);
}

TileWire::~TileWire() {
    _boardPtr->wireUpdates.erase(this);
}

TileWire::Type TileWire::getType() const {
    return _type;
}

State TileWire::getState() const {
    return _state1;
}

void TileWire::setDirection(Direction direction, bool noAdjacentUpdates) {
    if (_type != JUNCTION) {
        if (_type != CROSSOVER) {
            if (_type == STRAIGHT) {
                _direction = static_cast<Direction>(direction % 2);
            } else {
                _direction = direction;
            }
            addUpdate(false, noAdjacentUpdates);
            _boardPtr->changesMade = true;
        } else if (direction % 2 == 1) {    // If direction odd, assume 1 or 3 rotations were made to the crossover wire.
            State tempState = _state1;
            _state1 = _state2;
            _state2 = tempState;
            addUpdate(false, noAdjacentUpdates);
            _boardPtr->changesMade = true;
        }
    }
}

void TileWire::setState(State state) {
    _state1 = state;
    if (_type == CROSSOVER) {
        _state2 = state;
    }
    addUpdate();
}

void TileWire::flip(bool acrossHorizontal, bool noAdjacentUpdates) {
    if (_type == CORNER) {
        if (!acrossHorizontal) {
            _direction = static_cast<Direction>(3 - _direction);
        } else if (_direction % 2 == 0) {
            _direction = static_cast<Direction>(_direction + 1);
        } else {
            _direction = static_cast<Direction>(_direction - 1);
        }
        addUpdate(false, noAdjacentUpdates);
        _boardPtr->changesMade = true;
    } else if (_type == TEE && ((!acrossHorizontal && _direction % 2 == 0) || (acrossHorizontal && _direction % 2 == 1))) {
        _direction = static_cast<Direction>((_direction + 2) % 4);
        addUpdate(false, noAdjacentUpdates);
        _boardPtr->changesMade = true;
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

void TileWire::updateWire(State state) {
    if (_type == CROSSOVER) {
        if (_updateTimestamp1 != Tile::currentUpdateTime) {    // For crossover, check both paths for update time mismatch.
            followWire(NORTH, state);
        }
        if (_updateTimestamp2 != Tile::currentUpdateTime) {
            followWire(EAST, state);
        }
    } else {
        for (int i = 0; i < 4; ++i) {    // Check combinations of entry and exit directions through this wire until one works out.
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

void TileWire::followWire(Direction direction, State state) {
    assert(traversedWires.empty());
    assert(wireNodes.empty());
    
    //cout << "Follow wire started at (" << _position.x << ", " << _position.y << ")." << endl;
    
    FollowWireStage stage = INITIAL_STAGE;
    _addNextTile(this, direction, state, stage);    // Add the first wire to the traversal. If it did not connect or was already checked then we return, otherwise continue traversal.
    if (wireNodes.empty()) {
        return;
    }
    wireNodes.pop();
    
    const bool* exitDirections = CONNECTION_INFO[_direction][_type][direction];    // Check for connection with all adjacent tiles (in the case that this is some arbitrary wire chosen and did not come from a source gate).
    if (exitDirections[0] && _position.y > 0) {
        _addNextTile(_boardPtr->getTile(Vector2u(_position.x, _position.y - 1)), NORTH, state, stage);
    }
    if (exitDirections[1] && _position.x < _boardPtr->getSize().x - 1) {
        _addNextTile(_boardPtr->getTile(Vector2u(_position.x + 1, _position.y)), EAST, state, stage);
    }
    if (exitDirections[2] && _position.y < _boardPtr->getSize().y - 1) {
        _addNextTile(_boardPtr->getTile(Vector2u(_position.x, _position.y + 1)), SOUTH, state, stage);
    }
    if (exitDirections[3] && _position.x > 0) {
        _addNextTile(_boardPtr->getTile(Vector2u(_position.x - 1, _position.y)), WEST, state, stage);
    }
    
    while (!wireNodes.empty()) {    // Perform depth-first traversal on remaining connected wires.
        TileWire* currentWire = wireNodes.top().first;
        Direction currentDirection = wireNodes.top().second;
        wireNodes.pop();
        //cout << "  currently at (" << currentWire->_position.x << ", " << currentWire->_position.y << ") going direction " << currentDirection << endl;
        
        const bool* exitDirections = CONNECTION_INFO[currentWire->_direction][currentWire->_type][currentDirection];    // Check for connection with adjacent tiles that do not point back to source wire.
        if (exitDirections[0] && currentDirection != SOUTH && currentWire->_position.y > 0) {
            _addNextTile(_boardPtr->getTile(Vector2u(currentWire->_position.x, currentWire->_position.y - 1)), NORTH, state, stage);
        }
        if (exitDirections[1] && currentDirection != WEST && currentWire->_position.x < _boardPtr->getSize().x - 1) {
            _addNextTile(_boardPtr->getTile(Vector2u(currentWire->_position.x + 1, currentWire->_position.y)), EAST, state, stage);
        }
        if (exitDirections[2] && currentDirection != NORTH && currentWire->_position.y < _boardPtr->getSize().y - 1) {
            _addNextTile(_boardPtr->getTile(Vector2u(currentWire->_position.x, currentWire->_position.y + 1)), SOUTH, state, stage);
        }
        if (exitDirections[3] && currentDirection != EAST && currentWire->_position.x > 0) {
            _addNextTile(_boardPtr->getTile(Vector2u(currentWire->_position.x - 1, currentWire->_position.y)), WEST, state, stage);
        }
    }
    traversedWires.clear();
}

void TileWire::redrawTile() const {
    _boardPtr->redrawTileVertices(1 + _type * 3 + _state1 - 1 + (_state2 - 1) * 3, _position, _direction, _highlight);
}

string TileWire::toString() const {
    if (_type == STRAIGHT) {
        return WIRE_SYMBOL_TABLE[_direction * 3 + _state1 - 1];
    } else if (_type < JUNCTION) {
        return WIRE_SYMBOL_TABLE[-6 + _type * 12 + _direction * 3 + _state1 - 1];
    } else if (_type == JUNCTION) {
        return WIRE_SYMBOL_TABLE[30 + _state1 - 1];
    } else {
        return WIRE_SYMBOL_TABLE[33 + _state1 - 1 + (_state2 - 1) * 3];
    }
}

void TileWire::fixUpdateTime() {
    _updateTimestamp1 = 0;
    _updateTimestamp2 = 0;
}

bool TileWire::alternativeTile() {
    if (_type == JUNCTION) {
        _type = CROSSOVER;
        _state2 = _state1;
        addUpdate();
        _boardPtr->changesMade = true;
    } else if (_type == CROSSOVER) {
        _type = JUNCTION;
        _state2 = LOW;
        addUpdate();
        _boardPtr->changesMade = true;
    }
    return false;
}

Tile* TileWire::clone(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates) {
    return new TileWire(boardPtr, position, noAdjacentUpdates, _direction, _type, _state1, _state2);
}

void TileWire::_addNextTile(Tile* nextTile, Direction direction, State& state, FollowWireStage& stage) const {
    if (typeid(*nextTile) == typeid(TileWire)) {    // Check if nextTile is a wire (most common case).
        TileWire* nextWire = static_cast<TileWire*>(nextTile);
        if (CONNECTION_INFO[nextWire->_direction][nextWire->_type][direction][(direction + 2) % 4]) {    // Check if we have a connection back to the source.
            bool wireUpdated = false;
            if (nextWire->_type == CROSSOVER && direction % 2 == 1) {    // Update state of the wire and timestamp if this wire has not been traversed yet.
                if (nextWire->_updateTimestamp2 != Tile::currentUpdateTime) {
                    nextWire->_state2 = state;
                    nextWire->_updateTimestamp2 = Tile::currentUpdateTime;
                    wireUpdated = true;
                }
            } else if (nextWire->_updateTimestamp1 != Tile::currentUpdateTime) {
                nextWire->_state1 = state;
                nextWire->_updateTimestamp1 = Tile::currentUpdateTime;
                wireUpdated = true;
            }
            
            if (wireUpdated) {    // If wire updated in last step, give it a cosmetic update and add it to traversal structures.
                nextWire->addUpdate(true);
                traversedWires.push_back(pair<TileWire*, Direction>(nextWire, direction));
                wireNodes.push(pair<TileWire*, Direction>(nextWire, direction));
                if (!_boardPtr->wireUpdates.empty() && (nextWire->_type != CROSSOVER || nextWire->_updateTimestamp1 == nextWire->_updateTimestamp2)) {    // Attempt to remove update if scheduled and wire has been fully updated.
                    _boardPtr->wireUpdates.erase(nextWire);
                }
            }
        }
    } else if (typeid(*nextTile) == typeid(TileGate)) {    // Else check if it is a gate.
        TileGate* nextGate = static_cast<TileGate*>(nextTile);
        State gateNextState = (nextGate->getDirection() + 2) % 4 == direction ? nextGate->getNextState() : DISCONNECTED;    // If the gate outputs into previous wire, there may be a state conflict.
        if (gateNextState != DISCONNECTED) {
            if (Board::enableExtraLogicStates) {
                _checkForInvalidState(nextTile, gateNextState, state, stage);
            } else if (gateNextState == HIGH && state == LOW) {    // If currently LOW and gate outputs HIGH in the next state, conflict found.
                state = HIGH;
                _fixTraversedWires(state);
            }
        } else {    // This gate is an endpoint, no need to check for conflict.
            Board::endpointGates.push_back(nextGate);
        }
    } else if (typeid(*nextTile) == typeid(TileLED)) {    // Else check if it is an LED.
        Board::endpointLEDs.push_back(static_cast<TileLED*>(nextTile));
    } else if (typeid(*nextTile) == typeid(TileSwitch) || typeid(*nextTile) == typeid(TileButton)) {    // Else check if switch/button.
        if (Board::enableExtraLogicStates) {
            _checkForInvalidState(nextTile, nextTile->getState(), state, stage);
        } else if (nextTile->getState() == HIGH && state == LOW) {    // If currently LOW and switch/button outputs HIGH, conflict found.
            state = HIGH;
            _fixTraversedWires(state);
        }
    }
}

void TileWire::_fixTraversedWires(State state) const {
    for (pair<TileWire*, Direction>& wire : traversedWires) {
        if (wire.first->_type == CROSSOVER && wire.second % 2 == 1) {
            wire.first->_state2 = state;
        } else {
            wire.first->_state1 = state;
        }
    }
}

void TileWire::_checkForInvalidState(Tile* target, State targetState, State& state, FollowWireStage& stage) const {
    if (targetState == MIDDLE) {
        return;
    }
    if (stage == INITIAL_STAGE || state == MIDDLE) {
        stage = INPUT_FOUND;
        state = targetState;
        _fixTraversedWires(state);
    } else if (state != targetState) {
        cout << "Uh oh thats an error! At (" << target->getPosition().x << ", " << target->getPosition().y << ")" << endl;
        if (stage != INVALID_STAGE) {
            stage = INVALID_STAGE;
            ++Board::numStateErrors;
            state = MIDDLE;
            _fixTraversedWires(state);
        }
    }
}