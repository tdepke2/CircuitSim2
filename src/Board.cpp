#include "Board.h"
#include "Tile.h"
#include "TileButton.h"
#include "TileGate.h"
#include "TileLED.h"
#include "TileSwitch.h"
#include "TileWire.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <typeinfo>

bool Board::gridActive = true;
Texture* Board::_tilesetGridPtr = nullptr;
Texture* Board::_tilesetNoGridPtr = nullptr;
int Board::_textureIDMax;
Vector2u Board::_tileSize;

const vector<string> Board::WIRE_SYMBOL_TABLE = {
    "| ",  "[ ",  "--",  "==",
    "\'-", "\"=", ",-",  ";=",  ", ",  "; ",  "\' ", "\" ",
    ">-",  ">=",  "v-",  "v=",  "< ",  "<.",  "^-",  "^=",
    "+-",  "#=",
    "|-",  "[-",  "|=",  "[="
};
const vector<string> Board::INPUT_SYMBOL_TABLE = {
    "s",   "S",
    "t",   "T"
};
const vector<string> Board::OUTPUT_SYMBOL_TABLE = {
    "..",  "##"
};
const vector<string> Board::GATE_SYMBOL_TABLE = {
    "^d",  "^D",  ">d",  ">D",  "vd",  "vD",  "<d",  "<D",
    "^m",  "^M",  ">m",  ">M",  "vm",  "vM",  "<m",  "<M",
    "^n",  "^N",  ">n",  ">N",  "vn",  "vN",  "<n",  "<N",
    "^a",  "^A",  ">a",  ">A",  "va",  "vA",  "<a",  "<A",
    "^b",  "^B",  ">b",  ">B",  "vb",  "vB",  "<b",  "<B",
    "^o",  "^O",  ">o",  ">O",  "vo",  "vO",  "<o",  "<O",
    "^p",  "^P",  ">p",  ">P",  "vp",  "vP",  "<p",  "<P",
    "^x",  "^X",  ">x",  ">X",  "vx",  "vX",  "<x",  "<X",
    "^y",  "^Y",  ">y",  ">Y",  "vy",  "vY",  "<y",  "<Y"
};

void Board::loadTextures(const string& filenameGrid, const string& filenameNoGrid, const Vector2u& tileSize) {
    cout << "Clamping and stitching textures." << endl;
    delete _tilesetGridPtr;
    delete _tilesetNoGridPtr;
    _tilesetGridPtr = new Texture;
    _tilesetNoGridPtr = new Texture;
    Image tilesetImage;
    if (!tilesetImage.loadFromFile(filenameGrid)) {
        throw runtime_error("\"" + filenameGrid + "\": Unable to load texture file.");
    }
    _buildTexture(tilesetImage, _tilesetGridPtr, tileSize);
    _tilesetGridPtr->setSmooth(true);
    if (!_tilesetGridPtr->generateMipmap()) {
        cout << "Warn: \"" << filenameGrid << "\": Unable to generate mipmap for texture." << endl;
    }
    
    if (!tilesetImage.loadFromFile(filenameNoGrid)) {
        throw runtime_error("\"" + filenameNoGrid + "\": Unable to load texture file.");
    }
    _buildTexture(tilesetImage, _tilesetNoGridPtr, tileSize);
    _tilesetNoGridPtr->setSmooth(true);
    if (!_tilesetNoGridPtr->generateMipmap()) {
        cout << "Warn: \"" << filenameNoGrid << "\": Unable to generate mipmap for texture." << endl;
    }
    
    _textureIDMax = tilesetImage.getSize().x / tileSize.x * tilesetImage.getSize().y / tileSize.y;
    _tileSize = tileSize;
}

const Vector2u& Board::getTileSize() {
    return _tileSize;
}

Board::Board() {
    _vertices.setPrimitiveType(Quads);
    _size = Vector2u(0, 0);
    _tileArray = nullptr;
}

Board::~Board() {
    clear();
}

const Vector2u& Board::getSize() const {
    return _size;
}

Tile* Board::getTile(const Vector2i& position) const {
    return _tileArray[position.y][position.x];
}

Tile* Board::getTile(const Vector2u& position) const {
    return _tileArray[position.y][position.x];
}

