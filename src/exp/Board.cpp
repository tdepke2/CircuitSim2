#include <Board.h>
#include <Tile.h>

#include <cassert>
#include <fstream>
#include <iostream>

Board::Board(sf::Texture* tilesetGrid) :
    tilesetGrid_(tilesetGrid) {

    chunks_.emplace(0, Chunk(tilesetGrid->getSize().x, 32));
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
            parseFile(line, lineNumber, parseState);
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

void Board::saveToFile(const std::string& filename) {

}

void Board::parseFile(const std::string& line, int lineNumber, ParseState& parseState) {
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
        while (parseState.x < parseState.width) {
            char c1 = line[parseState.x * 2 + 1], c2 = line[parseState.x * 2 + 2];
            std::cout << c1 << c2 << " ";
            ++parseState.x;
        }
        std::cout << "\n";
        ++parseState.y;
        if (parseState.y == parseState.height) {
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
