#include "Board.h"
#include "Tile.h"

Tile::Tile() {}

Tile::Tile(const Vector2u& position, Board& board) {
    _position = position;
    board.redrawTile(this);
}

Tile::~Tile() {}

int Tile::getTextureID() const {
    return 0;
}

const Vector2u& Tile::getPosition() const {
    return _position;
}

void Tile::setPosition(const Vector2u& position, Board& board) {
    if (board.getTileArray()[position.y][position.x] != this) {
        delete board.getTileArray()[position.y][position.x];
        board.getTileArray()[position.y][position.x] = board.getTileArray()[_position.y][_position.x];
        board.getTileArray()[_position.y][_position.x] = new Tile(_position, board);
        _position = position;
        board.redrawTile(this);
    }
}