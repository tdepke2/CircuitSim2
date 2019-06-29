#include "Board.h"
#include "TileSwitch.h"

TileSwitch::TileSwitch(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates, char charID, State state) : Tile(boardPtr, position, true, true) {
    _charID = charID;
    _state = state;
    boardPtr->switchKeybinds[charID].push_back(this);
    boardPtr->tileLabels[this] = Text("", Board::getFont(), 25);
    boardPtr->tileLabels[this].setString(string(1, charID));
    boardPtr->tileLabels[this].setPosition(getPosition().x * Board::getTileSize().x + 9.0f, getPosition().y * Board::getTileSize().y - 2.0f);
    addUpdate(false, noAdjacentUpdates);
}

TileSwitch::~TileSwitch() {
    getBoardPtr()->switchUpdates.erase(this);
    auto mapIter = getBoardPtr()->switchKeybinds.find(_charID);
    for (auto vectorIter = mapIter->second.begin(); vectorIter != mapIter->second.end(); ++vectorIter) {
        if (*vectorIter == this) {
            mapIter->second.erase(vectorIter);
            break;
        }
    }
    getBoardPtr()->tileLabels.erase(this);
}

State TileSwitch::getState() const {
    return _state;
}

void TileSwitch::setPosition(const Vector2u& position, bool noAdjacentUpdates, bool keepOverwrittenTile) {
    Tile::setPosition(position, noAdjacentUpdates, keepOverwrittenTile);
    getBoardPtr()->tileLabels[this].setPosition(getPosition().x * Board::getTileSize().x + 9.0f, getPosition().y * Board::getTileSize().y - 2.0f);
}

void TileSwitch::setCharID(char charID) {
    auto mapIter = getBoardPtr()->switchKeybinds.find(_charID);
    for (auto vectorIter = mapIter->second.begin(); vectorIter != mapIter->second.end(); ++vectorIter) {
        if (*vectorIter == this) {
            mapIter->second.erase(vectorIter);
            break;
        }
    }
    _charID = charID;
    getBoardPtr()->switchKeybinds[charID].push_back(this);
    getBoardPtr()->tileLabels[this].setString(string(1, charID));
    addUpdate(true);
    getBoardPtr()->changesMade = true;
}

void TileSwitch::setState(State state) {
    _state = state;
    addUpdate();
}

State TileSwitch::checkOutput(Direction direction) const {
    return _state;
}

void TileSwitch::addUpdate(bool isCosmetic, bool noAdjacentUpdates) {
    getBoardPtr()->cosmeticUpdates.insert(this);
    if (!isCosmetic) {
        getBoardPtr()->switchUpdates.insert(this);
        if (!noAdjacentUpdates) {
            _updateAdjacentTiles();
        }
    }
}

void TileSwitch::updateOutput() {
    getBoardPtr()->switchUpdates.erase(this);
    
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

void TileSwitch::redrawTile() const {
    getBoardPtr()->redrawTileVertices(22 + _state - 1, getPosition(), _direction, getHighlight());
}

string TileSwitch::toString() const {
    string s(INPUT_SYMBOL_TABLE[_state - 1]);
    s.push_back(_charID);
    return s;
}

bool TileSwitch::alternativeTile() {
    return true;
}

Tile* TileSwitch::clone(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates) {
    return new TileSwitch(boardPtr, position, noAdjacentUpdates, _charID, _state);
}