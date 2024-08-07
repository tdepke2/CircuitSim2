#pragma once

#include <Chunk.h>
#include <ChunkCoords.h>
#include <ChunkCoordsRange.h>
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

/**
 * Chunk-based file format used for circuits.
 * 
 * This format allows chunks to load and save back to disk dynamically. This
 * makes it ideal for very large circuits. Some caching is used to batch load
 * chunks in a square area, so it is most efficient to load nearby chunks
 * instead of jumping around to different spots when loading them.
 * 
 * The file structure starts with a directory set to the board name, and that
 * contains a "board.txt" file with the board properties along with a "region"
 * directory containing region data files. Each region data file is named
 * "x.y.dat" corresponding to the region x and y coordinates, and contains data
 * for all of the saved chunks for that region.
 * 
 * The region represents a 32 by 32 area, containing up to 1024 chunks. The file
 * starts with a 4096 byte header serving as a lookup table for the chunk data.
 * Each entry in the lookup table has a 3 byte sector offset and 1 byte sector
 * count (in big-endian format). These represent the offset of the chunk data
 * (from the start of the file) in 256 byte sectors, and the number of allocated
 * sectors respectively. The lookup table is also in row-major order, so the
 * first 4 bytes are for the chunk at (0,0) and the next for (1,0). The chunk
 * data is just composed of a 4 byte total length (big-endian) and the following
 * bytes, padded with zeros to an even number of sectors. The length includes
 * the length bytes itself, so it can be 4 at minimum.
 * 
 * The region file format is based on the McRegion format used in Minecraft:
 * https://minecraft.wiki/w/Region_file_format
 */
class RegionFileFormat : public FileStorage {
public:
    static constexpr int REGION_WIDTH = 32;
    static constexpr int SECTOR_SIZE = 256;

    RegionFileFormat(const fs::path& filename = "boards/NewBoard/board.txt");

    virtual bool validateFileVersion(float version) override;
    virtual void loadFromFile(Board& board, const fs::path& filename, fs::ifstream& boardFile) override;
    virtual void saveToFile(Board& board) override;

    virtual void updateVisibleChunks(Board& board, const ChunkCoordsRange& visibleChunks) override;
    virtual bool loadChunk(Board& board, ChunkCoords::repr chunkCoords) override;

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
    static void readRegionHeader(ChunkHeader header[], const fs::path& filename, std::istream& regionFile);
    static void writeRegionHeader(ChunkHeader header[], const fs::path& filename, std::ostream& regionFile);
    static std::vector<char> readChunk(ChunkHeader header[], int headerIndex, const fs::path& filename, std::istream& regionFile);
    static void writeChunk(ChunkHeader header[], int headerIndex, const char emptySector[], uint32_t& lastOffset, const std::vector<char>& chunkData, const fs::path& filename, std::ostream& regionFile);

    void loadRegion(Board& board, const RegionCoords& regionCoords);
    void saveRegion(Board& board, const RegionCoords& regionCoords, const Region& region);

    std::map<RegionCoords, Region> savedRegions_;
    ChunkCoordsRange lastVisibleChunks_;
    std::unordered_map<ChunkCoords::repr, Chunk> chunkCache_;
    FlatMap<ChunkCoords::repr, std::chrono::time_point<std::chrono::steady_clock>> chunkCacheTimes_;
};
