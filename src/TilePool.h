#pragma once

#include <Chunk.h>

#include <bitset>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

class Tile;

/**
 * Provides a space in memory to store `Tile` data. This is useful because all
 * of the information for a tile is stored within its chunk.
 * 
 * To store a tile, space must be reserved with `allocateSector()` to get a
 * sector id corresponding to `SECTOR_SIZE` number of tiles. Sectors should be
 * freed when none of the tiles in the sector are needed anymore.
 * 
 * FIXME: the TilePool is greedy, and never deallocates the chunks. Perhaps we
 * could fix this by cutting the items pool in half when we have enough
 * zero-allocated items at the end of the vector? This still leaves the
 * potential for a partially allocated item at the end of the vector to hog
 * space (should not be an issue when using this for edit history).
 * 
 * FIXME: no protection against a double free, did I just miss that?
 * it should also be an option to allocate-and-clear like with calloc().
 */
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
