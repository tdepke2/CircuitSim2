#include "Board.h"
#include "Tile.h"

using namespace std;
//using namespace sf;

Tile::Tile(const Vector2u& position, const Board& board) {
    _position = position;
    board.getTileArray()[position.y][position.x];
}

Tile::~Tile() {}

int Tile::getID() const {
    return 0;
}

const Vector2u& Tile::getPosition() const {
    return _position;
}

void Tile::setPosition(const Vector2u& position, const Board& board) {
    if (board.getTileArray()[position.y][position.x] != this) {
        delete board.getTileArray()[position.y][position.x];
        board.getTileArray()[position.y][position.x] = board.getTileArray()[_position.y][_position.x];
        board.getTileArray()[_position.y][_position.x] = new Tile(_position, board);
        _position = position;
    }
}