#include "Board.h"
#include "Tile.h"

Tile::Tile() {}

Tile::Tile(Board* boardPtr, const Vector2u& position, bool suppressUpdate) {
    _boardPtr = boardPtr;
    _position = position;
    _direction = NORTH;
    _highlight = false;
    if (!suppressUpdate) {
        _boardPtr->addUpdate(this, true);
    }
}

Tile::~Tile() {
    _boardPtr->removeUpdate(this);
}

int Tile::getTextureID() const {
    return 0;
}

const Vector2u& Tile::getPosition() const {
    return _position;
}

Direction Tile::getDirection() const {
    return _direction;
}

bool Tile::getHighlight() const {
    return _highlight;
}

void Tile::setPosition(const Vector2u& position, Board& board, bool keepOverwrittenTile) {
    if (!keepOverwrittenTile) {
        if (board.getTileArray()[position.y][position.x] != this) {
            delete board.getTileArray()[position.y][position.x];
            board.getTileArray()[position.y][position.x] = this;
            board.getTileArray()[_position.y][_position.x] = new Tile(_boardPtr, _position);
            _position = position;
            _boardPtr->addUpdate(this, true);
        }
    } else {
        board.getTileArray()[position.y][position.x] = this;
        _position = position;
        _boardPtr->addUpdate(this, true);
    }
}

void Tile::setHighlight(bool highlight) {
    if (_highlight != highlight) {
        _highlight = highlight;
        _boardPtr->addUpdate(this, false);
    }
}

void Tile::setDirection(Direction direction, Board& board) {}

void Tile::flip(bool acrossHorizontal, Board& board) {}

Tile* Tile::clone(Board* boardPtr, const Vector2u& position) {
    return new Tile(boardPtr, position);
}