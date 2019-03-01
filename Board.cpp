#include "Board.h"
#include "Tile.h"
#include <stdexcept>

Board::Board() {
    gridActive = true;
    _vertices.setPrimitiveType(Quads);
    _boardSize = Vector2u(0, 0);
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

void Board::loadTextures(const string& filenameGrid, const string& filenameNoGrid, const Vector2u& tileSize) {
    if (!_tilesetGrid.loadFromFile(filenameGrid)) {
        throw runtime_error("\"" + filenameGrid + "\": Unable to load texture file.");
    }
    if (!_tilesetNoGrid.loadFromFile(filenameNoGrid)) {
        throw runtime_error("\"" + filenameNoGrid + "\": Unable to load texture file.");
    }
    _tileSize = tileSize;
}

void Board::resize(const Vector2u& boardSize) {
    _vertices.resize(boardSize.x * boardSize.y * 4);
    Vector2u oldSize = _boardSize;
    _boardSize = boardSize;
    
    Tile*** newTileArray = new Tile**[boardSize.y];    // Create new array for the new size.
    unsigned int xStop = min(boardSize.x, oldSize.x), yStop = min(boardSize.y, oldSize.y);
    for (unsigned int y = 0; y < yStop; ++y) {    // Allocate new array, copy over tiles from old array, and delete old tiles that are not used.
        newTileArray[y] = new Tile*[boardSize.x];
        for (unsigned int x = 0; x < xStop; ++x) {
            newTileArray[y][x] = _tileArray[y][x];
        }
        for (unsigned int x = xStop; x < boardSize.x; ++x) {
            newTileArray[y][x] = new Tile(Vector2u(x, y), *this);
        }
        for (unsigned int x = xStop; x < oldSize.x; ++x) {
            delete _tileArray[y][x];
        }
        delete[] _tileArray[y];
    }
    for (unsigned int y = yStop; y < boardSize.y; ++y) {    // Add extra rows if necessary.
        newTileArray[y] = new Tile*[boardSize.x];
        for (unsigned int x = 0; x < boardSize.x; ++x) {
            newTileArray[y][x] = new Tile(Vector2u(x, y), *this);
        }
    }
    for (unsigned int y = yStop; y < oldSize.y; ++y) {    // Delete extra rows if necessary.
        for (unsigned int x = 0; x < oldSize.x; ++x) {
            delete _tileArray[y][x];
        }
        delete[] _tileArray[y];
    }
    delete[] _tileArray;
    
    _tileArray = newTileArray;
}

void Board::redrawTile(Tile* tile) {
    Vertex* tileVertices = &_vertices[(tile->getPosition().y * _boardSize.x + tile->getPosition().x) * 4];
    
    float windowX = static_cast<float>(tile->getPosition().x * _tileSize.x);
    float windowY = static_cast<float>(tile->getPosition().y * _tileSize.y);
    tileVertices[0].position = Vector2f(windowX, windowY);
    tileVertices[1].position = Vector2f(windowX + _tileSize.x, windowY);
    tileVertices[2].position = Vector2f(windowX + _tileSize.x, windowY + _tileSize.y);
    tileVertices[3].position = Vector2f(windowX, windowY + _tileSize.y);
    
    float tileX = static_cast<float>((tile->getNumericID() % (_tilesetGrid.getSize().x / _tileSize.x)) * _tileSize.x);
    float tileY = static_cast<float>((tile->getNumericID() / (_tilesetGrid.getSize().x / _tileSize.x)) * _tileSize.y);
    tileVertices[0].texCoords = Vector2f(tileX, tileY);
    tileVertices[1].texCoords = Vector2f(tileX + _tileSize.x, tileY);
    tileVertices[2].texCoords = Vector2f(tileX + _tileSize.x, tileY + _tileSize.y);
    tileVertices[3].texCoords = Vector2f(tileX, tileY + _tileSize.y);
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