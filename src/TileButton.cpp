#include "Board.h"
#include "TileButton.h"

vector<TileButton*> TileButton::_transitioningButtons;

void TileButton::updateTransitioningButtons() {
    for (TileButton* button : _transitioningButtons) {
        button->_state = LOW;
        button->addUpdate();
    }
    _transitioningButtons.clear();
}

TileButton::TileButton(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates, char charID, State state) : Tile(boardPtr, position, true, true) {
    _charID = charID;
    _state = state;
    boardPtr->buttonKeybinds[charID].push_back(this);
    boardPtr->tileLabels[this] = Text("", Board::getFont(), 25);
    boardPtr->tileLabels[this].setString(string(1, charID));
    boardPtr->tileLabels[this].setPosition(_position.x * Board::getTileSize().x + 9.0f, _position.y * Board::getTileSize().y - 2.0f);
    addUpdate(false, noAdjacentUpdates);
}

TileButton::~TileButton() {
    _boardPtr->buttonUpdates.erase(this);
    auto mapIter = _boardPtr->buttonKeybinds.find(_charID);
    for (auto vectorIter = mapIter->second.begin(); vectorIter != mapIter->second.end(); ++vectorIter) {
        if (*vectorIter == this) {
            mapIter->second.erase(vectorIter);
            break;
        }
    }
    _boardPtr->tileLabels.erase(this);
}

State TileButton::getState() const {
    return _state;
}

void TileButton::setPosition(const Vector2u& position, bool noAdjacentUpdates, bool keepOverwrittenTile) {
    Tile::setPosition(position, noAdjacentUpdates, keepOverwrittenTile);
    _boardPtr->tileLabels[this].setPosition(_position.x * Board::getTileSize().x + 9.0f, _position.y * Board::getTileSize().y - 2.0f);
}

void TileButton::setCharID(char charID) {
    auto mapIter = _boardPtr->buttonKeybinds.find(_charID);
    for (auto vectorIter = mapIter->second.begin(); vectorIter != mapIter->second.end(); ++vectorIter) {
        if (*vectorIter == this) {
            mapIter->second.erase(vectorIter);
            break;
        }
    }
    _charID = charID;
    _boardPtr->buttonKeybinds[_charID].push_back(this);
    _boardPtr->tileLabels[this].setString(string(1, charID));
    addUpdate(true);
    _boardPtr->changesMade = true;
}

void TileButton::setState(State state) {
    _state = state;
    addUpdate();
}

State TileButton::checkOutput(Direction direction) const {
    return _state;
}

void TileButton::addUpdate(bool isCosmetic, bool noAdjacentUpdates) {
    _boardPtr->cosmeticUpdates.insert(this);
    if (!isCosmetic) {
        _boardPtr->buttonUpdates.insert(this);
        if (!noAdjacentUpdates) {
            _updateAdjacentTiles();
        }
    }
}

void TileButton::updateOutput() {
    _boardPtr->buttonUpdates.erase(this);
    if (_state == HIGH) {
        _transitioningButtons.push_back(this);
    }
    
    if (_position.y > 0) {    // Follow wire on all adjacent sides.
        _boardPtr->getTile(Vector2u(_position.x, _position.y - 1))->followWire(NORTH, _state);
    }
    if (_position.x < _boardPtr->getSize().x - 1) {
        _boardPtr->getTile(Vector2u(_position.x + 1, _position.y))->followWire(EAST, _state);
    }
    if (_position.y < _boardPtr->getSize().y - 1) {
        _boardPtr->getTile(Vector2u(_position.x, _position.y + 1))->followWire(SOUTH, _state);
    }
    if (_position.x > 0) {
        _boardPtr->getTile(Vector2u(_position.x - 1, _position.y))->followWire(WEST, _state);
    }
}

void TileButton::redrawTile() const {
    _boardPtr->redrawTileVertices(24 + _state - 1, _position, _direction, _highlight);
}

string TileButton::toString() const {
    string s(INPUT_SYMBOL_TABLE[2 + _state - 1]);
    s.push_back(_charID);
    return s;
}

bool TileButton::alternativeTile() {
    return true;
}

Tile* TileButton::clone(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates) {
    return new TileButton(boardPtr, position, noAdjacentUpdates, _charID, _state);
}