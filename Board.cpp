#include "Board.h"
#include "Tile.h"
#include <stdexcept>

using namespace std;
using namespace sf;

Board::Board() {
    gridActive = true;
    _tileArray = nullptr;
}

Board::~Board() {
    for (unsigned int y = 0; y < _boardSize.y; ++y) {
        for (unsigned int x = 0; x < _boardSize.x; ++x) {
            delete _tileArray[y][x];
        }
        delete[] _tileArray[y];
    }
    delete[] _tileArray;
}

Tile*** Board::getTileArray() const {
    return _tileArray;
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
    //Tile*** newTileArray = new Tile**[];
    
    
    for (unsigned int y = 0; y < _boardSize.y; ++y) {
        for (unsigned int x = 0; x < _boardSize.x; ++x) {
            delete _tileArray[y][x];
        }
        delete[] _tileArray[y];
    }
    delete[] _tileArray;
    
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