#include "Board.h"
#include "Tile.h"
#include "TileButton.h"
#include "TileGate.h"
#include "TileLED.h"
#include "TileSwitch.h"
#include "TileWire.h"
#include <fstream>
#include <iostream>
#include <stdexcept>

const string Board::SYMBOL_INFO_TABLE[97] = {    // Should split this up into multiple tables ##################################################################################################
    "  ",
    "| ",  "[ ",  "--",  "==",
    "\'-", "\"=", ",-",  ";=",  ", ",  "; ",  "\' ", "\" ",
    ">-",  ">=",  "v-",  "v=",  "< ",  "<.",  "^-",  "^=",
    "+-",  "#=",
    "|-",  "[-",  "|=",  "[=",
    "s",   "S",
    "t",   "T",
    "..",  "##",
    "^d",  "^D",  ">d",  ">D",  "vd",  "vD",  "<d",  "<D",
    "^n",  "^N",  ">n",  ">N",  "vn",  "vN",  "<n",  "<N",
    "^a",  "^A",  ">a",  ">A",  "va",  "vA",  "<a",  "<A",
    "^o",  "^O",  ">o",  ">O",  "vo",  "vO",  "<o",  "<O",
    "^x",  "^X",  ">x",  ">X",  "vx",  "vX",  "<x",  "<X",
    "^b",  "^B",  ">b",  ">B",  "vb",  "vB",  "<b",  "<B",
    "^p",  "^P",  ">p",  ">P",  "vp",  "vP",  "<p",  "<P",
    "^y",  "^Y",  ">y",  ">Y",  "vy",  "vY",  "<y",  "<Y"
};

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

void Board::loadFile(const string& filename) {
    ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        throw runtime_error("\"" + filename + "\": Unable to open file for reading.");
    }
    
    for (unsigned int y = 0; y < _size.y; ++y) {    // Delete _tileArray data completely.
        for (unsigned int x = 0; x < _size.x; ++x) {
            delete _tileArray[y][x];
        }
        delete[] _tileArray[y];
    }
    delete[] _tileArray;
    _size = Vector2u(0, 0);
    _tileArray = nullptr;
    
    string line;
    int lineNumber = 0, numEntries = 0;
    unsigned int posX = 0, posY = 0;
    try {
        while (getline(inputFile, line)) {
            ++lineNumber;
            if (line.length() == 0) {
                continue;
            } else if (numEntries == 3) {
                if (line.length() != _size.x * 2 + 2) {
                    throw runtime_error("Length of this line is incorrect.");
                }
                posX = 0;
                while (posX < _size.x) {
                    int i = 0;
                    while (i < 97) {
                        if (line[posX * 2 + 1] == SYMBOL_INFO_TABLE[i][0] && (i >= 27 && i <= 30 || line[posX * 2 + 2] == SYMBOL_INFO_TABLE[i][1])) {
                            break;
                        }
                        ++i;
                    }
                    
                    if (i == 0) {
                        _tileArray[posY][posX] = new Tile(Vector2u(posX, posY), *this);
                    } else if (i < 23) {
                        _tileArray[posY][posX] = new TileWire(Vector2u(posX, posY), *this, (i - 1) / 2, i % 2 == 0, false);
                    } else if (i < 27) {
                        _tileArray[posY][posX] = new TileWire(Vector2u(posX, posY), *this, 11, i % 2 == 0, i >= 25);
                    } else if (i < 29) {
                        _tileArray[posY][posX] = new TileSwitch(Vector2u(posX, posY), *this, line[posX * 2 + 2], i % 2 == 0);
                    } else if (i < 31) {
                        _tileArray[posY][posX] = new TileButton(Vector2u(posX, posY), *this, line[posX * 2 + 2], i % 2 == 0);
                    } else if (i < 33) {
                        _tileArray[posY][posX] = new TileLED(Vector2u(posX, posY), *this, i % 2 == 0);
                    } else if (i < 97) {
                        _tileArray[posY][posX] = new TileGate(Vector2u(posX, posY), *this, static_cast<LogicGate>((i - 33) / 8), static_cast<Direction>((i - 33) / 2 % 4), i % 2 == 0);
                    } else {
                        throw runtime_error("Invalid symbols \"" + line.substr(posX * 2 + 1, 2) + "\" at position (" + to_string(posX) + ", " + to_string(posY) + ").");
                    }
                    ++posX;
                }
                ++posY;
                if (posY == _size.y) {
                    ++numEntries;
                }
            } else if (numEntries == 0) {
                _size.x = stoul(line);
                ++numEntries;
            } else if (numEntries == 1) {
                _size.y = stoul(line);
                _vertices.resize(_size.x * _size.y * 4);
                _tileArray = new Tile**[_size.y];
                for (unsigned int y = 0; y < _size.y; ++y) {
                    _tileArray[y] = new Tile*[_size.x];
                }
                ++numEntries;
            } else if (numEntries == 2 || numEntries == 4) {
                if (line.length() != _size.x * 2 + 2) {
                    throw runtime_error("Length of this line is incorrect.");
                }
                ++numEntries;
            } else {
                throw runtime_error("Invalid save file data.");
            }
        }
        if (numEntries != 5) {
            throw runtime_error("Missing data, end of file reached.");
        }
    } catch (exception& ex) {    // File error happened, need to clean up partially loaded board and make a new one.
        for (unsigned int y = 0; y < _size.y; ++y) {
            if (y < posY) {
                for (unsigned int x = 0; x < _size.x; ++x) {
                    delete _tileArray[y][x];
                }
            } else if (y == posY) {
                for (unsigned int x = 0; x < posX; ++x) {
                    delete _tileArray[y][x];
                }
            }
            delete[] _tileArray[y];
        }
        delete[] _tileArray;
        _vertices.resize(0);
        _size = Vector2u(0, 0);
        _tileArray = nullptr;
        
        // Make a new board ################################################################################################################################
        inputFile.close();
        throw runtime_error(filename + " at line " + to_string(lineNumber) + ": " + ex.what());
    }
    inputFile.close();
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