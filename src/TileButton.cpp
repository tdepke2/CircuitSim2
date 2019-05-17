#include "Board.h"
#include "TileButton.h"

TileButton::TileButton(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates, char charID, State state) : Tile(boardPtr, position, true, true) {
    _charID = charID;
    _state = state;
    boardPtr->buttonKeybinds[charID].push_back(this);
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
}

int TileButton::getTextureID() const {
    return 15 + (_state == HIGH);
}

State TileButton::getState() const {
    return _state;
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
    addUpdate(true);
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

Tile* TileButton::clone(Board* boardPtr, const Vector2u& position, bool noAdjacentUpdates) {
    return new TileButton(boardPtr, position, noAdjacentUpdates, _charID, _state);
}