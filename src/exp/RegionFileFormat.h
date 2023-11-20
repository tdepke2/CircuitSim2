#pragma once

#include <ChunkCoords.h>
#include <FileStorage.h>
#include <Filesystem.h>

#include <fstream>
#include <map>
#include <set>
#include <utility>

class Board;

class RegionFileFormat : public FileStorage {
public:
    static constexpr int REGION_WIDTH = 32;

    RegionFileFormat();

    virtual bool validateFileVersion(float version) override;
    virtual void loadFromFile(Board& board, const fs::path& filename, std::ifstream& inputFile) override;
    virtual void saveToFile(Board& board) override;

    void loadChunk();    // will load (and return?) a single chunk, reads in a batch of chunks from region file at a time.
    // it is efficient to load nearby chunks sequentially instead of jumping around to different spots.
    void saveAllChunks();

private:
    using Region = std::set<ChunkCoords::repr>;
    using RegionCoords = std::pair<int, int>;

    fs::path filename_;
    std::map<RegionCoords, Region> savedRegions_;
};
