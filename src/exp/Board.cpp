#include <Board.h>
#include <Tile.h>
#include <tiles/Gate.h>
#include <tiles/Input.h>
#include <tiles/Led.h>
#include <tiles/Wire.h>

#include <array>
#include <cassert>
#include <fstream>
#include <iostream>

struct TileSymbol {
    char a, b;

    constexpr TileSymbol(const char* s) :
        a(s[0]), b(s[1]) {
    }
    constexpr TileSymbol(char a, char b) :
        a(a), b(b) {
    }
    bool operator<(const TileSymbol& rhs) const {
        return a < rhs.a || (a == rhs.a && b < rhs.b);
    }
};

constexpr std::array<TileSymbol, 157> TILE_SYMBOLS = {
    "  ",
    "| ",  "[ ",  "{ ",  "--",  "==",  "~~",
    "\'-", "\"=", "`~",  ",-",  ";=",  ".~",  ", ",  "; ",  ". ",  "\' ", "\" ", "` ",
    ">-",  ">=",  ">~",  "v-",  "v=",  "v~",  "< ",  "<.",  "<:",  "^-",  "^=",  "^~",
    "+-",  "#=",  "+~",
    "|-",  "[-",  "{-",  "|=",  "[=",  "{=",  "|~",  "[~",  "{~",
    "s",   "S",
    "t",   "T",
    "..",  "##",
    "^d",  "^D",  "`d",  ">d",  ">D",  "}d",  "vd",  "vD",  ",d",  "<d",  "<D",  "{d",
    "^m",  "^M",  "`m",  ">m",  ">M",  "}m",  "vm",  "vM",  ",m",  "<m",  "<M",  "{m",
    "^n",  "^N",  "`n",  ">n",  ">N",  "}n",  "vn",  "vN",  ",n",  "<n",  "<N",  "{n",
    "^a",  "^A",  "`a",  ">a",  ">A",  "}a",  "va",  "vA",  ",a",  "<a",  "<A",  "{a",
    "^b",  "^B",  "`b",  ">b",  ">B",  "}b",  "vb",  "vB",  ",b",  "<b",  "<B",  "{b",
    "^o",  "^O",  "`o",  ">o",  ">O",  "}o",  "vo",  "vO",  ",o",  "<o",  "<O",  "{o",
    "^p",  "^P",  "`p",  ">p",  ">P",  "}p",  "vp",  "vP",  ",p",  "<p",  "<P",  "{p",
    "^x",  "^X",  "`x",  ">x",  ">X",  "}x",  "vx",  "vX",  ",x",  "<x",  "<X",  "{x",
    "^y",  "^Y",  "`y",  ">y",  ">Y",  "}y",  "vy",  "vY",  ",y",  "<y",  "<Y",  "{y"
};

namespace TileSymbolIndex {
    enum t : unsigned int {
        blank = 0,
        wireStraight = 1,
        wireCorner = 7,
        wireTee = 19,
        wireJunction = 31,
        wireCrossover = 34,
        inSwitch = 43,
        inButton = 45,
        outLed = 47,
        gateDiode = 49,
        gateBuffer = 61,
        gateNot = 73,
        gateAnd = 85,
        gateNand = 97,
        gateOr = 109,
        gateNor = 121,
        gateXor = 133,
        gateXnor = 145
    };
}
// FIXME this stuff related to ascii file loading should go in a separate file.



Board::Board(sf::Texture* tilesetGrid) :
    tilesetGrid_(tilesetGrid) {

    chunks_.emplace(0, Chunk());
}

Tile Board::accessTile(int x, int y) {
    auto& chunk = getChunk(x, y);
    return chunk.accessTile(x % Chunk::WIDTH, y % Chunk::WIDTH);
}

