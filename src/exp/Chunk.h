#pragma once

#include <Tile.h>

#include <cassert>
#include <cstdint>
#include <ostream>

//#pragma pack(push, 1)
struct TileData {
    TileId::t      id : 5;
    uint8_t           : 3;
    State::t   state1 : 2;
    State::t   state2 : 2;
    Direction::t  dir : 2;
    bool    highlight : 1;
    uint8_t           : 1;
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

class Chunk {
public:
    static constexpr int WIDTH = 16;
    static void setupChunks();

    Chunk();
    ~Chunk() = default;
    Chunk(const Chunk& rhs) = delete;
    Chunk& operator=(const Chunk& rhs) = delete;

    Tile accessTile(unsigned int x, unsigned int y);
    void debugPrintChunk();

private:
    TileData tiles_[WIDTH * WIDTH];

    friend class ChunkDrawable;
    friend class TileType;

    friend std::ostream& operator<<(std::ostream& out, const Chunk& chunk);
};
