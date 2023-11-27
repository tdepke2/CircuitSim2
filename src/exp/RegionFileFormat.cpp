#include <Board.h>
#include <RegionFileFormat.h>

#include <limits>
#include <spdlog/spdlog.h>
#include <stdexcept>

constexpr int RegionFileFormat::REGION_WIDTH;
constexpr int RegionFileFormat::SECTOR_SIZE;

RegionFileFormat::RegionFileFormat() :
    filename_("boards/NewBoard/board.txt"),
    savedRegions_(),
    chunkCacheTimes_(),
    chunkCache_() {
}

bool RegionFileFormat::validateFileVersion(float version) {
    return version == 2.0;
}

void RegionFileFormat::loadFromFile(Board& board, const fs::path& filename, fs::ifstream& inputFile) {
    if (!inputFile.is_open()) {
        throw std::runtime_error("\"" + filename.string() + "\": unable to open file for reading.");
    }
    inputFile.clear();
    inputFile.seekg(0, std::ios::beg);

    std::string line;
    int lineNumber = 0;
    ParseState state;
    state.filename = filename;
    try {
        while (std::getline(inputFile, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            LegacyFileFormat::parseHeader(board, line, ++lineNumber, state);
        }
        while (std::getline(inputFile, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            parseRegionList(board, line, ++lineNumber, state);
        }
        if (state.lastField != "done") {
            throw std::runtime_error("missing data, end of file reached.");
        }
    } catch (std::exception& ex) {
        throw std::runtime_error("\"" + filename.string() + "\" at line " + std::to_string(lineNumber) + ": " + ex.what());
    }

    savedRegions_.clear();
    chunkCacheTimes_.clear();
    chunkCache_.clear();
    for (const auto& coords : state.regions) {
        spdlog::debug("loading region {}, {}", coords.first, coords.second);
        loadRegion(board, coords);
    }
}

void RegionFileFormat::saveToFile(Board& board) {
    if (filename_.has_parent_path()) {
        fs::create_directories(filename_.parent_path());
    }
    fs::ofstream outputFile(filename_);
    if (!outputFile.is_open()) {
        throw std::runtime_error("\"" + filename_.string() + "\": unable to open file for writing.");
    }
    LegacyFileFormat::writeHeader(board, filename_, outputFile, 2.0f);
    outputFile << "regions: {\n";
    for (const auto& region : savedRegions_) {

        // FIXME is this really the union of saved and dirty regions?

        outputFile << region.first.first << "," << region.first.second << "\n";
    }
    outputFile << "}\n";
    outputFile.close();
    fs::create_directory(filename_.parent_path() / "region");

    std::map<RegionCoords, Region> dirtyRegions;

    for (const auto& chunk : board.getLoadedChunks()) {
        if (/*FIXME chunk is dirty*/ true) {
            auto& region = dirtyRegions[toRegionCoords(chunk.first)];
            region.insert(chunk.first);
        }
    }

    spdlog::debug("save file: {}", filename_);
    spdlog::debug("dirtyRegions:");
    for (auto& region : dirtyRegions) {
        spdlog::debug("  ({}, {}) ->", region.first.first, region.first.second);
        for (auto& coords : region.second) {
            spdlog::debug("    {}, {}", ChunkCoords::x(coords), ChunkCoords::y(coords));
        }
    }

    //for (const auto& region : dirtyRegions) {
    //    saveRegion(board, region.first, region.second);
    //}
    const auto region = dirtyRegions.find({0, 0});
    saveRegion(board, region->first, region->second);
}

constexpr int constLog2(int x) {
    return x == 1 ? 0 : 1 + constLog2(x / 2);
}

RegionFileFormat::RegionCoords RegionFileFormat::toRegionCoords(ChunkCoords::repr chunkCoords) {
    constexpr int widthLog2 = constLog2(REGION_WIDTH);
    return {
        ChunkCoords::x(chunkCoords) >> widthLog2,
        ChunkCoords::y(chunkCoords) >> widthLog2
    };
}

std::pair<int, int> RegionFileFormat::toRegionOffset(ChunkCoords::repr chunkCoords) {
    return {
        ChunkCoords::x(chunkCoords) & (REGION_WIDTH - 1),
        ChunkCoords::y(chunkCoords) & (REGION_WIDTH - 1)
    };
}

void RegionFileFormat::loadChunk(Board& board, ChunkCoords::repr chunkCoords) const {
    const auto coords = toRegionCoords(chunkCoords);
    auto region = savedRegions_.find(coords);
    if (region == savedRegions_.end() || region->second.count(chunkCoords) == 0) {
        return;
    }

    auto cachedChunk = chunkCache_.find(chunkCoords);
    if (cachedChunk != chunkCache_.end()) {
        // FIXME found chunk, set it in board.
        return;
    }

    fs::path regionFilename = std::to_string(coords.first) + "." + std::to_string(coords.second) + ".dat";
    regionFilename = filename_.parent_path() / "region" / regionFilename;
    fs::ifstream regionFile(regionFilename, std::ios::binary);
    if (!regionFile.is_open()) {
        throw std::runtime_error("\"" + regionFilename.string() + "\": unable to open file for reading.");
    }
    ChunkHeader header[REGION_WIDTH * REGION_WIDTH];
    readRegionHeader(header, regionFilename, regionFile);

    constexpr int CACHE_LOAD_WIDTH = 4;
    constexpr int CACHE_SIZE = CACHE_LOAD_WIDTH * CACHE_LOAD_WIDTH * 4;

    const auto regionOffset = toRegionOffset(chunkCoords);
    int xStart = regionOffset.first / CACHE_LOAD_WIDTH;
    int yStart = regionOffset.second / CACHE_LOAD_WIDTH;
    for (int x = xStart; x < xStart + CACHE_LOAD_WIDTH; ++x) {
        for (int y = yStart; y < yStart + CACHE_LOAD_WIDTH; ++y) {


            // FIXME need to load the chunk data and set it in board.
            // also would be nice to refactor inputFile/outputFile to something clearer.


        }
    }
    regionFile.close();

    // FIXME need to work on cases for cache invalidation!
}

void RegionFileFormat::parseRegionList(Board& /*board*/, const std::string& line, int /*lineNumber*/, ParseState& state) {
    if (line.length() == 0) {
        return;
    } else if (state.lastField == "headerEnd" && line == "regions: {") {
        state.lastField = "regions: {";
    } else if (state.lastField == "regions: {") {
        if (line == "}") {
            state.lastField = "done";
        } else {
            if (line.find(',') == std::string::npos) {
                throw std::runtime_error("expected comma separator.");
            }
            int x = std::stoi(line.substr(0, line.find(',')));
            int y = std::stoi(line.substr(line.find(',') + 1));
            state.regions.emplace(x, y);
        }
    } else {
        throw std::runtime_error("unexpected board file data.");
    }
}

void RegionFileFormat::readRegionHeader(ChunkHeader header[], const fs::path& /*filename*/, fs::ifstream& regionFile) {
    for (int i = 0; i < REGION_WIDTH * REGION_WIDTH; ++i) {
        uint32_t offset;
        regionFile.read(reinterpret_cast<char*>(&offset), 3);
        header[i].offset = byteswap(offset) >> 8;
        uint8_t sectors;
        regionFile.read(reinterpret_cast<char*>(&sectors), 1);
        header[i].sectors = sectors;
    }
}

void RegionFileFormat::loadRegion(Board& board, const RegionCoords& coords) {
    fs::path regionFilename = std::to_string(coords.first) + "." + std::to_string(coords.second) + ".dat";
    regionFilename = filename_.parent_path() / "region" / regionFilename;
    fs::ifstream regionFile(regionFilename, std::ios::binary);
    if (!regionFile.is_open()) {
        throw std::runtime_error("\"" + regionFilename.string() + "\": unable to open file for reading.");
    }

    ChunkHeader header[REGION_WIDTH * REGION_WIDTH];
    readRegionHeader(header, regionFilename, regionFile);
    for (int i = 0; i < REGION_WIDTH * REGION_WIDTH; ++i) {
        if (header[i].sectors > 0) {
            savedRegions_[coords].insert(ChunkCoords::pack(i % REGION_WIDTH + coords.first * REGION_WIDTH, i / REGION_WIDTH + coords.second * REGION_WIDTH));
        }
    }
    regionFile.close();
}

void RegionFileFormat::saveRegion(Board& board, const RegionCoords& coords, const Region& region) {
    fs::path regionFilename = std::to_string(coords.first) + "." + std::to_string(coords.second) + ".dat";
    regionFilename = filename_.parent_path() / "region" / regionFilename;
    fs::ofstream regionFile(regionFilename, std::ios::binary);
    if (!regionFile.is_open()) {
        throw std::runtime_error("\"" + regionFilename.string() + "\": unable to open file for reading/writing.");
    }

    /*
    // FIXME this may not work unless we open in read mode first...

    // Determine number of bytes we can read from the file, should match the file size in most cases.
    // https://stackoverflow.com/questions/22984956/tellg-function-give-wrong-size-of-file/22986486#22986486
    regionFile.ignore(std::numeric_limits<std::streamsize>::max());
    std::streamsize readLength = regionFile.gcount();
    regionFile.clear();
    regionFile.seekg(0, std::ios::beg);

    spdlog::debug("readLength = {}", readLength);
    */

    const char emptySector[SECTOR_SIZE] = {};
    ChunkHeader header[REGION_WIDTH * REGION_WIDTH] = {};

    std::map<ChunkCoords::repr, std::vector<char>> serializedChunks;
    for (const auto& chunkCoords : region) {
        serializedChunks.emplace(chunkCoords, board.getLoadedChunks().at(chunkCoords).serialize());
    }

    // Write empty header.
    uint32_t lastOffset = 4 * REGION_WIDTH * REGION_WIDTH / SECTOR_SIZE;
    for (unsigned int i = 0; i < lastOffset; ++i) {
        regionFile.write(emptySector, SECTOR_SIZE);
    }

    // Write chunks.
    for (const auto& serialized : serializedChunks) {
        const auto regionOffset = toRegionOffset(serialized.first);
        const int headerIndex = regionOffset.first + regionOffset.second * REGION_WIDTH;
        const uint32_t serializedSize = serialized.second.size() + 4;
        header[headerIndex].offset = lastOffset;
        if (serializedSize > std::numeric_limits<uint8_t>::max() * static_cast<unsigned int>(SECTOR_SIZE)) {
            // FIXME unable to save this chunk, too much data
            spdlog::error("Failed to save chunk at {}, {} (serialized to {} bytes)", ChunkCoords::x(serialized.first), ChunkCoords::y(serialized.first), serialized.second.size());
        }
        header[headerIndex].sectors = (serializedSize + SECTOR_SIZE - 1) / SECTOR_SIZE;
        lastOffset += header[headerIndex].sectors;

        spdlog::debug("Writing chunk {}, {}", ChunkCoords::x(serialized.first), ChunkCoords::y(serialized.first));
        board.getLoadedChunks().at(serialized.first).debugPrintChunk();

        auto serializedSizeSwapped = byteswap(serializedSize);
        regionFile.write(reinterpret_cast<char*>(&serializedSizeSwapped), sizeof(serializedSizeSwapped));
        regionFile.write(serialized.second.data(), serialized.second.size());
        const auto paddingBytes = SECTOR_SIZE - (serializedSize - 1) % SECTOR_SIZE - 1;
        spdlog::debug("Writing {} extra padding bytes", paddingBytes);
        regionFile.write(emptySector, paddingBytes);
    }

    // Write (filled) header.
    regionFile.seekp(0, std::ios::beg);
    for (int i = 0; i < REGION_WIDTH * REGION_WIDTH; ++i) {
        uint32_t offset = byteswap(header[i].offset << 8);
        regionFile.write(reinterpret_cast<char*>(&offset), 3);
        uint8_t sectors = header[i].sectors;
        regionFile.write(reinterpret_cast<char*>(&sectors), 1);
    }

    regionFile.close();
}

/*

read header...

serialize chunks into memory

header exists and no existing chunks need more memory?
    write new/update existing chunks and replace header
else
    header exists?
        read all chunks into memory
    write header and chunks

clean up dirty chunks in board that are empty
*/
