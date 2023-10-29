#pragma once

#include <Tile.h>
#include <TileType.h>





#include <iostream>

class Chunk;

namespace tiles {

class Wire : public TileType {
public:
    static Wire* instance();
    virtual ~Wire() = default;
    Wire(const Wire& wire) = delete;
    Wire(Wire&& wire) = delete;
    Wire& operator=(const Wire& wire) = delete;
    Wire& operator=(Wire&& wire) = delete;

    virtual void setDirection(Chunk& chunk, unsigned int tileIndex, Direction::t direction) override;
    virtual void setState(Chunk& chunk, unsigned int tileIndex, State::t state) override;
    virtual void flip(Chunk& chunk, unsigned int tileIndex, bool acrossHorizontal) override;
    virtual void alternativeTile(Chunk& chunk, unsigned int tileIndex) override;

private:
    Wire() {
        std::cout << "Wire class has been constructed.\n";
    }

    void init(Chunk& chunk, unsigned int tileIndex, TileId::t wireId, Direction::t direction, State::t state1, State::t state2);

    friend class ::Tile;
};

} // namespace tiles
