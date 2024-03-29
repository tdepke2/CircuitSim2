#include "Board.h"
#include "TileButton.h"
#include "TileGate.h"
#include "TileLED.h"
#include "TileSwitch.h"
#include "TileWire.h"
#include "UserInterface.h"
#include <cassert>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>

string Board::newBoardDefaultPath = "boards/NewBoard.txt";
bool Board::gridActive = true;
bool Board::enableExtraLogicStates = true;
int Board::numStateErrors = 0;
vector<TileLED*> Board::endpointLEDs;
vector<TileGate*> Board::endpointGates;
Texture* Board::_tilesetGridPtr = nullptr;
Texture* Board::_tilesetNoGridPtr = nullptr;
Font* Board::_fontPtr = nullptr;
int Board::_textureIDMax;
Vector2u Board::_tileSize;

const Font& Board::getFont() {
    return *_fontPtr;
}

const Vector2u& Board::getTileSize() {
    return _tileSize;
}

void Board::loadTextures(const string& filenameGrid, const string& filenameNoGrid, const Vector2u& tileSize) {
    cout << "Building and stitching textures..." << endl;
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
        UserInterface::pushMessage("Warn: \"" + filenameGrid + "\": Unable to generate mipmap for texture.", true);
    }
    
    if (!tilesetImage.loadFromFile(filenameNoGrid)) {
        throw runtime_error("\"" + filenameNoGrid + "\": Unable to load texture file.");
    }
    _buildTexture(tilesetImage, _tilesetNoGridPtr, tileSize);
    _tilesetNoGridPtr->setSmooth(true);
    if (!_tilesetNoGridPtr->generateMipmap()) {
        UserInterface::pushMessage("Warn: \"" + filenameNoGrid + "\": Unable to generate mipmap for texture.", true);
    }
    
    _textureIDMax = tilesetImage.getSize().x / tileSize.x * tilesetImage.getSize().y / tileSize.y;
    _tileSize = tileSize;
}

void Board::loadFont(const string& filename) {
    delete _fontPtr;
    _fontPtr = new Font();
    if (!_fontPtr->loadFromFile(filename)) {
        throw runtime_error("\"" + filename + "\": Unable to load font file.");
    }
}

Board::Board() {
    _vertices.setPrimitiveType(Quads);
    _size = Vector2u(0, 0);
    _tileArray = nullptr;
    _notesBox.setSize(Vector2f(0.0f, 0.0f));
    _notesBox.setFillColor(Color(30, 30, 30));
    _notesText = Text("", getFont(), 25);
    _notesText.setFillColor(Color(150, 150, 150));
    changesMade = false;
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
    changesMade = true;
}

void Board::setTile(const Vector2u& position, Tile* tile) {
    _tileArray[position.y][position.x] = tile;
    changesMade = true;
}

void Board::redrawTileVertices(int textureID, const Vector2u& position, Direction direction, bool highlight) {
    Vertex* tileVertices = &_vertices[(position.y * _size.x + position.x) * 4];
    int offsetID = highlight ? _textureIDMax : 0;
    float tileX = static_cast<float>((textureID + offsetID) % (_tilesetGridPtr->getSize().x / 2 / _tileSize.x) * _tileSize.x * 2 + _tileSize.x / 2);
    float tileY = static_cast<float>((textureID + offsetID) / (_tilesetGridPtr->getSize().x / 2 / _tileSize.x) * _tileSize.y * 2 + _tileSize.y / 2);
    tileVertices[direction % 4].texCoords = Vector2f(tileX, tileY);
    tileVertices[(direction + 1) % 4].texCoords = Vector2f(tileX + _tileSize.x, tileY);
    tileVertices[(direction + 2) % 4].texCoords = Vector2f(tileX + _tileSize.x, tileY + _tileSize.y);
    tileVertices[(direction + 3) % 4].texCoords = Vector2f(tileX, tileY + _tileSize.y);
}

