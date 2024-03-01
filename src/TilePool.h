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

// FIXME: above is no longer true after redesign.

class TilePool {
public:
    static constexpr int SECTOR_SIZE = Chunk::WIDTH * Chunk::WIDTH / 64;

    TilePool();
    TilePool(const TilePool& rhs) = delete;
    TilePool& operator=(const TilePool& rhs) = delete;

    unsigned int getTotalAllocated() const;
    size_t allocateSector();
    void freeSector(size_t sectorId);
    Tile accessTile(size_t sectorId, unsigned int offset);
    void debugPrintInfo();

private:
    struct PoolItem {
        PoolItem() :
            chunk(nullptr, 0),
            allocated() {
        }

        Chunk chunk;
        std::bitset<Chunk::WIDTH * Chunk::WIDTH / SECTOR_SIZE> allocated;
    };

    std::vector<std::unique_ptr<PoolItem>> items_;
    std::vector<unsigned int> itemsAllocatedCount_;
};

// FIXME:
// lets allocate in sectors instead of individual tiles
// a command could track its tiles with a vector<sector_id> and vector<tile_pos>, that second vector tells the total number of tiles.
// sector size = Chunk::WIDTH / 64 ?
// swapping tiles from the pool to a chunk may have a problem, make entity store a chunk pointer instead of reference?
