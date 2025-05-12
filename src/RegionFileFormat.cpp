#include <Board.h>
#include <RegionFileFormat.h>

#include <cassert>
#include <iterator>
#include <limits>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ranges.h>
#include <stdexcept>
#include <tuple>

namespace {

constexpr int constLog2(int x) {
    return x == 1 ? 0 : 1 + constLog2(x / 2);
}

}

constexpr int RegionFileFormat::REGION_WIDTH;
constexpr int RegionFileFormat::SECTOR_SIZE;
constexpr int RegionFileFormat::HEADER_SIZE;

RegionFileFormat::RegionFileFormat(const fs::path& filename) :
    FileStorage(filename.filename() == "board.txt" ? filename.parent_path() : filename),
    savedRegions_(),
    lastVisibleChunks_(0, 0, 0, 0),
    chunkCache_(),
    chunkCacheTimes_() {
}

fs::path RegionFileFormat::getDefaultFileExtension() const {
    return "";
}

bool RegionFileFormat::validateFileVersion(float version) {
    return version == 2.0;
}

void RegionFileFormat::loadFromFile(Board& board, const fs::path& filename, fs::ifstream& boardFile) {
    // Reset all members to ensure a clean state.
    setFilename(filename.filename() == "board.txt" ? filename.parent_path() : filename);
    setNewFile(false);
    savedRegions_.clear();
    lastVisibleChunks_ = ChunkCoordsRange(0, 0, 0, 0);
    chunkCache_.clear();
    chunkCacheTimes_.clear();

    const fs::path boardFilename = getFilename() / "board.txt";
    if (!boardFile.is_open()) {
        throw std::runtime_error("\"" + boardFilename.string() + "\": unable to open file for reading.");
    }
    boardFile.clear();
    boardFile.seekg(0, std::ios::beg);

    std::string line;
    int lineNumber = 0;
    ParseState state;
    state.filename = boardFilename;
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
        throw std::runtime_error("\"" + boardFilename.string() + "\" at line " + std::to_string(lineNumber) + ": " + ex.what());
    }

    for (const auto& regionCoords : state.regions) {
        spdlog::debug("loading region {}", regionCoords);
        loadRegion(board, regionCoords);
    }
}

void RegionFileFormat::saveToFile(Board& board) {
    std::map<RegionCoords, Region> unsavedRegions;

    for (const auto& chunk : board.getLoadedChunks()) {
        const auto savedRegion = savedRegions_.find(toRegionCoords(chunk.first));
        bool chunkInSavedRegions = (savedRegion != savedRegions_.end() && savedRegion->second.count(chunk.first) > 0);
        if ((!chunkInSavedRegions && !chunk.second.isEmpty()) ||
            (chunkInSavedRegions && chunk.second.isUnsaved())) {

            unsavedRegions[toRegionCoords(chunk.first)].insert(chunk.first);
        } else if (chunk.second.isEmpty()) {
            chunk.second.markAsSaved();
        }
    }

    spdlog::debug("save file: {}", getFilename());
    spdlog::debug("unsavedRegions:");
    for (auto& region : unsavedRegions) {
        spdlog::debug("  region {} ->", region.first);
        for (auto& chunkCoords : region.second) {
            spdlog::debug("    chunk {}", ChunkCoords::toPair(chunkCoords));
        }
    }
    if (unsavedRegions.empty() && !isNewFile()) {
        spdlog::debug("No regions to save.");
        return;
    }

    fs::create_directories(getFilename());
    fs::create_directory(getFilename() / "region");
    for (const auto& region : unsavedRegions) {
        saveRegion(board, region.first, region.second);
    }

    const fs::path boardFilename = getFilename() / "board.txt";
    fs::ofstream boardFile(boardFilename);
    if (!boardFile.is_open()) {
        throw std::runtime_error("\"" + boardFilename.string() + "\": unable to open file for writing.");
    }
    setNewFile(false);
    LegacyFileFormat::writeHeader(board, boardFilename, boardFile, 2.0f);
    boardFile << "regions: {\n";
    for (const auto& region : savedRegions_) {
        boardFile << region.first.first << "," << region.first.second << "\n";
    }
    boardFile << "}\n";
    boardFile.close();
}

