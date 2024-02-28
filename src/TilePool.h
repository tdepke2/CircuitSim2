#pragma once

#include <Chunk.h>

#include <bitset>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

class Tile;

// Note: the TilePool operates somewhat like a std::deque, meaning it is
// efficient to free tiles in a similar order to the order of allocation. The
// internal vector will grow as new chunks need to be allocated for tiles. If
// all tiles in a chunk are freed, the chunk is moved to the last element of the
// vector for reuse. This creates holes in the vector, but the holes get removed
// when there are no allocated tiles in a previous element.

class TilePool {
public:
    TilePool();
    TilePool(const TilePool& rhs) = delete;
    TilePool& operator=(const TilePool& rhs) = delete;

    std::pair<Tile, uint64_t> allocateTile();
    void freeTile(uint64_t id);
    Tile accessTile(uint64_t id);
    void debugPrintInfo();

private:
    struct PoolItem {
        PoolItem() :
            chunk(nullptr, 0),
            allocated(),
            allocatedCount(0) {
        }

        Chunk chunk;
        std::bitset<Chunk::WIDTH * Chunk::WIDTH> allocated;
        unsigned int allocatedCount;
    };

    std::vector<std::unique_ptr<PoolItem>> items_;
    uint64_t baseId_, upperId_;
};

// FIXME: left off here
// lets allocate in sectors instead of individual tiles
// a command could track its tiles with a vector<sector_id> and vector<tile_pos>, that second vector tells the total number of tiles.
// sector size = Chunk::WIDTH / 64 ?
// swapping tiles from the pool to a chunk may have a problem, make entity store a chunk pointer instead of reference?
