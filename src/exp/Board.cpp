#include <Board.h>
#include <ResourceManager.h>
#include <Tile.h>
#include <tiles/Gate.h>
#include <tiles/Input.h>
#include <tiles/Led.h>
#include <tiles/Wire.h>

#include <array>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <spdlog/spdlog.h>

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



sf::Texture* Board::tilesetGrid_;
sf::Texture* Board::tilesetNoGrid_;
unsigned int Board::tileWidth_;

void clampImageToEdge(sf::Image& image, const sf::Vector2u& topLeft, const sf::Vector2u& bottomRight, const sf::Vector2u& borderSize) {
    sf::Vector2u borderTopLeft(topLeft - borderSize), borderBottomRight(bottomRight + borderSize);
    for (unsigned int y = borderTopLeft.y; y < borderBottomRight.y; ++y) {
        for (unsigned int x = borderTopLeft.x; x < borderBottomRight.x; ++x) {
            if (x < topLeft.x || x >= bottomRight.x || y < topLeft.y || y >= bottomRight.y) {
                unsigned int xTarget = std::min(std::max(x, topLeft.x), bottomRight.x - 1);
                unsigned int yTarget = std::min(std::max(y, topLeft.y), bottomRight.y - 1);
                image.setPixel(x, y, image.getPixel(xTarget, yTarget));
            }
        }
    }
}

void loadTileset(const std::string& filename, sf::Texture* target, unsigned int tileWidth) {
    sf::Image tileset;
    if (!tileset.loadFromFile(filename)) {
        std::cerr << "Failed to load texture file.\n";
        exit(-1);
    }
    target->create(tileset.getSize().x * 2, tileset.getSize().y * 4);
    sf::Image fullTileset;
    fullTileset.create(tileset.getSize().x * 2, tileset.getSize().y * 2, sf::Color::Red);

    for (unsigned int y = 0; y < tileset.getSize().y; y += tileWidth) {
        for (unsigned int x = 0; x < tileset.getSize().x; x += tileWidth) {
            sf::Vector2u tileTopLeft(x * 2 + tileWidth / 2, y * 2 + tileWidth / 2);
            fullTileset.copy(tileset, tileTopLeft.x, tileTopLeft.y, sf::IntRect(x, y, tileWidth, tileWidth));
            clampImageToEdge(fullTileset, tileTopLeft, tileTopLeft + sf::Vector2u(tileWidth, tileWidth), sf::Vector2u(tileWidth / 2, tileWidth / 2));
        }
    }
    target->update(fullTileset, 0, 0);

    for (unsigned int y = 0; y < fullTileset.getSize().y; ++y) {
        for (unsigned int x = 0; x < fullTileset.getSize().x; ++x) {
            fullTileset.setPixel(x, y, fullTileset.getPixel(x, y) + sf::Color(100, 100, 100));
        }
    }
    target->update(fullTileset, 0, tileset.getSize().y * 2);
    std::cout << "Built tileset texture with size " << target->getSize().x << " x " << target->getSize().y << "\n";
}

void Board::setupTextures(ResourceManager& resource, const std::string& filenameGrid, const std::string& filenameNoGrid, unsigned int tileWidth) {
    tileWidth_ = tileWidth;

    tilesetGrid_ = &resource.getTexture(filenameGrid, true);
    loadTileset(filenameGrid, tilesetGrid_, tileWidth);
    tilesetGrid_->setSmooth(true);
    if (!tilesetGrid_->generateMipmap()) {
        std::cerr << "Warn: \"" << filenameGrid << "\": Unable to generate mipmap for texture.\n";
    }

    tilesetNoGrid_ = &resource.getTexture(filenameNoGrid, true);
    loadTileset(filenameNoGrid, tilesetNoGrid_, tileWidth);
    tilesetNoGrid_->setSmooth(true);
    if (!tilesetNoGrid_->generateMipmap()) {
        std::cerr << "Warn: \"" << filenameNoGrid << "\": Unable to generate mipmap for texture.\n";
    }

    Chunk::setupTextureData(tilesetGrid_->getSize(), tileWidth);
}

Board::Board() :    // FIXME we really should be doing member initialization list for all members (needs to be fixed in other classes).
    maxSize_(),
    chunks_(),
    currentView_(),
    currentZoom_(0.0f),
    chunkRenderCache_(LEVELS_OF_DETAIL),
    emptyChunk_(),
    debugChunkBorder_(sf::Lines),
    debugDrawChunkBorder_(false),
    debugChunksDrawn_(0) {
}

