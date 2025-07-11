#include <Board.h>
#include <LegacyFileFormat.h>
#include <Tile.h>
#include <tiles/Blank.h>
#include <tiles/Gate.h>
#include <tiles/Input.h>
#include <tiles/Led.h>
#include <tiles/Wire.h>

#include <array>
#include <iomanip>
#include <map>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace {

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

    friend std::ostream& operator<<(std::ostream& out, const TileSymbol& symbol) {
        return out << symbol.a << symbol.b;
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

std::map<TileSymbol, unsigned int> symbolLookup;

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

void setTileFromSymbol(Board& board, int x, int y, const TileSymbol& symbol) {
    auto foundSymbol = symbolLookup.find(symbol);
    if (foundSymbol == symbolLookup.end()) {
        foundSymbol = symbolLookup.find(TileSymbol(symbol.a, '\0'));
        if (foundSymbol == symbolLookup.end()) {
            std::string s;
            s.push_back(symbol.a);
            s.push_back(symbol.b);
            throw FileStorageError("invalid symbols \"" + s + "\" at position (" + std::to_string(x) + ", " + std::to_string(y) + ").");
        }
    }

    Tile tile = board.accessTile(x, y);
    unsigned int symbolId = foundSymbol->second;
    //std::cout << symbolId << " ";
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
}

TileSymbol tileToSymbol(Board& board, int x, int y) {
    Tile tile = board.accessTile(x, y);
    auto tileId = tile.getId();
    if (tile.getType() == tiles::Blank::instance()) {
        return TILE_SYMBOLS[TileSymbolIndex::blank];
    } else if (tile.getType() == tiles::Wire::instance()) {
        if (tileId == TileId::wireStraight) {
            return TILE_SYMBOLS[TileSymbolIndex::wireStraight + tile.getDirection() * 3 + tile.getState() - 1];
        } else if (tileId == TileId::wireCorner) {
            return TILE_SYMBOLS[TileSymbolIndex::wireCorner + tile.getDirection() * 3 + tile.getState() - 1];
        } else if (tileId == TileId::wireTee) {
            return TILE_SYMBOLS[TileSymbolIndex::wireTee + tile.getDirection() * 3 + tile.getState() - 1];
        } else if (tileId == TileId::wireJunction) {
            return TILE_SYMBOLS[TileSymbolIndex::wireJunction + tile.getState() - 1];
        } else {
            return TILE_SYMBOLS[TileSymbolIndex::wireCrossover + (tile.call<tiles::Wire>(&tiles::Wire::getState2) - 1) * 3 + tile.getState() - 1];
        }
    } else if (tile.getType() == tiles::Input::instance()) {
        return {
            TILE_SYMBOLS[TileSymbolIndex::inSwitch + (tileId - TileId::inSwitch) * 2 + tile.getState() - 1].a,
            tile.call<tiles::Input>(&tiles::Input::getKeycode)
        };
    } else if (tile.getType() == tiles::Led::instance()) {
        return TILE_SYMBOLS[TileSymbolIndex::outLed + tile.getState() - 1];
    } else {
        return TILE_SYMBOLS[TileSymbolIndex::gateDiode + (tileId - TileId::gateDiode) * 12 + tile.getDirection() * 3 + tile.getState() - 1];
    }
}

}

void LegacyFileFormat::parseHeader(Board& board, const std::string& line, int lineNumber, HeaderState& state) {
    //std::cout << "Parse line [" << line << "]\n";

    if (line.length() == 0) {
        if (state.lastField == "notes: {") {
            state.notesString.push_back('\n');
        }
    } else if (state.lastField == "headerBegin" && line.find("version:") == 0) {
        state.lastField = "version:";
        state.fileVersion = std::stof(line.substr(state.lastField.length()));
        // Skip validating version as this is expected to already have passed.
    } else if (state.lastField == "version:" && line.find("width:") == 0) {
        state.lastField = "width:";
        int width = std::stoi(line.substr(state.lastField.length()));
        if (width < 0) {
            throw FileStorageError("board dimensions cannot be negative.");
        }
        state.width = width;
    } else if (state.lastField == "width:" && line.find("height:") == 0) {
        state.lastField = "height:";
        int height = std::stoi(line.substr(state.lastField.length()));
        if (height < 0) {
            throw FileStorageError("board dimensions cannot be negative.");
        }
        state.height = height;
    } else if (state.lastField == "height:" && line == "data: {") {
        state.lastField = "data: {";
    } else if (state.lastField == "data: {") {
        if (line.find("extraLogicStates:") == 0) {
            state.extraLogicStates = std::stoi(line.substr(17));
        } else if (line == "}") {
            state.lastField = "data: }";
        } else {
            spdlog::warn("\"{}\" at line {}: found some unrecognized data.", state.filename, lineNumber);
        }
    } else if (state.lastField == "data: }" && line == "notes: {") {
        state.lastField = "notes: {";
    } else if (state.lastField == "notes: {" && line != "}") {
        state.notesString += line + "\n";
    } else if (state.lastField == "notes: {") {
        state.lastField = "headerEnd";

        board.setMaxSize({state.width, state.height});
        board.setExtraLogicStates(state.extraLogicStates);
        board.setNotesString(state.notesString);

    } else {
        throw FileStorageError("unexpected board file data.");
    }
}