void RegionFileFormat::saveAsFile(Board& board, const fs::path& filename) {
    // If the files already exist, copy them into the new path.
    if (!isNewFile()) {
        fs::create_directories(filename);
        fs::copy(getFilename(), filename, fs::copy_options::recursive);
    }

    setFilename(filename);
    saveToFile(board);
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
    spdlog::debug("Checking chunk {} for load.", ChunkCoords::toPair(chunkCoords));
    const auto regionCoords = toRegionCoords(chunkCoords);
    auto region = savedRegions_.find(regionCoords);
    if (region == savedRegions_.end() || region->second.count(chunkCoords) == 0 || board.isChunkLoaded(chunkCoords)) {
        return false;
    }

    auto cachedChunk = chunkCache_.find(chunkCoords);
    if (cachedChunk != chunkCache_.end()) {
        spdlog::debug("Loading cached chunk {}.", ChunkCoords::toPair(chunkCoords));
        board.loadChunk(std::move(cachedChunk->second));
        chunkCache_.erase(cachedChunk);
        chunkCacheTimes_.erase(chunkCoords);
        return true;
    }

    fs::path regionFilename = std::to_string(regionCoords.first) + "." + std::to_string(regionCoords.second) + ".dat";
    regionFilename = getFilename() / "region" / regionFilename;
    fs::ifstream regionFile(regionFilename, std::ios::binary);
    if (!regionFile.is_open()) {
        throw std::runtime_error("\"" + regionFilename.string() + "\": unable to open file for reading.");
    }
    ChunkHeader header;
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
                        "Removing cached chunk at {} (caching would exceed {} chunks).",
                        ChunkCoords::toPair(minCacheTime->first), CACHE_SIZE
                    );
                    chunkCache_.erase(minCacheTime->first);
                    chunkCacheTimes_.erase(minCacheTime);
                }

                spdlog::debug(
                    "Caching chunk {} while loading chunk at {}.",
                    ChunkCoords::toPair(cacheChunkCoords), ChunkCoords::toPair(chunkCoords)
                );

                auto chunk = chunkCache_.emplace(std::piecewise_construct, std::forward_as_tuple(cacheChunkCoords), std::forward_as_tuple(nullptr, cacheChunkCoords)).first;
                chunkCacheTimes_.emplace(cacheChunkCoords, std::chrono::steady_clock::now());
                chunk->second.deserialize(readChunk(header[headerIndex], regionFilename, regionFile));
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

void RegionFileFormat::readRegionHeader(ChunkHeader& header, const fs::path& /*filename*/, std::istream& regionFile) {
    for (auto& entry : header) {
        uint32_t offset;
        regionFile.read(reinterpret_cast<char*>(&offset), 3);
        entry.offset = byteswap(offset) >> 8;
        uint8_t sectors;
        regionFile.read(reinterpret_cast<char*>(&sectors), 1);
        entry.sectors = sectors;
    }
}

void RegionFileFormat::writeRegionHeader(const ChunkHeader& header, const fs::path& /*filename*/, std::ostream& regionFile) {
    regionFile.seekp(0, std::ios::beg);
    for (const auto& entry : header) {
        uint32_t offset = byteswap(entry.offset << 8);
        regionFile.write(reinterpret_cast<char*>(&offset), 3);
        uint8_t sectors = entry.sectors;
        regionFile.write(reinterpret_cast<char*>(&sectors), 1);
    }
}

std::vector<char> RegionFileFormat::readChunk(const ChunkHeaderEntry& headerEntry, const fs::path& /*filename*/, std::istream& regionFile) {
    regionFile.seekg(headerEntry.offset * SECTOR_SIZE, std::ios::beg);
    uint32_t chunkSize;
    regionFile.read(reinterpret_cast<char*>(&chunkSize), sizeof(chunkSize));
    chunkSize = byteswap(chunkSize) - sizeof(chunkSize);
    std::vector<char> chunkData(chunkSize);
    regionFile.read(chunkData.data(), chunkSize);
    return chunkData;
}