void Board::setRenderArea(const sf::View& view, float zoom, DebugScreen& debugScreen) {
    currentView_ = view;
    currentZoom_ = zoom;

    int currentLod = static_cast<int>(std::max(std::floor(std::log2(currentZoom_)), 0.0f));
    debugScreen.getField("lod").setString(fmt::format("Lod: {}", currentLod));

    sf::Vector2f lodMaxView = view.getSize() / currentZoom_ * static_cast<float>(1 << (currentLod + 1));
    float x = std::ceil(std::round(lodMaxView.x) / (Chunk::WIDTH * static_cast<int>(tileWidth_))) + 1.0f;
    float y = std::ceil(std::round(lodMaxView.y) / (Chunk::WIDTH * static_cast<int>(tileWidth_))) + 1.0f;
    int x2 = 1 << static_cast<int>(std::ceil(std::log2(x)));
    int y2 = 1 << static_cast<int>(std::ceil(std::log2(y)));

    // this calculation still needs work, it should only be applied to the first lod level and shouldn't change from zooming.

    debugScreen.getField("lodRange").setString(fmt::format("Range: {}, {} (rounded up to {}, {})", x, y, x2, y2));
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

        /*std::cout << "symbolLookup contents:\n";
        for (const auto& x : symbolLookup) {
            std::cout << x.first.a << x.first.b << " -> " << x.second << "\n";
        }
        std::cout << "\n";*/
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

void Board::debugSetDrawChunkBorder(bool enabled) {
    debugDrawChunkBorder_ = enabled;
}

unsigned int Board::debugGetChunksDrawn() const {
    return debugChunksDrawn_;
}

void Board::parseFile(const std::string& line, int lineNumber, ParseState& parseState, const std::map<TileSymbol, unsigned int>& symbolLookup) {
    //std::cout << "Parse line [" << line << "]\n";

    if (line.length() == 0) {
        if (parseState.lastField == "notes: {") {
            parseState.notesString.push_back('\n');
        }
    } else if (parseState.lastField == "tiles") {
        if (parseState.y == 0) {
            //std::cout << "========== START OF TILES ==========\n";
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
            //std::cout << symbolId << " ";
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
        //std::cout << "\n";
        ++parseState.y;
        if (parseState.y == static_cast<int>(parseState.height)) {
            parseState.lastField = "tilesEnd";
            //std::cout << "========== END OF TILES ==========\n";
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
        } else {
            maxSize_ = {0, 0};
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
        spdlog::debug("Allocating new chunk at ({}, {})", x / Chunk::WIDTH, y / Chunk::WIDTH);
        return chunks_.emplace(mapIndex, Chunk()).first->second;
    }
}

void Board::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    states.transform *= getTransform();


    sf::Vector2i topLeft = {
        static_cast<int>(std::floor((currentView_.getCenter().x - currentView_.getSize().x / 2.0f) / (Chunk::WIDTH * tileWidth_))),
        static_cast<int>(std::floor((currentView_.getCenter().y - currentView_.getSize().y / 2.0f) / (Chunk::WIDTH * tileWidth_)))
    };
    sf::Vector2i bottomRight = {
        static_cast<int>(std::floor((currentView_.getCenter().x + currentView_.getSize().x / 2.0f) / (Chunk::WIDTH * tileWidth_))),
        static_cast<int>(std::floor((currentView_.getCenter().y + currentView_.getSize().y / 2.0f) / (Chunk::WIDTH * tileWidth_)))
    };
    //std::cout << bottomRight.x << "    " << bottomRight.y << "\n";
    int totalDrawn = 0;
    int totalEmpty = 0;

    sf::Transform originalTransform = states.transform;

    states.texture = tilesetGrid_;
    debugChunksDrawn_ = 0;
    for (int y = topLeft.y; y <= bottomRight.y; ++y) {
        for (int x = topLeft.x; x <= bottomRight.x; ++x) {
            states.transform = sf::Transform(originalTransform).translate(
                static_cast<float>(x * Chunk::WIDTH * static_cast<int>(tileWidth_)),
                static_cast<float>(y * Chunk::WIDTH * static_cast<int>(tileWidth_))
            );
            auto chunk = chunks_.find(static_cast<uint64_t>(y) << 32 | static_cast<uint32_t>(x));
            if (chunk != chunks_.end()) {
                target.draw(chunk->second, states);
            } else {
                target.draw(emptyChunk_, states);
                ++totalEmpty;
            }
            // FIXME faster to instead walk the iterator in the inner loop? we would need to assume there are holes, so walk one at a time and read the chunk if it lines up with index. #########################
            ++debugChunksDrawn_;
        }
    }
    states.texture = nullptr;
    states.transform = originalTransform;

    //states.texture = tilesetGrid_;
    //target.draw(emptyChunk_, states);
    //states.transform = states.transform.translate(1 * Chunk::WIDTH * tileWidth_, 1 * Chunk::WIDTH * tileWidth_);
    //target.draw(chunks_.at(0), states);

    //std::cout << "d: " << totalDrawn << " e: " << totalEmpty << "\n";

    if (debugDrawChunkBorder_) {
        debugChunkBorder_.resize((bottomRight.x - topLeft.x + 1) * (bottomRight.y - topLeft.y + 1) * 4);
        const int chunkDistance = Chunk::WIDTH * static_cast<int>(tileWidth_);
        unsigned int i = 0;
        for (int y = topLeft.y; y <= bottomRight.y; ++y) {
            float yChunkPos = static_cast<float>(y * chunkDistance);
            for (int x = topLeft.x; x <= bottomRight.x; ++x) {
                float xChunkPos = static_cast<float>(x * chunkDistance);
                sf::Color borderColor = sf::Color::Blue;
                if (chunks_.find(static_cast<uint64_t>(y) << 32 | static_cast<uint32_t>(x)) != chunks_.end()) {
                    borderColor = sf::Color::Yellow;
                }

                debugChunkBorder_[i + 0].position = {xChunkPos, yChunkPos};
                debugChunkBorder_[i + 0].color = borderColor;
                debugChunkBorder_[i + 1].position = {xChunkPos + chunkDistance, yChunkPos};
                debugChunkBorder_[i + 1].color = borderColor;
                debugChunkBorder_[i + 2].position = {xChunkPos, yChunkPos};
                debugChunkBorder_[i + 2].color = borderColor;
                debugChunkBorder_[i + 3].position = {xChunkPos, yChunkPos + chunkDistance};
                debugChunkBorder_[i + 3].color = borderColor;
                i += 4;
            }
        }
        target.draw(debugChunkBorder_, states);
    }
}
