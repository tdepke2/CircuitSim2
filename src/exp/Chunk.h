#pragma once

#include <Tile.h>

#include <cassert>
#include <SFML/Graphics.hpp>

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

    TileData();
    TileData(TileId::t id, State::t state1, State::t state2, Direction::t dir = Direction::north, bool highlight = false, uint16_t meta = 0);
    uint16_t getTextureHash() const;
};
//#pragma pack(pop)

static_assert(sizeof(TileData) == 4, "Size of TileData struct is expected to be 4 bytes.");
static_assert(TileId::count <= 32, "TileId::t is expected to fit within a 5 bit value.");

class Chunk : public sf::Drawable {
public:
    static constexpr int WIDTH = 16;

    static uint8_t textureLookup[512];
    static void buildTextureLookup();

    Chunk(unsigned int textureWidth, unsigned int tileWidth);
    Tile accessTile(unsigned int x, unsigned int y);
    void debugPrintChunk();

private:
    void redrawTile(unsigned int x, unsigned int y);
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    TileData tiles_[WIDTH * WIDTH];
    sf::VertexArray vertices_;
    unsigned int textureWidth_, tileWidth_;

    friend class TileType;
};
