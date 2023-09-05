#pragma once

#include <cassert>
#include <memory>
#include <SFML/Graphics.hpp>

class Tile;

namespace TileId {
    enum t : uint8_t {
        blank = 0,
        wireStraight, wireCorner, wireTee, wireJunction, wireCrossover,
        inSwitch, inButton, outLed,
        gateDiode, gateBuffer, gateNot, gateAnd, gateNand, gateOr, gateNor, gateXor, gateXnor,
        count
    };
}
static_assert(TileId::count <= 32, "TileId::t is expected to fit within a 5 bit value.");

namespace State {
    enum t : uint8_t {
        disconnected = 0, low, high, middle
    };
}

namespace Direction {
    enum t : uint8_t {
        north = 0, east, south, west
    };
}

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
};
//#pragma pack(pop)

static_assert(sizeof(TileData) == 4, "Size of TileData struct is expected to be 4 bytes.");

class Chunk : public sf::Drawable {
public:
    static constexpr int WIDTH = 8;

    static uint8_t textureLookup[];
    static void buildTextureLookup();

    Chunk(unsigned int textureWidth, unsigned int tileWidth);
    Tile accessTile(unsigned int x, unsigned int y);

private:
    void redrawTile(unsigned int x, unsigned int y);
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    TileData tiles_[WIDTH * WIDTH];
    sf::VertexArray vertices_;
    unsigned int textureWidth_, tileWidth_;
};