void Board::updateCosmetics() {
    for (auto setIter = cosmeticUpdates.begin(); setIter != cosmeticUpdates.end(); ++setIter) {
        (*setIter)->redrawTile();
    }
    cosmeticUpdates.clear();
}

void Board::updateTiles() {
    assert(endpointLEDs.empty());
    assert(endpointGates.empty());
    if (wireUpdates.empty() && gateUpdates.empty() && switchUpdates.empty() && buttonUpdates.empty() && LEDUpdates.empty()) {
        return;
    }
    /*if (!wireUpdates.empty() || !gateUpdates.empty() || !switchUpdates.empty() || !buttonUpdates.empty() || !LEDUpdates.empty()) {
        cout << "\nUpdates scheduled: w" << wireUpdates.size() << " g" << gateUpdates.size() << " s" << switchUpdates.size() << " b" << buttonUpdates.size() << " L" << LEDUpdates.size() << endl;
    }*/
    
    //cout << "---- GATE STATE CHECKS ----" << endl;
    for (auto setIter = gateUpdates.begin(); setIter != gateUpdates.end();) {    // Check state transitions for all gates (keep update only if gate changed).
        if ((*setIter)->updateNextState()) {
            ++setIter;
        } else {
            setIter = gateUpdates.erase(setIter);
        }
    }
    
    //cout << "---- SWITCH UPDATES ----" << endl;
    while (!switchUpdates.empty()) {    // Update all pending switches.
        (*switchUpdates.begin())->updateOutput();
    }
    //cout << "---- BUTTON UPDATES ----" << endl;
    while (!buttonUpdates.empty()) {    // Update all pending buttons.
        (*buttonUpdates.begin())->updateOutput();
    }
    //cout << "---- GATE UPDATES ----" << endl;
    while (!gateUpdates.empty()) {    // Update all pending gates that were left over from before.
        (*gateUpdates.begin())->updateOutput();
    }
    
    //cout << "---- REMAINING WIRES AND LEDS ----" << endl;
    while (!wireUpdates.empty()) {
        //cout << "Found a remaining wire update." << endl;
        (*wireUpdates.begin())->updateWire(LOW);
    }
    while (!LEDUpdates.empty()) {
        //cout << "Found a remaining LED update." << endl;
        (*LEDUpdates.begin())->updateLED(LOW);
    }
    
    //cout << "LED endpoints:" << endl;
    for (TileLED* LED : endpointLEDs) {
        //cout << "  (" << LED->getPosition().x << ", " << LED->getPosition().y << ")" << endl;
        LED->updateLED(LOW);
    }
    endpointLEDs.clear();
    //cout << "Gate endpoints:" << endl;
    for (TileGate* gate : endpointGates) {
        //cout << "  (" << gate->getPosition().x << ", " << gate->getPosition().y << ")" << endl;
        gateUpdates.insert(gate);
    }
    endpointGates.clear();
    
    TileButton::updateTransitioningButtons();
    //cout << endl;
    if (Tile::currentUpdateTime == numeric_limits<unsigned int>::max()) {
        UserInterface::pushMessage("Update counter has reached max value (" + to_string(numeric_limits<unsigned int>::max()) + " updates), resetting tiles...");
        for (unsigned int y = 0; y < _size.y; ++y) {
            for (unsigned int x = 0; x < _size.x; ++x) {
                _tileArray[y][x]->fixUpdateTime();
            }
        }
        Tile::currentUpdateTime = 1;
        cout << "Tiles reset." << endl;
    } else {
        ++Tile::currentUpdateTime;
    }
}

void Board::replaceTile(Tile* tile) {
    delete _tileArray[tile->getPosition().y][tile->getPosition().x];
    _tileArray[tile->getPosition().y][tile->getPosition().x] = tile;
    changesMade = true;
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
    _notesBox.setSize(Vector2f(0.0f, 0.0f));
    _notesText.setString("");
    changesMade = false;
}

