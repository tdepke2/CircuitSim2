#pragma once

#include <Tile.h>
#include <TileType.h>

class Chunk;

namespace tiles {

class Gate : public TileType {
public:
    static Gate* instance();
    virtual ~Gate() = default;
    Gate(const Gate& gate) = delete;
    Gate(Gate&& gate) = delete;
    Gate& operator=(const Gate& gate) = delete;
    Gate& operator=(Gate&& gate) = delete;

    virtual void setDirection(Chunk& chunk, unsigned int tileIndex, Direction::t direction) override;
    virtual void setState(Chunk& chunk, unsigned int tileIndex, State::t state) override;
    virtual void flip(Chunk& chunk, unsigned int tileIndex, bool acrossHorizontal) override;
    virtual void alternativeTile(Chunk& chunk, unsigned int tileIndex) override;

private:
    Gate();

    void init(Chunk& chunk, unsigned int tileIndex, TileId::t gateId, Direction::t direction, State::t state);

    friend class ::Tile;
};

} // namespace tiles
