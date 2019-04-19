#include "Board.h"
#include "TileLED.h"

#include <iostream>

TileLED::TileLED(Board* boardPtr, const Vector2u& position, State state) : Tile(boardPtr, position, true) {
    _state = state;
    addUpdate();
    
    cout << "New LED:\n  North ";
    if (_position.y > 0) {
        cout << _boardPtr->getTileArray()[_position.y - 1][_position.x]->checkOutput(NORTH);
    }
    cout << "\n  East ";
    if (_position.x < _boardPtr->getSize().x - 1) {
        cout << _boardPtr->getTileArray()[_position.y][_position.x + 1]->checkOutput(EAST);
    }
    cout << "\n  South ";
    if (_position.y < _boardPtr->getSize().y - 1) {
        cout << _boardPtr->getTileArray()[_position.y + 1][_position.x]->checkOutput(SOUTH);
    }
    cout << "\n  West ";
    if (_position.x > 0) {
        cout << _boardPtr->getTileArray()[_position.y][_position.x - 1]->checkOutput(WEST);
    }
    cout << endl;
}

TileLED::~TileLED() {
    _boardPtr->LEDUpdates.erase(this);
}

int TileLED::getTextureID() const {
    return 17 + (_state == HIGH);
}

State TileLED::checkOutput(Direction direction) const {
    return _state;
}

void TileLED::addUpdate(bool isCosmetic) {
    if (isCosmetic) {
        _boardPtr->cosmeticUpdates.insert(this);
    } else {
        _boardPtr->LEDUpdates.insert(this);
    }
}

Tile* TileLED::clone(Board* boardPtr, const Vector2u& position) {
    return new TileLED(boardPtr, position, _state);
}