void Board::resize(const Vector2u& size) {
    if (size.x == 0 || size.y == 0) {
        throw runtime_error("Board dimensions cannot be zero or negative.");
    }
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
            newTileArray[y][x]->addUpdate(false, true);
        }
        for (unsigned int x = xStop; x < size.x; ++x) {
            newTileArray[y][x] = new Tile(this, Vector2u(x, y), true);
        }
        for (unsigned int x = xStop; x < oldSize.x; ++x) {
            delete _tileArray[y][x];
        }
        delete[] _tileArray[y];
    }
    for (unsigned int y = yStop; y < size.y; ++y) {    // Add extra rows if necessary.
        newTileArray[y] = new Tile*[size.x];
        for (unsigned int x = 0; x < size.x; ++x) {
            newTileArray[y][x] = new Tile(this, Vector2u(x, y), true);
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
    changesMade = true;
    UserInterface::pushMessage("Board size changed to " + to_string(_size.x) + " x " + to_string(_size.y) + ".");
}

void Board::cloneArea(const Board& source, const IntRect& region, const Vector2i& destination, bool noAdjacentUpdates, bool keepOverwrittenTiles) {
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
            _tileArray[yThis][xThis] = source.getTile(Vector2u(xSource, ySource))->clone(this, Vector2u(xThis, yThis), noAdjacentUpdates);
            ++xSource;
            ++xThis;
        }
        ++ySource;
        ++yThis;
    }
    changesMade = true;
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
                oldTileArray[y][x]->setPosition(Vector2u(_size.x - 1 - y, x), true, true);
                oldTileArray[y][x]->setDirection(static_cast<Direction>((oldTileArray[y][x]->getDirection() + 1) % 4), true);
            } else {
                oldTileArray[y][x]->setPosition(Vector2u(y, _size.y - 1 - x), true, true);
                oldTileArray[y][x]->setDirection(static_cast<Direction>((oldTileArray[y][x]->getDirection() + 3) % 4), true);
            }
        }
    }
    
    for (unsigned int y = 0; y < _size.x; ++y) {    // Delete old tile array.
        delete[] oldTileArray[y];
    }
    delete[] oldTileArray;
    changesMade = true;
}

void Board::flip(bool acrossHorizontal) {
    if (!acrossHorizontal) {
        for (unsigned int y = 0; y < _size.y; ++y) {
            for (unsigned int x = 0; x < _size.x / 2; ++x) {
                Tile* tempTile1 = _tileArray[y][x];
                Tile* tempTile2 = _tileArray[y][_size.x - 1 - x];
                tempTile1->setPosition(Vector2u(_size.x - 1 - x, y), true, true);
                tempTile1->flip(false, true);
                tempTile2->setPosition(Vector2u(x, y), true, true);
                tempTile2->flip(false, true);
            }
            if (_size.x % 2 == 1) {
                Tile* tempTile = _tileArray[y][_size.x / 2];
                tempTile->flip(false, true);
            }
        }
    } else {
        for (unsigned int y = 0; y < _size.y / 2; ++y) {
            for (unsigned int x = 0; x < _size.x; ++x) {
                Tile* tempTile1 = _tileArray[y][x];
                Tile* tempTile2 = _tileArray[_size.y - 1 - y][x];
                tempTile1->setPosition(Vector2u(x, _size.y - 1 - y), true, true);
                tempTile1->flip(true, true);
                tempTile2->setPosition(Vector2u(x, y), true, true);
                tempTile2->flip(true, true);
            }
        }
        if (_size.y % 2 == 1) {
            for (unsigned int x = 0; x < _size.x; ++x) {
                Tile* tempTile = _tileArray[_size.y / 2][x];
                tempTile->flip(true, true);
            }
        }
    }
    changesMade = true;
}

void Board::newBoard(const Vector2u& size, const string& filename, bool startEmpty) {
    if (size.x == 0 || size.y == 0) {
        throw runtime_error("Board dimensions cannot be zero or negative.");
    }
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
                _tileArray[y][x] = new Tile(this, Vector2u(x, y), true);
            }
        }
    }
    changesMade = false;
}