void Board::loadFromFile(const std::string& filename) {
    std::ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        throw std::runtime_error("\"" + filename + "\": unable to open file for reading.");
    }

    static std::map<TileSymbol, unsigned int> symbolLookup;

    if (symbolLookup.empty()) {
        std::cout << "First time init symbolLookup.\n";
        for (size_t i = 0; i < TILE_SYMBOLS.size(); ++i) {
            symbolLookup.emplace(TILE_SYMBOLS[i], i);
        }

        std::cout << "symbolLookup contents:\n";
        for (const auto& x : symbolLookup) {
            std::cout << x.first.a << x.first.b << " -> " << x.second << "\n";
        }
        std::cout << "\n";
    }

    std::string line;
    int lineNumber = 0;
    ParseState parseState;
    parseState.filename = filename;
    try {
        while (std::getline(inputFile, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            ++lineNumber;
            parseFile(line, lineNumber, parseState, symbolLookup);
        }
        if (parseState.lastField != "done") {
            throw std::runtime_error("missing data, end of file reached.");
        }
    } catch (std::exception& ex) {
        inputFile.close();
        throw std::runtime_error("\"" + filename + "\" at line " + std::to_string(lineNumber) + ": " + ex.what());
    }
    inputFile.close();
    std::cout << "Load completed.\n";

    std::cout << "\nfileVersion = " << parseState.fileVersion << "\n";
    std::cout << "width = " << parseState.width << "\n";
    std::cout << "height = " << parseState.height << "\n";
    std::cout << "enableExtraLogicStates = " << parseState.enableExtraLogicStates << "\n";
    std::cout << "notesString = \"" << parseState.notesString << "\"\n";
}

void Board::saveToFile(const std::string& /*filename*/) {

}

