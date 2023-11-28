#pragma once

#include <Chunk.h>
#include <ChunkCoords.h>
#include <FileStorage.h>
#include <Filesystem.h>
#include <FlatMap.h>
#include <LegacyFileFormat.h>

#include <chrono>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>

class Board;

class RegionFileFormat : public FileStorage {
public:
    static constexpr int REGION_WIDTH = 32;
    static constexpr int SECTOR_SIZE = 256;

    RegionFileFormat();

    virtual bool validateFileVersion(float version) override;
    virtual void loadFromFile(Board& board, const fs::path& filename, fs::ifstream& boardFile) override;
    virtual void saveToFile(Board& board) override;

    void loadChunk(Board& board, ChunkCoords::repr chunkCoords) const;

private:
    using Region = std::set<ChunkCoords::repr>;
    using RegionCoords = std::pair<int, int>;

    struct ParseState : public LegacyFileFormat::HeaderState {
        std::set<RegionCoords> regions;
    };
    struct ChunkHeader {
        uint32_t offset : 24;
        uint8_t sectors : 8;
    };

    static RegionCoords toRegionCoords(ChunkCoords::repr chunkCoords);
    static std::pair<int, int> toRegionOffset(ChunkCoords::repr chunkCoords);
    static void parseRegionList(Board& board, const std::string& line, int lineNumber, ParseState& state);
    static void readRegionHeader(ChunkHeader header[], const fs::path& filename, fs::ifstream& regionFile);

    void loadRegion(Board& board, const RegionCoords& regionCoords);
    void saveRegion(Board& board, const RegionCoords& regionCoords, const Region& region);

    fs::path filename_;
    std::map<RegionCoords, Region> savedRegions_;
    mutable std::unordered_map<ChunkCoords::repr, Chunk> chunkCache_;
    mutable FlatMap<ChunkCoords::repr, std::chrono::time_point<std::chrono::steady_clock>> chunkCacheTimes_;
};
