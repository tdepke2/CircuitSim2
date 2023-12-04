#pragma once

#include <ChunkCoords.h>
#include <Tile.h>

#include <bitset>
#include <cassert>
#include <cstdint>
#include <ostream>
#include <vector>

class Board;

//#pragma pack(push, 1)
struct TileData {
    TileId::t      id : 5;
    uint8_t           : 3;
    State::t   state1 : 2;
    State::t   state2 : 2;
    Direction::t  dir : 2;
    bool    highlight : 1;
    bool       redraw : 1;
    uint16_t     meta : 16;

    // Default initialization specifies that all values (including padding) will be zero-initialized.
    TileData() = default;
    // Note that this ctor does not initialize the padding bits.
    TileData(TileId::t id, State::t state1, State::t state2, Direction::t dir = Direction::north, bool highlight = false, uint16_t meta = 0);
    uint16_t getTextureHash() const;
};
//#pragma pack(pop)

static_assert(sizeof(TileData) == 4, "Size of TileData struct is expected to be 4 bytes.");
static_assert(TileId::count <= 32, "TileId::t is expected to fit within a 5 bit value.");

namespace ChunkDirtyFlag {
    enum t {
        unsaved = 0, emptyIsStale, drawPending, count
    };
}

class Chunk {
public:
    static constexpr int WIDTH = 16;
    static void setupChunks();

    Chunk(Board* board, ChunkCoords::repr coords);
    ~Chunk() = default;
    Chunk(const Chunk& rhs) = delete;
    Chunk(Chunk&& rhs) noexcept = default;
    Chunk& operator=(const Chunk& rhs) = delete;
    Chunk& operator=(Chunk&& rhs) noexcept = default;

    ChunkCoords::repr getCoords() const;
    void setBoard(Board* board);
    bool isUnsaved() const;
    bool isEmpty() const;
    Tile accessTile(unsigned int x, unsigned int y);
    std::vector<char> serialize() const;
    void deserialize(const std::vector<char>& data);
    void debugPrintChunk() const;

private:
    TileData tiles_[WIDTH * WIDTH];
    Board* board_;
    ChunkCoords::repr coords_;
    mutable std::bitset<ChunkDirtyFlag::count> dirtyFlags_;
    mutable bool empty_;

    void markTileDirty(unsigned int tileIndex);
    void markAsSaved();
    void markAsDrawn();

    friend class ChunkDrawable;
    friend class TileType;

    friend std::ostream& operator<<(std::ostream& out, const Chunk& chunk);
};
