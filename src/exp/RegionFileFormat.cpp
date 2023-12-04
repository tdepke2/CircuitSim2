#include <Board.h>
#include <RegionFileFormat.h>

#include <limits>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <tuple>

constexpr int RegionFileFormat::REGION_WIDTH;
constexpr int RegionFileFormat::SECTOR_SIZE;

RegionFileFormat::RegionFileFormat() :
    filename_("boards/NewBoard/board.txt"),
    savedRegions_(),
    lastVisibleChunks_(0, 0, 0, 0),
    chunkCache_(),
    chunkCacheTimes_() {
}

bool RegionFileFormat::validateFileVersion(float version) {
    return version == 2.0;
}

void RegionFileFormat::loadFromFile(Board& board, const fs::path& filename, fs::ifstream& boardFile) {
    if (!boardFile.is_open()) {
        throw std::runtime_error("\"" + filename.string() + "\": unable to open file for reading.");
    }
    boardFile.clear();
    boardFile.seekg(0, std::ios::beg);

    std::string line;
    int lineNumber = 0;
    ParseState state;
    state.filename = filename;
    try {
        while (state.lastField != "headerEnd" && std::getline(boardFile, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            LegacyFileFormat::parseHeader(board, line, ++lineNumber, state);
        }
        while (std::getline(boardFile, line)) {
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
    chunkCache_.clear();
    chunkCacheTimes_.clear();
    for (const auto& regionCoords : state.regions) {
        spdlog::debug("loading region {}, {}", regionCoords.first, regionCoords.second);
        loadRegion(board, regionCoords);
    }
}

void RegionFileFormat::saveToFile(Board& board) {
    std::map<RegionCoords, Region> unsavedRegions;

    for (const auto& chunk : board.getLoadedChunks()) {
        const auto savedRegion = savedRegions_.find(toRegionCoords(chunk.first));
        if (savedRegion == savedRegions_.end() || savedRegion->second.count(chunk.first) == 0 || chunk.second.isUnsaved()) {
            unsavedRegions[toRegionCoords(chunk.first)].insert(chunk.first);
        }
    }

    spdlog::debug("save file: {}", filename_);
    spdlog::debug("unsavedRegions:");
    for (auto& region : unsavedRegions) {
        spdlog::debug("  ({}, {}) ->", region.first.first, region.first.second);
        for (auto& chunkCoords : region.second) {
            spdlog::debug("    {}, {}", ChunkCoords::x(chunkCoords), ChunkCoords::y(chunkCoords));
        }
    }
    if (unsavedRegions.empty()) {
        spdlog::debug("No regions to save.");
        return;
    }

    fs::create_directory(filename_.parent_path() / "region");
    //for (const auto& region : unsavedRegions) {
    //    saveRegion(board, region.first, region.second);
    //}
    {
        const auto region = unsavedRegions.find({0, 0});
        saveRegion(board, region->first, region->second);
    }

    if (filename_.has_parent_path()) {
        fs::create_directories(filename_.parent_path());
    }
    fs::ofstream boardFile(filename_);
    if (!boardFile.is_open()) {
        throw std::runtime_error("\"" + filename_.string() + "\": unable to open file for writing.");
    }
    LegacyFileFormat::writeHeader(board, filename_, boardFile, 2.0f);
    boardFile << "regions: {\n";
    for (const auto& region : savedRegions_) {
        boardFile << region.first.first << "," << region.first.second << "\n";
    }
    boardFile << "}\n";
    boardFile.close();
}

void RegionFileFormat::updateVisibleChunks(Board& board, const ChunkCoordsRange& visibleChunks) {
    if (visibleChunks == lastVisibleChunks_) {
        return;
    }

    for (int y = visibleChunks.top; y < visibleChunks.top + visibleChunks.height; ++y) {
        for (int x = visibleChunks.left; x < visibleChunks.left + visibleChunks.width; ++x) {
            if (!lastVisibleChunks_.contains(x, y)) {
                loadChunk(board, ChunkCoords::pack(x, y));
            }
        }
    }

    lastVisibleChunks_ = visibleChunks;
}

bool RegionFileFormat::loadChunk(Board& board, ChunkCoords::repr chunkCoords) {
    spdlog::debug("Checking chunk {}, {} for load.", ChunkCoords::x(chunkCoords), ChunkCoords::y(chunkCoords));
    const auto regionCoords = toRegionCoords(chunkCoords);
    auto region = savedRegions_.find(regionCoords);
    if (region == savedRegions_.end() || region->second.count(chunkCoords) == 0 || board.isChunkLoaded(chunkCoords)) {
        return false;
    }

    auto cachedChunk = chunkCache_.find(chunkCoords);
    if (cachedChunk != chunkCache_.end()) {
        spdlog::debug("Loading cached chunk {}, {}.", ChunkCoords::x(chunkCoords), ChunkCoords::y(chunkCoords));
        board.loadChunk(std::move(cachedChunk->second));
        chunkCache_.erase(cachedChunk);
        chunkCacheTimes_.erase(chunkCoords);
        return true;
    }

    fs::path regionFilename = std::to_string(regionCoords.first) + "." + std::to_string(regionCoords.second) + ".dat";
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
    int xStart = (regionOffset.first / CACHE_LOAD_WIDTH) * CACHE_LOAD_WIDTH;
    int yStart = (regionOffset.second / CACHE_LOAD_WIDTH) * CACHE_LOAD_WIDTH;
    for (int y = yStart; y < yStart + CACHE_LOAD_WIDTH; ++y) {
        for (int x = xStart; x < xStart + CACHE_LOAD_WIDTH; ++x) {
            const int headerIndex = x + y * REGION_WIDTH;
            const ChunkCoords::repr cacheChunkCoords = ChunkCoords::pack(x + regionCoords.first * REGION_WIDTH, y + regionCoords.second * REGION_WIDTH);
            if (header[headerIndex].sectors > 0 && chunkCacheTimes_.count(cacheChunkCoords) == 0) {
                // Remove chunk from the cache if full to make more space.
                if (chunkCache_.size() >= CACHE_SIZE) {
                    auto minCacheTime = chunkCacheTimes_.begin();
                    for (auto chunkCacheTime = chunkCacheTimes_.begin(); chunkCacheTime != chunkCacheTimes_.end(); ++chunkCacheTime) {
                        if (chunkCacheTime->second < minCacheTime->second) {
                            minCacheTime = chunkCacheTime;
                        }
                    }
                    spdlog::debug(
                        "Removing cached chunk at {}, {} (caching would exceed {} chunks).",
                        ChunkCoords::x(minCacheTime->first), ChunkCoords::y(minCacheTime->first), CACHE_SIZE
                    );
                    chunkCache_.erase(minCacheTime->first);
                    chunkCacheTimes_.erase(minCacheTime);
                }

                spdlog::debug(
                    "Caching chunk {}, {} while loading chunk at {}, {}.",
                    ChunkCoords::x(cacheChunkCoords), ChunkCoords::y(cacheChunkCoords),
                    ChunkCoords::x(chunkCoords), ChunkCoords::y(chunkCoords)
                );

                auto chunk = chunkCache_.emplace(std::piecewise_construct, std::forward_as_tuple(cacheChunkCoords), std::forward_as_tuple(nullptr, cacheChunkCoords)).first;
                chunkCacheTimes_.emplace(cacheChunkCoords, std::chrono::steady_clock::now());
                chunk->second.deserialize(readChunk(header, headerIndex, regionFilename, regionFile));
            }
        }
    }
    regionFile.close();

    cachedChunk = chunkCache_.find(chunkCoords);
    board.loadChunk(std::move(cachedChunk->second));
    chunkCache_.erase(cachedChunk);
    chunkCacheTimes_.erase(chunkCoords);

    // FIXME need to work on cases for cache invalidation!

    return true;
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

void RegionFileFormat::readRegionHeader(ChunkHeader header[], const fs::path& /*filename*/, std::istream& regionFile) {
    for (int i = 0; i < REGION_WIDTH * REGION_WIDTH; ++i) {
        uint32_t offset;
        regionFile.read(reinterpret_cast<char*>(&offset), 3);
        header[i].offset = byteswap(offset) >> 8;
        uint8_t sectors;
        regionFile.read(reinterpret_cast<char*>(&sectors), 1);
        header[i].sectors = sectors;
    }
}

void RegionFileFormat::writeRegionHeader(ChunkHeader header[], const fs::path& /*filename*/, std::ostream& regionFile) {
    regionFile.seekp(0, std::ios::beg);
    for (int i = 0; i < REGION_WIDTH * REGION_WIDTH; ++i) {
        uint32_t offset = byteswap(header[i].offset << 8);
        regionFile.write(reinterpret_cast<char*>(&offset), 3);
        uint8_t sectors = header[i].sectors;
        regionFile.write(reinterpret_cast<char*>(&sectors), 1);
    }
}

std::vector<char> RegionFileFormat::readChunk(ChunkHeader header[], int headerIndex, const fs::path& /*filename*/, std::istream& regionFile) {
    regionFile.seekg(header[headerIndex].offset * SECTOR_SIZE, std::ios::beg);
    uint32_t chunkSize;
    regionFile.read(reinterpret_cast<char*>(&chunkSize), sizeof(chunkSize));
    chunkSize = byteswap(chunkSize) - sizeof(chunkSize);
    std::vector<char> chunkData(chunkSize);
    regionFile.read(chunkData.data(), chunkSize);
    return chunkData;
}

void RegionFileFormat::writeChunk(ChunkHeader header[], int headerIndex, const char emptySector[], uint32_t& lastOffset, const std::vector<char>& chunkData, const fs::path& /*filename*/, std::ostream& regionFile) {
    const uint32_t serializedSize = chunkData.size() + sizeof(serializedSize);
    const unsigned int sectorCount = (serializedSize + SECTOR_SIZE - 1) / SECTOR_SIZE;
    if (header[headerIndex].sectors > 0) {
        assert(header[headerIndex].sectors >= sectorCount);
        regionFile.seekp(header[headerIndex].offset * SECTOR_SIZE, std::ios::beg);
    } else {
        header[headerIndex].offset = lastOffset;
        header[headerIndex].sectors = sectorCount;
        regionFile.seekp(lastOffset * SECTOR_SIZE, std::ios::beg);
        lastOffset += header[headerIndex].sectors;
    }

    auto serializedSizeSwapped = byteswap(serializedSize);
    regionFile.write(reinterpret_cast<char*>(&serializedSizeSwapped), sizeof(serializedSizeSwapped));
    regionFile.write(chunkData.data(), chunkData.size());
    const auto paddingBytes = SECTOR_SIZE - (serializedSize - 1) % SECTOR_SIZE - 1;
    spdlog::debug("Writing {} extra padding bytes", paddingBytes);
    regionFile.write(emptySector, paddingBytes);
}

void RegionFileFormat::loadRegion(Board& /*board*/, const RegionCoords& regionCoords) {
    fs::path regionFilename = std::to_string(regionCoords.first) + "." + std::to_string(regionCoords.second) + ".dat";
    regionFilename = filename_.parent_path() / "region" / regionFilename;
    fs::ifstream regionFile(regionFilename, std::ios::binary);
    if (!regionFile.is_open()) {
        throw std::runtime_error("\"" + regionFilename.string() + "\": unable to open file for reading.");
    }

    ChunkHeader header[REGION_WIDTH * REGION_WIDTH];
    readRegionHeader(header, regionFilename, regionFile);
    for (int i = 0; i < REGION_WIDTH * REGION_WIDTH; ++i) {
        if (header[i].sectors > 0) {
            savedRegions_[regionCoords].insert(ChunkCoords::pack(i % REGION_WIDTH + regionCoords.first * REGION_WIDTH, i / REGION_WIDTH + regionCoords.second * REGION_WIDTH));
        }
    }
    regionFile.close();
}

void RegionFileFormat::saveRegion(Board& board, const RegionCoords& regionCoords, const Region& region) {
    fs::path regionFilename = std::to_string(regionCoords.first) + "." + std::to_string(regionCoords.second) + ".dat";
    regionFilename = filename_.parent_path() / "region" / regionFilename;
    fs::fstream regionFile(regionFilename, std::ios::in | std::ios::out | std::ios::binary);
    spdlog::debug("Saving chunks for region {}, {}.", regionCoords.first, regionCoords.second);

    ChunkHeader header[REGION_WIDTH * REGION_WIDTH] = {};
    std::streamsize regionFileLength = 0;
    if (regionFile.is_open()) {
        // Determine number of bytes we can read from the file, should match the file size in most cases.
        // https://stackoverflow.com/questions/22984956/tellg-function-give-wrong-size-of-file/22986486#22986486
        regionFile.ignore(std::numeric_limits<std::streamsize>::max());
        regionFileLength = regionFile.gcount();
        regionFile.clear();
        regionFile.seekg(0, std::ios::beg);
        if (regionFileLength < 4 * REGION_WIDTH * REGION_WIDTH) {
            throw std::runtime_error(
                "\"" + regionFilename.string() + "\": binary file with " + std::to_string(regionFileLength) +
                " bytes is less than minimum size of " + std::to_string(4 * REGION_WIDTH * REGION_WIDTH) + " bytes."
            );
        }
        if (regionFileLength % SECTOR_SIZE != 0) {
            throw std::runtime_error(
                "\"" + regionFilename.string() + "\": binary file with " + std::to_string(regionFileLength) +
                " bytes is not evenly divisible by " + std::to_string(SECTOR_SIZE) + " byte sectors."
            );
        }

        spdlog::debug("Found existing region file with {} bytes.", regionFileLength);
        readRegionHeader(header, regionFilename, regionFile);
    } else {
        regionFile.open(regionFilename, std::ios::out | std::ios::binary);
        if (!regionFile.is_open()) {
            throw std::runtime_error("\"" + regionFilename.string() + "\": unable to open file for writing.");
        }
    }
    regionFile.seekp(0, std::ios::beg);

    bool reallocationRequired = false;
    std::map<ChunkCoords::repr, std::vector<char>> serializedChunks;
    for (const auto& chunkCoords : region) {
        const auto serialized = serializedChunks.emplace(chunkCoords, board.getLoadedChunks().at(chunkCoords).serialize()).first;
        if (regionFileLength > 0) {
            const auto regionOffset = toRegionOffset(chunkCoords);
            const int headerIndex = regionOffset.first + regionOffset.second * REGION_WIDTH;
            const uint32_t serializedSize = serialized->second.size() + sizeof(serializedSize);
            const unsigned int sectorCount = (serializedSize + SECTOR_SIZE - 1) / SECTOR_SIZE;
            if (header[headerIndex].sectors > 0) {
                if (serialized->second.empty()) {
                    spdlog::debug("Chunk {}, {} requires 0 sectors, removing.", ChunkCoords::x(chunkCoords), ChunkCoords::y(chunkCoords));
                    reallocationRequired = true;
                } else if (header[headerIndex].sectors < sectorCount) {
                    const unsigned int currentSectors = header[headerIndex].sectors;
                    spdlog::debug("Chunk {}, {} requires {} sectors but only {} are allocated.", ChunkCoords::x(chunkCoords), ChunkCoords::y(chunkCoords), sectorCount, currentSectors);
                    reallocationRequired = true;
                }
            }
        }
    }

    const char emptySector[SECTOR_SIZE] = {};
    uint32_t lastOffset = static_cast<uint32_t>(regionFileLength / SECTOR_SIZE);
    if (reallocationRequired) {
        // Read in all chunks and wipe header.
        spdlog::debug("Preparing all chunks for reallocation.");
        for (int i = 0; i < REGION_WIDTH * REGION_WIDTH; ++i) {
            if (header[i].sectors > 0) {
                serializedChunks.emplace(
                    ChunkCoords::pack(i % REGION_WIDTH + regionCoords.first * REGION_WIDTH, i / REGION_WIDTH + regionCoords.second * REGION_WIDTH),
                    readChunk(header, i, regionFilename, regionFile)
                );
            }
            header[i].offset = 0;
            header[i].sectors = 0;
        }
    } else if (lastOffset == 0) {
        // Write empty header.
        lastOffset = 4 * REGION_WIDTH * REGION_WIDTH / SECTOR_SIZE;
        for (unsigned int i = 0; i < lastOffset; ++i) {
            regionFile.write(emptySector, SECTOR_SIZE);
        }
    }

    // Write chunks.
    for (const auto& serialized : serializedChunks) {
        const auto regionOffset = toRegionOffset(serialized.first);
        const int headerIndex = regionOffset.first + regionOffset.second * REGION_WIDTH;
        const uint32_t serializedSize = serialized.second.size() + sizeof(serializedSize);
        if (serialized.second.empty()) {
            savedRegions_[regionCoords].erase(serialized.first);
            continue;
        } else if (serializedSize <= std::numeric_limits<uint8_t>::max() * static_cast<unsigned int>(SECTOR_SIZE)) {
            savedRegions_[regionCoords].insert(serialized.first);
        } else {
            // FIXME unable to save this chunk, too much data
            spdlog::error("Failed to save chunk at {}, {} (serialized to {} bytes)", ChunkCoords::x(serialized.first), ChunkCoords::y(serialized.first), serialized.second.size());
            continue;
        }

        spdlog::debug("Writing chunk {}, {}", ChunkCoords::x(serialized.first), ChunkCoords::y(serialized.first));
        board.getLoadedChunks().at(serialized.first).debugPrintChunk();
        writeChunk(header, headerIndex, emptySector, lastOffset, serialized.second, regionFilename, regionFile);
    }

    // Write (filled) header.
    writeRegionHeader(header, regionFilename, regionFile);

    regionFile.close();

    // FIXME need to clean up empty chunks (or should we do this after all regions have been saved?).


}

/*

read header...

serialize chunks into memory

existing chunks need more memory?
    read all chunks into memory
    wipe header

write chunks
write header
clean up dirty chunks in board that are empty

saving and loading:
new - creates a new fileFormat
save - saves the current fileFormat, replacing any existing file with same name
save as - create a new fileFormat
rename - renames the fileFormat, prevents using an existing board name
open - creates a new fileFormat if needed
autosave - creates a new fileFormat
*/