void Board::parseFile(const std::string& line, int lineNumber, ParseState& parseState, const std::map<TileSymbol, unsigned int>& symbolLookup) {
    std::cout << "Parse line [" << line << "]\n";

    if (line.length() == 0) {
        if (parseState.lastField == "notes: {") {
            parseState.notesString.push_back('\n');
        }
    } else if (parseState.lastField == "tiles") {
        if (parseState.y == 0) {
            std::cout << "========== START OF TILES ==========\n";
        }
        if (line.length() != parseState.width * 2 + 2) {
            throw std::runtime_error("incorrect length of line (expected " + std::to_string(parseState.width * 2 + 2) + ", got " + std::to_string(line.length()) + ").");
        }
        parseState.x = 0;
        while (parseState.x < static_cast<int>(parseState.width)) {
            TileSymbol symbol(line[parseState.x * 2 + 1], line[parseState.x * 2 + 2]);
            auto foundSymbol = symbolLookup.find(symbol);
            if (foundSymbol == symbolLookup.end()) {
                foundSymbol = symbolLookup.find(TileSymbol(symbol.a, '\0'));
                if (foundSymbol == symbolLookup.end()) {
                    std::string s;
                    s.push_back(symbol.a);
                    s.push_back(symbol.b);
                    throw std::runtime_error("invalid symbols \"" + s + "\" at position (" + std::to_string(parseState.x) + ", " + std::to_string(parseState.y) + ").");
                }
            }

            unsigned int symbolId = foundSymbol->second;
            std::cout << symbolId << " ";
            Tile tile = accessTile(parseState.x, parseState.y);
            if (symbolId == TileSymbolIndex::blank) {
                // Blank tile.
            } else if (symbolId < TileSymbolIndex::inSwitch) {
                // Wire tile.
                if (symbolId < TileSymbolIndex::wireCorner) {
                    tile.setType(
                        tiles::Wire::instance(), TileId::wireStraight,
                        static_cast<Direction::t>((symbolId - TileSymbolIndex::wireStraight) / 3),
                        static_cast<State::t>((symbolId - TileSymbolIndex::wireStraight) % 3 + 1)
                    );
                } else if (symbolId < TileSymbolIndex::wireTee) {
                    tile.setType(
                        tiles::Wire::instance(), TileId::wireCorner,
                        static_cast<Direction::t>((symbolId - TileSymbolIndex::wireCorner) / 3),
                        static_cast<State::t>((symbolId - TileSymbolIndex::wireCorner) % 3 + 1)
                    );
                } else if (symbolId < TileSymbolIndex::wireJunction) {
                    tile.setType(
                        tiles::Wire::instance(), TileId::wireTee,
                        static_cast<Direction::t>((symbolId - TileSymbolIndex::wireTee) / 3),
                        static_cast<State::t>((symbolId - TileSymbolIndex::wireTee) % 3 + 1)
                    );
                } else if (symbolId < TileSymbolIndex::wireCrossover) {
                    tile.setType(
                        tiles::Wire::instance(), TileId::wireJunction,
                        Direction::north,
                        static_cast<State::t>((symbolId - TileSymbolIndex::wireJunction) % 3 + 1)
                    );
                } else {
                    tile.setType(
                        tiles::Wire::instance(), TileId::wireCrossover,
                        Direction::north,
                        static_cast<State::t>((symbolId - TileSymbolIndex::wireCrossover) % 3 + 1),
                        static_cast<State::t>((symbolId - TileSymbolIndex::wireCrossover) / 3 + 1)
                    );
                }
            } else if (symbolId < TileSymbolIndex::outLed) {
                // Input tile.
                tile.setType(
                    tiles::Input::instance(),
                    static_cast<TileId::t>((symbolId - TileSymbolIndex::inSwitch) / 2 + TileId::inSwitch),
                    static_cast<State::t>((symbolId - TileSymbolIndex::inSwitch) % 2 + 1),
                    symbol.b
                );
            } else if (symbolId < TileSymbolIndex::gateDiode) {
                // Output tile.
                tile.setType(
                    tiles::Led::instance(),
                    static_cast<State::t>((symbolId - TileSymbolIndex::outLed) % 2 + 1)
                );
            } else {
                // Gate tile.
                tile.setType(
                    tiles::Gate::instance(),
                    static_cast<TileId::t>((symbolId - TileSymbolIndex::gateDiode) / 12 + TileId::gateDiode),
                    static_cast<Direction::t>((symbolId - TileSymbolIndex::gateDiode) / 3 % 4),
                    static_cast<State::t>((symbolId - TileSymbolIndex::gateDiode) % 3 + 1)
                );
            }

            ++parseState.x;
        }
        std::cout << "\n";
        ++parseState.y;
        if (parseState.y == static_cast<int>(parseState.height)) {
            parseState.lastField = "tilesEnd";
            std::cout << "========== END OF TILES ==========\n";
        }
    } else if (parseState.lastField == "" && line.find("version:") == 0) {
        parseState.lastField = "version:";
        parseState.fileVersion = std::stof(line.substr(parseState.lastField.length()));
        if (parseState.fileVersion != 1.0f) {
            throw std::runtime_error("invalid file version.");
        }
    } else if (parseState.lastField == "version:" && line.find("width:") == 0) {
        parseState.lastField = "width:";
        int width = std::stoi(line.substr(parseState.lastField.length()));
        if (width < 0) {
            throw std::runtime_error("board dimensions cannot be negative.");
        }
        parseState.width = width;
    } else if (parseState.lastField == "width:" && line.find("height:") == 0) {
        parseState.lastField = "height:";
        int height = std::stoi(line.substr(parseState.lastField.length()));
        if (height < 0) {
            throw std::runtime_error("board dimensions cannot be negative.");
        }
        parseState.height = height;

        if (parseState.width > 0 && parseState.height > 0) {
            maxSize_.x = ((parseState.width - 1) / Chunk::WIDTH + 1) * Chunk::WIDTH;
            maxSize_.y = ((parseState.height - 1) / Chunk::WIDTH + 1) * Chunk::WIDTH;
        }

    } else if (parseState.lastField == "height:" && line == "data: {") {
        parseState.lastField = "data: {";
    } else if (parseState.lastField == "data: {") {
        if (line.find("extraLogicStates:") == 0) {
            parseState.enableExtraLogicStates = std::stoi(line.substr(17));
        } else if (line == "}") {
            parseState.lastField = "data: }";
        } else {
            std::cout << "Warn: \"" + parseState.filename + "\" at line " + std::to_string(lineNumber) + ": found some unrecognized data.\n";
        }
    } else if (parseState.lastField == "data: }" && line == "notes: {") {
        parseState.lastField = "notes: {";
    } else if (parseState.lastField == "notes: {") {
        if (line != "}") {
            parseState.notesString += line + "\n";
        } else {
            parseState.lastField = "notes: }";
        }
    } else if (parseState.lastField == "notes: }" || parseState.lastField == "tilesEnd") {
        if (line.length() != parseState.width * 2 + 2) {
            throw std::runtime_error("incorrect length of line (expected " + std::to_string(parseState.width * 2 + 2) + ", got " + std::to_string(line.length()) + ").");
        }
        if (parseState.lastField == "notes: }") {
            parseState.lastField = "tiles";
        } else {
            parseState.lastField = "done";
        }
    } else {
        throw std::runtime_error("unexpected board file data.");
    }
}

Chunk& Board::getChunk(int x, int y) {
    uint64_t mapIndex = static_cast<uint64_t>(y / Chunk::WIDTH) << 32 | static_cast<uint32_t>(x / Chunk::WIDTH);
    auto chunk = chunks_.find(mapIndex);
    if (chunk != chunks_.end()) {
        return chunk->second;
    } else {
        assert(false);
    }
}

void Board::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    states.transform *= getTransform();
    states.texture = tilesetGrid_;
    for (const auto& chunk : chunks_) {
        target.draw(chunk.second, states);
    }
}