void Board::loadFile(const string& filename) {
    UserInterface::pushMessage("Loading board file \"" + filename + "\"...");
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
    enableExtraLogicStates = false;    // By default, extra logic states are off for backwards compatibility.
    
    string line, notesString;
    float fileVersion;
    int lineNumber = 0, numEntries = 0;
    unsigned int posX = 0, posY = 0;
    try {
        while (getline(inputFile, line)) {
            ++lineNumber;
            if (line.length() == 0 && numEntries < 8) {
                if (numEntries == 6) {
                    notesString += "\n";
                } else {
                    continue;
                }
            } else if (numEntries == 8) {
                if (line.length() != _size.x * 2 + 2) {
                    throw runtime_error("Length of this line is incorrect.");
                }
                posX = 0;
                while (posX < _size.x) {
                    int i;
                    char c1 = line[posX * 2 + 1], c2 = line[posX * 2 + 2];
                    if (c1 == ' ' && c2 == ' ') {     // Check for blank tile.
                        _tileArray[posY][posX] = new Tile(this, Vector2u(posX, posY), true);
                    } else if ((i = _findSymbol(c1, c2, WIRE_SYMBOL_TABLE)) != -1) {     // Check for wire tile.
                        if (i < 6) {
                            _tileArray[posY][posX] = new TileWire(this, Vector2u(posX, posY), true, static_cast<Direction>(i / 3 % 4), TileWire::STRAIGHT, static_cast<State>(i % 3 + 1), LOW);
                        } else if (i < 30) {
                            _tileArray[posY][posX] = new TileWire(this, Vector2u(posX, posY), true, static_cast<Direction>((i - 6) / 3 % 4), static_cast<TileWire::Type>((i + 6) / 12), static_cast<State>(i % 3 + 1), LOW);
                        } else if (i < 33) {
                            _tileArray[posY][posX] = new TileWire(this, Vector2u(posX, posY), true, NORTH, TileWire::JUNCTION, static_cast<State>(i % 3 + 1), LOW);
                        } else {
                            _tileArray[posY][posX] = new TileWire(this, Vector2u(posX, posY), true, NORTH, TileWire::CROSSOVER, static_cast<State>(i % 3 + 1), static_cast<State>((i - 33) / 3 + 1));
                        }
                    } else if ((i = _findSymbol(c1, '\0', INPUT_SYMBOL_TABLE)) != -1) {     // Check for input tile.
                        if (i < 2) {
                            _tileArray[posY][posX] = new TileSwitch(this, Vector2u(posX, posY), true, c2, static_cast<State>(i % 2 + 1));
                        } else {
                            _tileArray[posY][posX] = new TileButton(this, Vector2u(posX, posY), true, c2, static_cast<State>(i % 2 + 1));
                        }
                    } else if ((i = _findSymbol(c1, c2, OUTPUT_SYMBOL_TABLE)) != -1) {    // Check for output tile.
                        _tileArray[posY][posX] = new TileLED(this, Vector2u(posX, posY), true, static_cast<State>(i % 2 + 1));
                    } else if ((i = _findSymbol(c1, c2, GATE_SYMBOL_TABLE)) != -1) {    // Check for gate tile.
                        _tileArray[posY][posX] = new TileGate(this, Vector2u(posX, posY), true, static_cast<Direction>(i / 3 % 4), static_cast<TileGate::Type>(i / 12), static_cast<State>(i % 3 + 1));
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
            } else if (numEntries == 0 && line.find("version:") == 0) {
                fileVersion = stof(line.substr(8));
                if (fileVersion != 1.0f) {
                    throw runtime_error("Invalid file version.");
                }
                ++numEntries;
            } else if (numEntries == 1 && line.find("width:") == 0) {
                int sizeX = stol(line.substr(6));
                if (sizeX <= 0) {
                    throw runtime_error("Board dimensions cannot be zero or negative.");
                } else {
                    _size.x = sizeX;
                }
                ++numEntries;
            } else if (numEntries == 2 && line.find("height:") == 0) {
                int sizeY = stol(line.substr(7));
                if (sizeY <= 0) {
                    throw runtime_error("Board dimensions cannot be zero or negative.");
                } else {
                    _size.y = sizeY;
                }
                _vertices.resize(_size.x * _size.y * 4);
                _setVertexCoords();
                _tileArray = new Tile**[_size.y];
                for (unsigned int y = 0; y < _size.y; ++y) {
                    _tileArray[y] = new Tile*[_size.x];
                }
                ++numEntries;
            } else if (numEntries == 3 && line == "data: {") {
                ++numEntries;
            } else if (numEntries == 4) {
                if (line.find("extraLogicStates:") == 0) {
                    enableExtraLogicStates = stoi(line.substr(17));
                } else if (line != "}") {
                    UserInterface::pushMessage("Warn: \"" + filename + "\" at line " + to_string(lineNumber) + ": Found some unrecognized data.", true);
                } else {
                    ++numEntries;
                }
            } else if (numEntries == 5 && line == "notes: {") {
                ++numEntries;
            } else if (numEntries == 6) {
                if (line != "}") {
                    notesString += line + "\n";
                } else {
                    if (notesString.length() != 0) {
                        _notesText.setString(notesString);
                        _notesBox.setSize(Vector2f(_notesText.getLocalBounds().width + 8.0f, _notesText.getLocalBounds().height + 8.0f));
                        _notesBox.setPosition(0.0f, -_notesBox.getSize().y - 2.0f);
                        _notesText.setPosition(4.0f, -_notesBox.getSize().y - 5.0f);
                    }
                    ++numEntries;
                }
            } else if (numEntries == 7 || numEntries == 9) {
                if (line.length() != _size.x * 2 + 2) {
                    throw runtime_error("Length of this line is incorrect.");
                }
                ++numEntries;
            } else {
                throw runtime_error("Invalid save file data.");
            }
        }
        if (numEntries != 10) {
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
        throw runtime_error("\"" + filename + "\" at line " + to_string(lineNumber) + ": " + ex.what());
    }
    inputFile.close();
    changesMade = false;
    cout << "Load completed." << endl;
}

void Board::saveFile(const string& filename) {
    UserInterface::pushMessage("Saving board file \"" + filename + "\"...");
    ofstream outputFile(filename);
    if (!outputFile.is_open()) {
        throw runtime_error("\"" + filename + "\": Unable to open file for writing.");
    }
    
    size_t dotPosition = filename.rfind('.');
    if (dotPosition != string::npos) {
        name = filename.substr(0, dotPosition);
    } else {
        name = filename;
    }
    
    outputFile << "version: 1.0" << endl;
    outputFile << "width: " << _size.x << endl;
    outputFile << "height: " << _size.y << endl;
    outputFile << "data: {" << endl;
    outputFile << "extraLogicStates: " << enableExtraLogicStates << endl;
    outputFile << "}" << endl;
    outputFile << "notes: {" << endl;
    outputFile << _notesText.getString().toAnsiString();
    outputFile << "}" << endl << endl;
    outputFile << setfill ('*') << setw (_size.x * 2 + 2) << "*" << setfill (' ') << endl;
    for (unsigned int y = 0; y < _size.y; ++y) {
        outputFile << "*";
        for (unsigned int x = 0; x < _size.x; ++x) {
            outputFile << _tileArray[y][x]->toString();
        }
        outputFile << "*" << endl;
    }
    outputFile << setfill ('*') << setw (_size.x * 2 + 2) << "*" << setfill (' ');
    
    outputFile.close();
    changesMade = false;
    cout << "Save completed." << endl;
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
    for (const pair<Tile*, Text>& tileLabel : tileLabels) {
        target.draw(tileLabel.second, states);
    }
    target.draw(_notesBox, states);
    target.draw(_notesText, states);
}