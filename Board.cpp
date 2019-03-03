#include "Board.h"
#include "Tile.h"
#include <iostream>
#include <stdexcept>

Board::Board() {
    gridActive = true;
    _vertices.setPrimitiveType(Quads);
    _size = Vector2u(0, 0);
    _tileArray = nullptr;
}

Board::~Board() {
    for (unsigned int y = 0; y < _size.y; ++y) {
        for (unsigned int x = 0; x < _size.x; ++x) {
            delete _tileArray[y][x];
        }
        delete[] _tileArray[y];
    }
    delete[] _tileArray;
}

const Vector2u& Board::getSize() const {
    return _size;
}

const Vector2u& Board::getTileSize() const {
    return _tileSize;
}

Tile*** Board::getTileArray() const {
    return _tileArray;
}

void Board::loadTextures(const string& filenameGrid, const string& filenameNoGrid, const Vector2u& tileSize) {
    if (!_tilesetGrid.loadFromFile(filenameGrid)) {
        throw runtime_error("\"" + filenameGrid + "\": Unable to load texture file.");
    }
    _tilesetGrid.setSmooth(true);
    if (!_tilesetGrid.generateMipmap()) {
        cout << "Warn: \"" << filenameGrid << "\": Unable to generate mipmap for texture." << endl;
    }
    
    if (!_tilesetNoGrid.loadFromFile(filenameNoGrid)) {
        throw runtime_error("\"" + filenameNoGrid + "\": Unable to load texture file.");
    }
    _tilesetNoGrid.setSmooth(true);
    if (!_tilesetNoGrid.generateMipmap()) {
        cout << "Warn: \"" << filenameNoGrid << "\": Unable to generate mipmap for texture." << endl;
    }
    
    _tileSize = tileSize;
}

void Board::resize(const Vector2u& size) {
    _vertices.resize(size.x * size.y * 4);
    Vector2u oldSize = _size;
    _size = size;
    
    Tile*** newTileArray = new Tile**[size.y];    // Create new array for the new size.
    unsigned int xStop = min(size.x, oldSize.x), yStop = min(size.y, oldSize.y);
    for (unsigned int y = 0; y < yStop; ++y) {    // Allocate new array, copy over tiles from old array, and delete old tiles that are not used.
        newTileArray[y] = new Tile*[size.x];
        for (unsigned int x = 0; x < xStop; ++x) {
            newTileArray[y][x] = _tileArray[y][x];
        }
        for (unsigned int x = xStop; x < size.x; ++x) {
            newTileArray[y][x] = new Tile(Vector2u(x, y), *this);
        }
        for (unsigned int x = xStop; x < oldSize.x; ++x) {
            delete _tileArray[y][x];
        }
        delete[] _tileArray[y];
    }
    for (unsigned int y = yStop; y < size.y; ++y) {    // Add extra rows if necessary.
        newTileArray[y] = new Tile*[size.x];
        for (unsigned int x = 0; x < size.x; ++x) {
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
    Vertex* tileVertices = &_vertices[(tile->getPosition().y * _size.x + tile->getPosition().x) * 4];
    
    float windowX = static_cast<float>(tile->getPosition().x * _tileSize.x);
    float windowY = static_cast<float>(tile->getPosition().y * _tileSize.y);
    tileVertices[0].position = Vector2f(windowX, windowY);
    tileVertices[1].position = Vector2f(windowX + _tileSize.x, windowY);
    tileVertices[2].position = Vector2f(windowX + _tileSize.x, windowY + _tileSize.y);
    tileVertices[3].position = Vector2f(windowX, windowY + _tileSize.y);
    
    float tileX = static_cast<float>((tile->getTextureID() % (_tilesetGrid.getSize().x / _tileSize.x)) * _tileSize.x);
    float tileY = static_cast<float>((tile->getTextureID() / (_tilesetGrid.getSize().x / _tileSize.x)) * _tileSize.y);
    tileVertices[0].texCoords = Vector2f(tileX, tileY);
    tileVertices[1].texCoords = Vector2f(tileX + _tileSize.x, tileY);
    tileVertices[2].texCoords = Vector2f(tileX + _tileSize.x, tileY + _tileSize.y);
    tileVertices[3].texCoords = Vector2f(tileX, tileY + _tileSize.y);
}

void Board::replaceTile(Tile* tile) {
    delete _tileArray[tile->getPosition().y][tile->getPosition().x];
    _tileArray[tile->getPosition().y][tile->getPosition().x] = tile;
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