void Board::setTile(const Vector2i& position, Tile* tile) {
    _tileArray[position.y][position.x] = tile;
}

void Board::setTile(const Vector2u& position, Tile* tile) {
    _tileArray[position.y][position.x] = tile;
}

void Board::updateCosmetics() {
    for (auto setIter = cosmeticUpdates.begin(); setIter != cosmeticUpdates.end(); ++setIter) {
        _redrawTile(*setIter);
    }
    cosmeticUpdates.clear();
}

void Board::updateTiles() {
    if (!wireUpdates.empty() || !gateUpdates.empty() || !switchUpdates.empty() || !buttonUpdates.empty() || !LEDUpdates.empty()) {
        cout << "\nUpdates scheduled: w" << wireUpdates.size() << " g" << gateUpdates.size() << " s" << switchUpdates.size() << " b" << buttonUpdates.size() << " L" << LEDUpdates.size() << endl;
    }
    if (TileWire::currentUpdateTime > 100) {
        cout << "Thats a lot of updates, remember to check for integer rollover with TileWire::currentUpdateTime." << endl;
    }
    
    for (auto setIter = gateUpdates.begin(); setIter != gateUpdates.end();) {
        if ((*setIter)->updateNextState()) {
            ++setIter;
        } else {
            setIter = gateUpdates.erase(setIter);
        }
    }
    
    // update switches
    
    // update buttons
    
    while (!gateUpdates.empty()) {
        (*gateUpdates.begin())->updateOutput();
    }
    
    while (!wireUpdates.empty()) {
        cout << "Found a remaining wire update." << endl;
        (*wireUpdates.begin())->updateWire(LOW);
    }
    
    TileWire::updateEndpointTiles(this);
    ++TileWire::currentUpdateTime;
}

void Board::replaceTile(Tile* tile) {
    delete _tileArray[tile->getPosition().y][tile->getPosition().x];
    _tileArray[tile->getPosition().y][tile->getPosition().x] = tile;
}

void Board::clear() {
    for (unsigned int y = 0; y < _size.y; ++y) {
        for (unsigned int x = 0; x < _size.x; ++x) {
            delete _tileArray[y][x];
        }
        delete[] _tileArray[y];
    }
    delete[] _tileArray;
    _vertices.clear();
    _size = Vector2u(0, 0);
    _tileArray = nullptr;
}

