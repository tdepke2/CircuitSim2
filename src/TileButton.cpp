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
    boardPtr->tileLabels[this].setPosition(getPosition().x * Board::getTileSize().x + 9.0f, getPosition().y * Board::getTileSize().y - 2.0f);
    addUpdate(false, noAdjacentUpdates);
}

TileButton::~TileButton() {
    getBoardPtr()->buttonUpdates.erase(this);
    auto mapIter = getBoardPtr()->buttonKeybinds.find(_charID);
    for (auto vectorIter = mapIter->second.begin(); vectorIter != mapIter->second.end(); ++vectorIter) {
        if (*vectorIter == this) {
            mapIter->second.erase(vectorIter);
            break;
        }
    }
    getBoardPtr()->tileLabels.erase(this);
}

State TileButton::getState() const {
    return _state;
}

void TileButton::setPosition(const Vector2u& position, bool noAdjacentUpdates, bool keepOverwrittenTile) {
    Tile::setPosition(position, noAdjacentUpdates, keepOverwrittenTile);
    getBoardPtr()->tileLabels[this].setPosition(getPosition().x * Board::getTileSize().x + 9.0f, getPosition().y * Board::getTileSize().y - 2.0f);
}

void TileButton::setCharID(char charID) {
    auto mapIter = getBoardPtr()->buttonKeybinds.find(_charID);
    for (auto vectorIter = mapIter->second.begin(); vectorIter != mapIter->second.end(); ++vectorIter) {
        if (*vectorIter == this) {
            mapIter->second.erase(vectorIter);
            break;
        }
    }
    _charID = charID;
    getBoardPtr()->buttonKeybinds[_charID].push_back(this);
    getBoardPtr()->tileLabels[this].setString(string(1, charID));
    addUpdate(true);
    getBoardPtr()->changesMade = true;
}

void TileButton::setState(State state) {
    _state = state;
    addUpdate();
}

State TileButton::checkOutput(Direction direction) const {
    return _state;
}

void TileButton::addUpdate(bool isCosmetic, bool noAdjacentUpdates) {
    getBoardPtr()->cosmeticUpdates.insert(this);
    if (!isCosmetic) {
        getBoardPtr()->buttonUpdates.insert(this);
        if (!noAdjacentUpdates) {
            _updateAdjacentTiles();
        }
    }
}

void TileButton::updateOutput() {
    getBoardPtr()->buttonUpdates.erase(this);
    if (_state == HIGH) {
        _transitioningButtons.push_back(this);
    }
    
    if (getPosition().y > 0) {    // Follow wire on all adjacent sides.
        getBoardPtr()->getTile(Vector2u(getPosition().x, getPosition().y - 1))->followWire(NORTH, _state);
    }
    if (getPosition().x < getBoardPtr()->getSize().x - 1) {
        getBoardPtr()->getTile(Vector2u(getPosition().x + 1, getPosition().y))->followWire(EAST, _state);
    }
    if (getPosition().y < getBoardPtr()->getSize().y - 1) {
        getBoardPtr()->getTile(Vector2u(getPosition().x, getPosition().y + 1))->followWire(SOUTH, _state);
    }
    if (getPosition().x > 0) {
        getBoardPtr()->getTile(Vector2u(getPosition().x - 1, getPosition().y))->followWire(WEST, _state);
    }
}

void TileButton::redrawTile() const {
    getBoardPtr()->redrawTileVertices(24 + _state - 1, getPosition(), _direction, getHighlight());
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