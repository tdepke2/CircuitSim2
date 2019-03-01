#include "Board.h"
#include "Tile.h"
#include <iostream>

Tile::Tile(const Vector2u& position, Board& board) {
    _position = position;
    board.redrawTile(this);
    cout << "Tile (" << _position.x << ", " << _position.y << ") created." << endl;
}

Tile::~Tile() {
    cout << "Tile (" << _position.x << ", " << _position.y << ") destroyed." << endl;
}

int Tile::getNumericID() const {
    return 1;
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