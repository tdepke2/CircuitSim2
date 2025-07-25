#pragma once

#include <ChunkCoords.h>
#include <Entity.h>
#include <Tile.h>

#include <array>
#include <bitset>
#include <cassert>
#include <cstdint>
#include <istream>
#include <memory>
#include <ostream>

class LodRenderer;

//#pragma pack(push, 1)

/**
 * The internal state of a tile, using bitfields to pack the members into just 4
 * bytes. See the `Tile` class for an interface that makes it easier to interact
 * with a tile (without touching the ugly bitfields directly).
 */
struct TileData {
    TileId::t      id : 5;
    uint8_t           : 3;
    State::t   state1 : 2;
    State::t   state2 : 2;
    Direction::t  dir : 2;
    bool    highlight : 1;
    uint8_t           : 1;
    uint16_t     meta : 16;

    // Default initialization specifies that all values (including padding) will be zero-initialized when using braced-initialization.
    TileData() = default;
    // Note that this ctor does not initialize the padding bits.
    TileData(TileId::t id, State::t state1, State::t state2, Direction::t dir = Direction::north, bool highlight = false, uint16_t meta = 0);
    // Converts texture-specific data (id, state1, and state2) into a unique unsigned integer. Result is guaranteed to fit in 9 bits, or 512 total values.
    uint16_t getTextureHash() const;

    // Swapping ignores the highlight bit.
    friend void swap(TileData& lhs, TileData& rhs);
    friend bool operator==(const TileData& lhs, const TileData& rhs);
    friend bool operator!=(const TileData& lhs, const TileData& rhs);
};
//#pragma pack(pop)

static_assert(sizeof(TileData) == 4, "Size of TileData struct is expected to be 4 bytes.");
static_assert(TileId::count <= 32, "TileId::t is expected to fit within a 5 bit value.");

namespace ChunkDirtyFlag {
    enum t {
        unsaved = 0, emptyIsStale, highlightedIsStale, drawPending, count
    };
}

/**
 * A square block of tiles. Storing tiles in large blocks instead of
 * individually helps improve performance of drawing, simulation, and open up
 * opportunities for multithreading.
 * 
 * Chunks can be drawn with a `ChunkDrawable` and serialized to/from a character
 * array for file i/o. The tiles are stored in a fixed array in row-major order
 * (with the zero coordinates as the first element in the array). The array is
 * also part of the class footprint (instead of dynamically allocated), so
 * copying and moving chunks is an expensive operation.
 */
class Chunk {
public:
    static constexpr int WIDTH = 32;

    Chunk(LodRenderer* lodRenderer, ChunkCoords::repr coords);
    ~Chunk() = default;
    Chunk(const Chunk& rhs) = delete;
    Chunk(Chunk&& rhs) noexcept = default;
    Chunk& operator=(const Chunk& rhs) = delete;
    Chunk& operator=(Chunk&& rhs) noexcept = default;

    ChunkCoords::repr getCoords() const;
    void setLodRenderer(LodRenderer* lodRenderer);
    const LodRenderer* getLodRenderer() const;
    LodRenderer* getLodRenderer();
    bool isUnsaved() const;
    bool isEmpty() const;
    bool isHighlighted() const;
    Tile accessTile(unsigned int tileIndex);
    uint32_t serializeLength() const;
    uint32_t serialize(std::ostream& out) const;
    void deserialize(std::istream& in);
    void markAsSaved() const;
    void markAsDrawn() const;
    void debugPrintChunk() const;

private:
    struct StaticInit {
        StaticInit();
        std::array<TileType*, TileId::count> tileIdToType;
    };
    static StaticInit* staticInit_;

    using EntityArray = std::unique_ptr<std::unique_ptr<Entity>[]>;

    TileData tiles_[WIDTH * WIDTH];
    EntityArray entities_;
    size_t entitiesCapacity_;
    LodRenderer* lodRenderer_;
    ChunkCoords::repr coords_;
    mutable std::bitset<ChunkDirtyFlag::count> dirtyFlags_;
    mutable bool empty_, highlighted_;

    void markTileDirty(unsigned int tileIndex);
    void markHighlightDirty(unsigned int tileIndex);
    void allocateEntity(unsigned int tileIndex, std::unique_ptr<Entity>&& entity);
    void freeEntity(unsigned int tileIndex);

    friend class ChunkDrawable;
    friend class TileType;

    // For simplicity, equality requires that any entities have the same ordering in the entity array for both chunks.
    friend bool operator==(const Chunk& lhs, const Chunk& rhs);
    friend bool operator!=(const Chunk& lhs, const Chunk& rhs);
    friend std::ostream& operator<<(std::ostream& out, const Chunk& chunk);
};