void LegacyFileFormat::writeHeader(Board& board, const fs::path& filename, std::ostream& boardFile, float version) {
    std::streamsize defaultPrecision = boardFile.precision();
    boardFile << "version: " << std::fixed << std::setprecision(1) << version << std::defaultfloat << std::setprecision(defaultPrecision) << "\n";
    boardFile << "width: " << board.getMaxSize().x << "\n";
    boardFile << "height: " << board.getMaxSize().y << "\n";
    boardFile << "data: {\n";
    boardFile << "extraLogicStates: " << board.getExtraLogicStates() << "\n";
    boardFile << "}\n";
    boardFile << "notes: {\n";
    boardFile << board.getNotesString().toAnsiString();
    boardFile << "}\n";
    if (!boardFile) {
        throw FileStorageError("file I/O error while writing.", filename);
    }
}

LegacyFileFormat::LegacyFileFormat(const fs::path& filename) :
    FileStorage(filename) {
}

fs::path LegacyFileFormat::getDefaultFileExtension() const {
    return ".txt";
}

bool LegacyFileFormat::validateFileVersion(float version) {
    return version == 1.0;
}

void LegacyFileFormat::loadFromFile(Board& board, const fs::path& filename, fs::ifstream& boardFile) {
    // Reset all members to ensure a clean state.
    setFilename(filename);
    setNewFile(false);

    if (!boardFile.is_open()) {
        throw FileStorageError("unable to open file for reading.", filename);
    }
    boardFile.clear();
    boardFile.seekg(0, std::ios::beg);

    if (symbolLookup.empty()) {
        spdlog::debug("First time init symbolLookup.");
        for (size_t i = 0; i < TILE_SYMBOLS.size(); ++i) {
            symbolLookup.emplace(TILE_SYMBOLS[i], static_cast<unsigned int>(i));
        }
    }

    std::string line;
    int lineNumber = 0;
    ParseState state;
    state.filename = filename;
    try {
        while (state.lastField != "headerEnd" && std::getline(boardFile, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            parseHeader(board, line, ++lineNumber, state);
        }
        while (std::getline(boardFile, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            parseTiles(board, line, ++lineNumber, state);
        }
        if (state.lastField != "done") {
            throw FileStorageError("missing data, end of file reached.");
        }
    } catch (FileStorageError& ex) {
        throw FileStorageError("\"" + filename.string() + "\" at line " + std::to_string(lineNumber) + ": " + ex.what());
    }

    spdlog::debug("Load completed.");
    spdlog::debug("fileVersion = {}", state.fileVersion);
    spdlog::debug("width = {}", state.width);
    spdlog::debug("height = {}", state.height);
    spdlog::debug("extraLogicStates = {}", state.extraLogicStates);
    spdlog::debug("notesString = \"{}\"", state.notesString);
}

void LegacyFileFormat::saveToFile(Board& board) {
    if (getFilename().has_parent_path()) {
        fs::create_directories(getFilename().parent_path());
    }
    fs::ofstream boardFile(getFilename());
    if (!boardFile.is_open()) {
        throw FileStorageError("unable to open file for writing.", getFilename());
    }
    setNewFile(false);
    writeHeader(board, getFilename(), boardFile, 1.0f);

    boardFile << "\n";
    boardFile << std::setfill('*') << std::setw(board.getMaxSize().x * 2 + 2) << "*" << std::setfill(' ') << "\n";
    for (unsigned int y = 0; y < board.getMaxSize().y; ++y) {
        boardFile << "*";
        for (unsigned int x = 0; x < board.getMaxSize().x; ++x) {
            boardFile << tileToSymbol(board, x, y);
        }
        boardFile << "*\n";
    }
    boardFile << std::setfill('*') << std::setw(board.getMaxSize().x * 2 + 2) << "*" << std::setfill(' ') << "\n";
    boardFile.close();
    if (!boardFile) {
        throw FileStorageError("file I/O error while writing.", getFilename());
    }

    spdlog::debug("Save completed.");
}

void LegacyFileFormat::saveAsFile(Board& board, const fs::path& filename) {
    setFilename(filename);
    setNewFile(true);
    saveToFile(board);
}

void LegacyFileFormat::parseTiles(Board& board, const std::string& line, int /*lineNumber*/, ParseState& state) {
    //std::cout << "Parse line [" << line << "]\n";

    if (line.length() == 0) {
        return;
    } else if (state.lastField == "tiles") {
        if (state.y == 0) {
            //std::cout << "========== START OF TILES ==========\n";
        }
        if (line.length() != state.width * 2 + 2) {
            throw FileStorageError("incorrect length of line (expected " + std::to_string(state.width * 2 + 2) + ", got " + std::to_string(line.length()) + ").");
        }
        state.x = 0;
        while (state.x < static_cast<int>(state.width)) {
            TileSymbol symbol(line[state.x * 2 + 1], line[state.x * 2 + 2]);
            setTileFromSymbol(board, state.x, state.y, symbol);
            ++state.x;
        }
        //std::cout << "\n";
        ++state.y;
        if (state.y == static_cast<int>(state.height)) {
            state.lastField = "tilesEnd";
            //std::cout << "========== END OF TILES ==========\n";
        }
    } else if (state.lastField == "headerEnd" || state.lastField == "tilesEnd") {
        if (line.length() != state.width * 2 + 2) {
            throw FileStorageError("incorrect length of line (expected " + std::to_string(state.width * 2 + 2) + ", got " + std::to_string(line.length()) + ").");
        }
        if (state.lastField == "headerEnd") {
            state.lastField = "tiles";
        } else {
            state.lastField = "done";
        }
    } else {
        throw FileStorageError("unexpected board file data.");
    }
}