void Board::resize(const Vector2u& size) {
    _vertices.resize(size.x * size.y * 4);
    Vector2u oldSize = _size;
    _size = size;
    _setVertexCoords();
    
    Tile*** newTileArray = new Tile**[size.y];    // Create new array for the new size.
    unsigned int xStop = min(size.x, oldSize.x), yStop = min(size.y, oldSize.y);
    for (unsigned int y = 0; y < yStop; ++y) {    // Allocate new array, copy over tiles from old array, and delete old tiles that are not used.
        newTileArray[y] = new Tile*[size.x];
        for (unsigned int x = 0; x < xStop; ++x) {
            newTileArray[y][x] = _tileArray[y][x];
        }
        for (unsigned int x = xStop; x < size.x; ++x) {
            newTileArray[y][x] = new Tile(this, Vector2u(x, y));
        }
        for (unsigned int x = xStop; x < oldSize.x; ++x) {
            delete _tileArray[y][x];
        }
        delete[] _tileArray[y];
    }
    for (unsigned int y = yStop; y < size.y; ++y) {    // Add extra rows if necessary.
        newTileArray[y] = new Tile*[size.x];
        for (unsigned int x = 0; x < size.x; ++x) {
            newTileArray[y][x] = new Tile(this, Vector2u(x, y));
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

void Board::cloneArea(const Board& source, const IntRect& region, const Vector2i& destination, bool keepOverwrittenTiles) {
    assert(region.left >= 0 && region.left + region.width <= static_cast<int>(source.getSize().x) && region.top >= 0 && region.top + region.height <= static_cast<int>(source.getSize().y));
    assert(destination.x >= 0 && destination.x + region.width <= static_cast<int>(_size.x) && destination.y >= 0 && destination.y + region.height <= static_cast<int>(_size.y));
    
    int ySource = region.top, yThis = destination.y;
    int yStop = region.top + region.height, xStop = region.left + region.width;
    while (ySource < yStop) {
        int xSource = region.left, xThis = destination.x;
        while (xSource < xStop) {
            if (!keepOverwrittenTiles) {
                delete _tileArray[yThis][xThis];
            }
            _tileArray[yThis][xThis] = source.getTile(Vector2u(xSource, ySource))->clone(this, Vector2u(xThis, yThis));
            ++xSource;
            ++xThis;
        }
        ++ySource;
        ++yThis;
    }
}

void Board::highlightArea(const IntRect& region, bool highlight) {
    for (int y = region.top + region.height - 1; y >= region.top; --y) {
        for (int x = region.left + region.width - 1; x >= region.left; --x) {
            _tileArray[y][x]->setHighlight(highlight);
        }
    }
}

void Board::rotate(bool counterClockwise) {
    _size = Vector2u(_size.y, _size.x);
    _setVertexCoords();
    Tile*** oldTileArray = _tileArray;
    _tileArray = new Tile**[_size.y];
    for (unsigned int y = 0; y < _size.y; ++y) {    // Create new tile array that is same size but rows and columns are swapped.
        _tileArray[y] = new Tile*[_size.x];
    }
    
    for (unsigned int y = 0; y < _size.x; ++y) {    // Loop through each tile in old array and set its position in new array.
        for (unsigned int x = 0; x < _size.y; ++x) {
            if (!counterClockwise) {
                oldTileArray[y][x]->setPosition(Vector2u(_size.x - 1 - y, x), true);
                oldTileArray[y][x]->setDirection(static_cast<Direction>((oldTileArray[y][x]->getDirection() + 1) % 4));
            } else {
                oldTileArray[y][x]->setPosition(Vector2u(y, _size.y - 1 - x), true);
                oldTileArray[y][x]->setDirection(static_cast<Direction>((oldTileArray[y][x]->getDirection() + 3) % 4));
            }
        }
    }
    
    for (unsigned int y = 0; y < _size.x; ++y) {    // Delete old tile array.
        delete[] oldTileArray[y];
    }
    delete[] oldTileArray;
}

void Board::flip(bool acrossHorizontal) {
    if (!acrossHorizontal) {
        for (unsigned int y = 0; y < _size.y; ++y) {
            for (unsigned int x = 0; x < _size.x / 2; ++x) {
                Tile* tempTile1 = _tileArray[y][x];
                Tile* tempTile2 = _tileArray[y][_size.x - 1 - x];
                tempTile1->setPosition(Vector2u(_size.x - 1 - x, y), true);
                tempTile1->flip(false);
                tempTile2->setPosition(Vector2u(x, y), true);
                tempTile2->flip(false);
            }
            if (_size.x % 2 == 1) {
                Tile* tempTile = _tileArray[y][_size.x / 2];
                tempTile->flip(false);
            }
        }
    } else {
        for (unsigned int y = 0; y < _size.y / 2; ++y) {
            for (unsigned int x = 0; x < _size.x; ++x) {
                Tile* tempTile1 = _tileArray[y][x];
                Tile* tempTile2 = _tileArray[_size.y - 1 - y][x];
                tempTile1->setPosition(Vector2u(x, _size.y - 1 - y), true);
                tempTile1->flip(true);
                tempTile2->setPosition(Vector2u(x, y), true);
                tempTile2->flip(true);
            }
        }
        if (_size.y % 2 == 1) {
            for (unsigned int x = 0; x < _size.x; ++x) {
                Tile* tempTile = _tileArray[_size.y / 2][x];
                tempTile->flip(true);
            }
        }
    }
}

void Board::newBoard(const Vector2u& size, const string& filename, bool startEmpty) {
    size_t dotPosition = filename.rfind('.');
    if (dotPosition != string::npos) {
        name = filename.substr(0, dotPosition);
    } else {
        name = filename;
    }
    clear();
    
    _vertices.resize(size.x * size.y * 4);
    _size = size;
    _setVertexCoords();
    _tileArray = new Tile**[size.y];
    for (unsigned int y = 0; y < size.y; ++y) {    // Create new board and add blank tiles if startEmpty is false.
        _tileArray[y] = new Tile*[size.x];
        if (!startEmpty) {
            for (unsigned int x = 0; x < size.x; ++x) {
                _tileArray[y][x] = new Tile(this, Vector2u(x, y));
            }
        }
    }
}

void Board::loadFile(const string& filename) {
    cout << "Loading board file \"" << filename << "\"." << endl;
    ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        throw runtime_error("\"" + filename + "\": Unable to open file for reading.");
    }
    
    size_t dotPosition = filename.rfind('.');
    if (dotPosition != string::npos) {
        name = filename.substr(0, dotPosition);
    } else {
        name = filename;
    }
    clear();
    
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
                    int i;
                    char c1 = line[posX * 2 + 1], c2 = line[posX * 2 + 2];
                    if (c1 == ' ' && c2 == ' ') {     // Check for blank tile.
                        _tileArray[posY][posX] = new Tile(this, Vector2u(posX, posY));
                    } else if ((i = _findSymbol(c1, c2, WIRE_SYMBOL_TABLE)) != -1) {     // Check for wire tile.
                        if (i < 4) {
                            _tileArray[posY][posX] = new TileWire(this, Vector2u(posX, posY), static_cast<Direction>(i / 2 % 4), TileWire::STRAIGHT, static_cast<State>(i % 2 + 1), LOW);
                        } else if (i < 20) {
                            _tileArray[posY][posX] = new TileWire(this, Vector2u(posX, posY), static_cast<Direction>((i - 4) / 2 % 4), static_cast<TileWire::Type>((i + 4) / 8), static_cast<State>(i % 2 + 1), LOW);
                        } else if (i < 22) {
                            _tileArray[posY][posX] = new TileWire(this, Vector2u(posX, posY), NORTH, TileWire::JUNCTION, static_cast<State>(i % 2 + 1), LOW);
                        } else {
                            _tileArray[posY][posX] = new TileWire(this, Vector2u(posX, posY), NORTH, TileWire::CROSSOVER, static_cast<State>(i % 2 + 1), static_cast<State>((i >= 24) + 1));
                        }
                    } else if ((i = _findSymbol(c1, '\0', INPUT_SYMBOL_TABLE)) != -1) {     // Check for input tile.
                        if (i < 2) {
                            _tileArray[posY][posX] = new TileSwitch(this, Vector2u(posX, posY), c2, static_cast<State>(i % 2 + 1));
                        } else {
                            _tileArray[posY][posX] = new TileButton(this, Vector2u(posX, posY), c2, static_cast<State>(i % 2 + 1));
                        }
                    } else if ((i = _findSymbol(c1, c2, OUTPUT_SYMBOL_TABLE)) != -1) {    // Check for output tile.
                        _tileArray[posY][posX] = new TileLED(this, Vector2u(posX, posY), static_cast<State>(i % 2 + 1));
                    } else if ((i = _findSymbol(c1, c2, GATE_SYMBOL_TABLE)) != -1) {    // Check for gate tile.
                        _tileArray[posY][posX] = new TileGate(this, Vector2u(posX, posY), static_cast<Direction>(i / 2 % 4), static_cast<TileGate::Type>(i / 8), static_cast<State>(i % 2 + 1));
                    } else {    // Else, symbol is not valid.
                        string s1, s2;
                        s1.push_back(c1);
                        s2.push_back(c2);
                        throw runtime_error("Invalid symbols \"" + s1 + s2 + "\" at position (" + to_string(posX) + ", " + to_string(posY) + ").");
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
                _setVertexCoords();
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
        _size = Vector2u(0, 0);
        _tileArray = nullptr;
        
        inputFile.close();
        newBoard();
        throw runtime_error(filename + " at line " + to_string(lineNumber) + ": " + ex.what());
    }
    inputFile.close();
    cout << "Load completed." << endl;
}

void Board::_clampToSize(Image& image, const Vector2u& topLeft, const Vector2u& bottomRight) {
    Vector2u tileTopLeft(topLeft + (bottomRight - topLeft) / 4u), tileBottomRight(tileTopLeft + (bottomRight - topLeft) / 2u);
    for (unsigned int y = topLeft.y; y < bottomRight.y; ++y) {
        for (unsigned int x = topLeft.x; x < bottomRight.x; ++x) {
            unsigned int targetX = max(x, tileTopLeft.x), targetY = max(y, tileTopLeft.y);
            if (targetX >= tileBottomRight.x) {
                targetX = tileBottomRight.x - 1;
            }
            if (targetY >= tileBottomRight.y) {
                targetY = tileBottomRight.y - 1;
            }
            if (x < tileTopLeft.x || x >= tileBottomRight.x || y < tileTopLeft.y || y >= tileBottomRight.y) {
                image.setPixel(x, y, image.getPixel(targetX, targetY));
            }
        }
    }
}

void Board::_buildTexture(const Image& source, Texture* target, const Vector2u& tileSize) {
    Image fullImage;
    fullImage.create(source.getSize().x * 2, source.getSize().y * 4, Color::Red);
    for (unsigned int y = 0; y < source.getSize().y; y += tileSize.y) {
        for (unsigned int x = 0; x < source.getSize().x; x += tileSize.x) {
            fullImage.copy(source, x * 2 + tileSize.x / 2, y * 2 + tileSize.y / 2, IntRect(x, y, tileSize.x, tileSize.y));
            _clampToSize(fullImage, Vector2u(x * 2, y * 2), Vector2u((x + tileSize.x) * 2, (y + tileSize.y) * 2));
        }
    }
    fullImage.copy(fullImage, 0, source.getSize().y * 2);
    for (unsigned int y = source.getSize().y * 2; y < fullImage.getSize().y; ++y) {
        for (unsigned int x = 0; x < fullImage.getSize().x; ++x) {
            fullImage.setPixel(x, y, fullImage.getPixel(x, y) + Color(100, 100, 100));
        }
    }
    target->loadFromImage(fullImage);
}

void Board::_redrawTile(Tile* tile) {
    Vertex* tileVertices = &_vertices[(tile->getPosition().y * _size.x + tile->getPosition().x) * 4];
    int offsetID = tile->getHighlight() ? _textureIDMax : 0;
    float tileX = static_cast<float>((tile->getTextureID() + offsetID) % (_tilesetGridPtr->getSize().x / 2 / _tileSize.x) * _tileSize.x * 2 + _tileSize.x / 2);
    float tileY = static_cast<float>((tile->getTextureID() + offsetID) / (_tilesetGridPtr->getSize().x / 2 / _tileSize.x) * _tileSize.y * 2 + _tileSize.y / 2);
    int d = tile->getDirection();
    tileVertices[d % 4].texCoords = Vector2f(tileX, tileY);
    tileVertices[(d + 1) % 4].texCoords = Vector2f(tileX + _tileSize.x, tileY);
    tileVertices[(d + 2) % 4].texCoords = Vector2f(tileX + _tileSize.x, tileY + _tileSize.y);
    tileVertices[(d + 3) % 4].texCoords = Vector2f(tileX, tileY + _tileSize.y);
}

void Board::_setVertexCoords() {
    for (unsigned int y = 0; y < _size.y; ++y) {
        for (unsigned int x = 0; x < _size.x; ++x) {
            Vertex* tileVertices = &_vertices[(y * _size.x + x) * 4];
            float windowX = static_cast<float>(x * _tileSize.x);
            float windowY = static_cast<float>(y * _tileSize.y);
            tileVertices[0].position = Vector2f(windowX, windowY);
            tileVertices[1].position = Vector2f(windowX + _tileSize.x, windowY);
            tileVertices[2].position = Vector2f(windowX + _tileSize.x, windowY + _tileSize.y);
            tileVertices[3].position = Vector2f(windowX, windowY + _tileSize.y);
        }
    }
}

int Board::_findSymbol(char c1, char c2, const vector<string>& symbolTable) const {
    if (c2 == '\0') {
        for (unsigned int i = 0; i < symbolTable.size(); ++i) {
            if (c1 == symbolTable[i][0]) {
                return i;
            }
        }
        return -1;
    } else {
        for (unsigned int i = 0; i < symbolTable.size(); ++i) {
            if (c1 == symbolTable[i][0] && c2 == symbolTable[i][1]) {
                return i;
            }
        }
        return -1;
    }
}

void Board::draw(RenderTarget& target, RenderStates states) const {
    states.transform *= getTransform();
    if (gridActive) {
        states.texture = _tilesetGridPtr;
    } else {
        states.texture = _tilesetNoGridPtr;
    }
    target.draw(_vertices, states);
}