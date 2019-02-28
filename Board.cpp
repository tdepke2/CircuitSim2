#include "Board.h"
#include <stdexcept>

using namespace std;
using namespace sf;

Board::Board() {
    gridActive = false;
}

void Board::loadTextures(const string& filenameNoGrid, const string& filenameGrid, const Vector2u& tileSize) {
    if (!_tilesetNoGrid.loadFromFile(filenameNoGrid)) {
        throw runtime_error("\"" + filenameNoGrid + "\": Unable to load texture file.");
    }
    if (!_tilesetGrid.loadFromFile(filenameGrid)) {
        throw runtime_error("\"" + filenameGrid + "\": Unable to load texture file.");
    }
}

void Board::resizeBoard(const Vector2u& boardSize) {
    _boardSize = boardSize;
    _vertices.resize(boardSize.x * boardSize.y * 4);
}

void Board::draw (RenderTarget& target, RenderStates states) const {
    states.transform *= getTransform();
    if (gridActive) {
        states.texture = &_tilesetGrid;
    } else {
        states.texture = &_tilesetNoGrid;
    }
    target.draw(_vertices, states);
}