void RegionFileFormat::writeChunk(ChunkHeaderEntry& headerEntry, const char emptySector[], uint32_t& lastOffset, const std::vector<char>& chunkData, const fs::path& /*filename*/, std::ostream& regionFile) {
    const uint32_t serializedSize = static_cast<uint32_t>(chunkData.size() + sizeof(serializedSize));
    const uint32_t sectorCount = (serializedSize + SECTOR_SIZE - 1) / SECTOR_SIZE;
    if (headerEntry.sectors > 0) {
        assert(headerEntry.sectors >= sectorCount);
        regionFile.seekp(headerEntry.offset * SECTOR_SIZE, std::ios::beg);
    } else {
        headerEntry.offset = lastOffset;
        headerEntry.sectors = static_cast<uint8_t>(sectorCount);
        regionFile.seekp(lastOffset * SECTOR_SIZE, std::ios::beg);
        lastOffset += headerEntry.sectors;
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
    regionFilename = getFilename() / "region" / regionFilename;
    fs::ifstream regionFile(regionFilename, std::ios::binary);
    if (!regionFile.is_open()) {
        throw std::runtime_error("\"" + regionFilename.string() + "\": unable to open file for reading.");
    }

    ChunkHeader header;
    readRegionHeader(header, regionFilename, regionFile);
    for (int i = 0; i < static_cast<int>(header.size()); ++i) {
        if (header[i].sectors > 0) {
            savedRegions_[regionCoords].insert(ChunkCoords::pack(i % REGION_WIDTH + regionCoords.first * REGION_WIDTH, i / REGION_WIDTH + regionCoords.second * REGION_WIDTH));
        }
    }
    regionFile.close();
}

void RegionFileFormat::saveRegion(Board& board, const RegionCoords& regionCoords, const Region& region) {
    fs::path regionFilename = std::to_string(regionCoords.first) + "." + std::to_string(regionCoords.second) + ".dat";
    regionFilename = getFilename() / "region" / regionFilename;
    fs::fstream regionFile(regionFilename, std::ios::in | std::ios::out | std::ios::binary);
    spdlog::debug("Saving chunks for region {}.", regionCoords);

    // FIXME: it seems like chunks saved in the region file never get deleted, need to fix.

    ChunkHeader header = {};
    std::streamsize regionFileLength = 0;
    if (regionFile.is_open()) {
        // Determine number of bytes we can read from the file, should match the file size in most cases.
        // https://stackoverflow.com/questions/22984956/tellg-function-give-wrong-size-of-file/22986486#22986486
        regionFile.ignore(std::numeric_limits<std::streamsize>::max());
        regionFileLength = regionFile.gcount();
        regionFile.clear();
        regionFile.seekg(0, std::ios::beg);
        if (regionFileLength < HEADER_SIZE) {
            throw std::runtime_error(
                "\"" + regionFilename.string() + "\": binary file with " + std::to_string(regionFileLength) +
                " bytes is less than minimum size of " + std::to_string(HEADER_SIZE) + " bytes."
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

    RegionSectorPool regionSectorPool(header);

    struct SerializeInfo {
        SerializeInfo(uint8_t oldSectorCount, uint8_t newSectorCount, ChunkCoords::repr coords, const std::vector<char>& data) :
            oldSectorCount(oldSectorCount),
            newSectorCount(newSectorCount),
            coords(coords),
            data(data) {
        }

        uint8_t oldSectorCount;
        uint8_t newSectorCount;
        ChunkCoords::repr coords;
        std::vector<char> data;
    };

    std::map<RegionSectorPool::SectorOffset, SerializeInfo> serializedChunks;
    for (const auto& chunkCoords : region) {
        const auto regionOffset = toRegionOffset(chunkCoords);
        const auto& headerEntry = header[regionOffset.first + regionOffset.second * REGION_WIDTH];

        auto chunkSerialized = board.getLoadedChunks().at(chunkCoords).serialize();
        const uint32_t serializedSize = static_cast<uint32_t>(chunkSerialized.size() + sizeof(serializedSize));
        const uint32_t sectorCount = (serializedSize + SECTOR_SIZE - 1) / SECTOR_SIZE;

        // FIXME: confirm sectorCount within range?

        RegionSectorPool::SectorOffset offset = headerEntry.offset;
        uint8_t oldSectorCount = headerEntry.sectors;
        uint8_t newSectorCount = sectorCount;
        if (oldSectorCount > 0) {
            if (chunkSerialized.empty()) {
                spdlog::debug("Chunk {} is now empty, removing.", ChunkCoords::toPair(chunkCoords));
                newSectorCount = 0;
            } else if (newSectorCount != oldSectorCount) {
                spdlog::debug("Chunk {} has {} allocated sectors and now requires {}, reallocating.", ChunkCoords::toPair(chunkCoords), oldSectorCount, newSectorCount);
                regionSectorPool.freeSectors(offset, oldSectorCount);
                offset = regionSectorPool.allocateSectors(newSectorCount);
            }
        } else {
            offset = regionSectorPool.allocateSectors(newSectorCount);
        }
        const auto emplaceResult = serializedChunks.emplace(
            offset,
            SerializeInfo(oldSectorCount, newSectorCount, chunkCoords, std::move(chunkSerialized))
        );
        assert(emplaceResult.second);
    }

    // go through serializedChunks and write them (marking dead sectors along the way). this should write to file (mostly) in order.
    // it's possible chunk A shrinks, and a new chunk B allocates after it. in this case we mark dead sectors for A and then go back and overwrite them, that's fine.
    // where do we update the header for the chunk? perhaps do it in writeChunk()?

    for (const auto& serialized : serializedChunks) {
        if (serialized.second.data.empty()) {
            savedRegions_[regionCoords].erase(serialized.first);
            board.getLoadedChunks().at(serialized.first).markAsSaved();
            continue;
            // ???
        }
    }









    /*std::map<ChunkCoords::repr, std::vector<char>> serializedChunks;
    for (const auto& chunkCoords : region) {
        const auto serialized = serializedChunks.emplace(chunkCoords, board.getLoadedChunks().at(chunkCoords).serialize()).first;
        if (regionFileLength > 0) {
            const auto regionOffset = toRegionOffset(chunkCoords);
            const int headerIndex = regionOffset.first + regionOffset.second * REGION_WIDTH;
            const uint32_t serializedSize = static_cast<uint32_t>(serialized->second.size() + sizeof(serializedSize));
            const uint32_t sectorCount = (serializedSize + SECTOR_SIZE - 1) / SECTOR_SIZE;
            if (header[headerIndex].sectors > 0) {
                if (serialized->second.empty()) {
                    spdlog::debug("Chunk {} requires 0 sectors, removing.", ChunkCoords::toPair(chunkCoords));
                    reallocationRequired = true;
                } else if (header[headerIndex].sectors < sectorCount) {
                    const uint32_t currentSectors = header[headerIndex].sectors;
                    spdlog::debug("Chunk {} requires {} sectors but only {} are allocated.", ChunkCoords::toPair(chunkCoords), sectorCount, currentSectors);
                    reallocationRequired = true;
                }
            }
        }
    }

    const char emptySector[SECTOR_SIZE] = {};
    uint32_t lastOffset = HEADER_SIZE / SECTOR_SIZE;
    if (reallocationRequired) {
        // Read in all chunks and wipe header.
        spdlog::debug("Preparing all chunks for reallocation.");
        for (int i = 0; i < static_cast<int>(header.size()); ++i) {
            const auto chunkCoords = ChunkCoords::pack(i % REGION_WIDTH + regionCoords.first * REGION_WIDTH, i / REGION_WIDTH + regionCoords.second * REGION_WIDTH);
            if (header[i].sectors > 0 && serializedChunks.count(chunkCoords) == 0) {
                serializedChunks.emplace(
                    chunkCoords,
                    readChunk(header[i], regionFilename, regionFile)
                );
            }
            header[i].offset = 0;
            header[i].sectors = 0;
        }
    } else if (regionFileLength == 0) {
        // Write empty header.
        for (uint32_t i = 0; i < lastOffset; ++i) {
            regionFile.write(emptySector, SECTOR_SIZE);
        }
    } else {
        lastOffset = static_cast<uint32_t>(regionFileLength / SECTOR_SIZE);
    }

    // Write chunks.
    for (const auto& serialized : serializedChunks) {
        const auto regionOffset = toRegionOffset(serialized.first);
        const int headerIndex = regionOffset.first + regionOffset.second * REGION_WIDTH;
        const uint32_t serializedSize = static_cast<uint32_t>(serialized.second.size() + sizeof(serializedSize));
        if (serialized.second.empty()) {
            savedRegions_[regionCoords].erase(serialized.first);
            board.getLoadedChunks().at(serialized.first).markAsSaved();
            continue;
        } else if (serializedSize <= std::numeric_limits<uint8_t>::max() * static_cast<uint32_t>(SECTOR_SIZE)) {
            savedRegions_[regionCoords].insert(serialized.first);
        } else {
            // FIXME unable to save this chunk, too much data
            // FIXME this check should be moved into writeChunk().
            spdlog::error("Failed to save chunk at {} (serialized to {} bytes).", ChunkCoords::toPair(serialized.first), serialized.second.size());
            continue;
        }

        spdlog::debug("Writing chunk {}.", ChunkCoords::toPair(serialized.first));
        board.getLoadedChunks().at(serialized.first).debugPrintChunk();
        writeChunk(header[headerIndex], emptySector, lastOffset, serialized.second, regionFilename, regionFile);
        board.getLoadedChunks().at(serialized.first).markAsSaved();
    }*/

    // Write (filled) header.
    writeRegionHeader(header, regionFilename, regionFile);

    // FIXME make a check if the regionFileLength is greater than length from lastOffset to determine if we need to
    // truncate the file with fs::resize_file

    regionFile.close();

    // FIXME need to clean up empty chunks (or should we do this after all regions have been saved?).


}

RegionSectorPool::RegionSectorPool(const RegionFileFormat::ChunkHeader& header) :
    freeSectors_({{
        RegionFileFormat::HEADER_SIZE / RegionFileFormat::SECTOR_SIZE,
        std::numeric_limits<unsigned int>::max() - RegionFileFormat::HEADER_SIZE / RegionFileFormat::SECTOR_SIZE
    }}) {

    for (int i = 0; i < static_cast<int>(header.size()); ++i) {
        if (header[i].sectors > 0) {
            const auto trailingSector = freeSectors_.upper_bound(header[i].offset);
            if (trailingSector == freeSectors_.begin()) {
                throw std::runtime_error("chunk " + std::to_string(i) + " at offset " + std::to_string(header[i].offset) + " begins before a free sector.");
            }
            const auto leadingSector = std::prev(trailingSector, 1);
            if (leadingSector->first == header[i].offset) {
                // Allocating at the start of the free sectors, we may have remaining space after allocation.
                if (leadingSector->second > header[i].sectors) {
                    freeSectors_.emplace(header[i].offset + header[i].sectors, leadingSector->second - header[i].sectors);
                } else if (leadingSector->second < header[i].sectors) {
                    throw std::runtime_error("chunk " + std::to_string(i) + " at offset " + std::to_string(header[i].offset) + " requires more sectors than available.");
                }
                freeSectors_.erase(leadingSector);
            } else if (leadingSector->first < header[i].offset) {
                // Allocating after the start, we will have space before the allocation and may have space after.
                const SectorOffset newOffset = header[i].offset + header[i].sectors;
                if (leadingSector->first + leadingSector->second > newOffset) {
                    freeSectors_.emplace(newOffset, leadingSector->first + leadingSector->second - newOffset);
                } else if (leadingSector->first + leadingSector->second < newOffset) {
                    throw std::runtime_error("chunk " + std::to_string(i) + " at offset " + std::to_string(header[i].offset) + " requires unavailable sectors.");
                }
                leadingSector->second = header[i].offset - leadingSector->first;
            } else {
                assert(false);
            }
        }
    }
}

const std::map<RegionSectorPool::SectorOffset, unsigned int>& RegionSectorPool::getFreeSectors() const {
    return freeSectors_;
}

RegionSectorPool::SectorOffset RegionSectorPool::allocateSectors(unsigned int count) {
    for (auto sectors = freeSectors_.begin(); sectors != freeSectors_.end(); ++sectors) {
        if (sectors->second >= count) {
            if (sectors->first >= (1 << 24)) {
                throw std::runtime_error("failed to allocate sectors (next offset of " + std::to_string(sectors->first) + " exceeds limit).");
            } else if (sectors->second > count) {
                freeSectors_.emplace(sectors->first + count, sectors->second - count);
            }
            const SectorOffset offset = sectors->first;
            freeSectors_.erase(sectors);
            return offset;
        }
    }
    assert(false);
    return 0;
}

void RegionSectorPool::freeSectors(SectorOffset offset, unsigned int count) {
    if (offset < RegionFileFormat::HEADER_SIZE / RegionFileFormat::SECTOR_SIZE) {
        throw std::runtime_error("attempt to free sectors within header (offset is " + std::to_string(offset) + " with count " + std::to_string(count) + ").");
    }

    const auto trailingSector = freeSectors_.upper_bound(offset);
    const auto leadingSector = (trailingSector != freeSectors_.begin() ? std::prev(trailingSector, 1) : freeSectors_.end());
    bool mergeTrailing = false, mergeLeading = false;

    if (trailingSector != freeSectors_.end()) {
        if (offset + count == trailingSector->first) {
            mergeTrailing = true;
        } else if (offset + count > trailingSector->first) {
            throw std::runtime_error("attempt to free trailing sectors that are already free (offset is " + std::to_string(offset) + " with count " + std::to_string(count) + ").");
        }
    }

    if (leadingSector != freeSectors_.end()) {
        if (leadingSector->first + leadingSector->second == offset) {
            mergeLeading = true;
        } else if (leadingSector->first + leadingSector->second > offset) {
            throw std::runtime_error("attempt to free leading sectors that are already free (offset is " + std::to_string(offset) + " with count " + std::to_string(count) + ").");
        }
    }

    if (mergeTrailing && mergeLeading) {
        leadingSector->second += count + trailingSector->second;
        freeSectors_.erase(trailingSector);
    } else if (mergeTrailing) {
        freeSectors_.emplace(offset, count + trailingSector->second);
        freeSectors_.erase(trailingSector);
    } else if (mergeLeading) {
        leadingSector->second += count;
    } else {
        freeSectors_.emplace(offset, count);
    }
}

// when we save a chunk:
// chunk uses less space -> freeSectors() it doesn't need.
// chunk uses same space -> ez
// chunk uses more space -> freeSectors() it was using, mark deadbeef, and allocateSectors() (may allocate into same space, or move it)
// actually, we may want to always free the whole space and reallocate if size changed. this could proactively reduce fragmentation when chunk sizes change.

/*

read header...

serialize chunks into memory

existing chunks need more memory?
    read all chunks into memory
    wipe header

write chunks
write header
clean up dirty chunks in board that are empty

above method sucks, lets try this:
* read header
* find holes in file and their sector counts
    - make a map of free sector offsets to sector counts, init with 16 (end of header) and u32::max?
    - for each chunk in header, find upper bound of offset in map and get iter before this (error if no iter before, could be caused by two chunks that overlap).
        * if iter equal, remove offset in map and replace with new value.
        * if not, update the sector count and add new entry for remaining.
        * in either case, if the original upper bound is not the end and is less/equal to the new offset, this is an error (chunks overlap).
* serialize chunks
* new chunk? toss in hole or at end of file
* existing chunk? update in place if possible, or find a hole/eof and mark its old sectors with "deadbeef"
* empty chunk? just mark the deadbeef sectors
* write header
* resize file if dead sectors at the end

FIXME: it (might?) be nice if we extended the header so that chunk size is not with the chunk data and most chunks could fit in the sectors evenly.
maybe not the best plan, we only lose about 6% of space with the current method and it allows some extra room for tile entities.

FIXME: issue with empty chunks not getting marked as saved (and show as orange) now that they get skipped during save...

saving and loading:
new - creates a new fileFormat
save - saves the current fileFormat, replacing any existing file with same name
save as - create a new fileFormat
rename - renames the fileFormat, prevents using an existing board name
open - creates a new fileFormat if needed
autosave - creates a new fileFormat
*/
