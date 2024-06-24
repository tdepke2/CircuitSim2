#pragma once

#include <Tile.h>
#include <TileType.h>

class Chunk;

namespace tiles {

class Wire : public TileType {
private:
    struct Private {
        explicit Private() = default;
    };

public:
    static Wire* instance();
    Wire(Private);
    virtual ~Wire() = default;
    Wire(const Wire& wire) = delete;
    Wire(Wire&& wire) = delete;
    Wire& operator=(const Wire& wire) = delete;
    Wire& operator=(Wire&& wire) = delete;

    void setState2(Chunk& chunk, unsigned int tileIndex, State::t state);
    State::t getState2(const Chunk& chunk, unsigned int tileIndex) const;

    virtual void setDirection(Chunk& chunk, unsigned int tileIndex, Direction::t direction) override;
    virtual void setState(Chunk& chunk, unsigned int tileIndex, State::t state) override;
    virtual void flip(Chunk& chunk, unsigned int tileIndex, bool acrossVertical) override;
    virtual void alternativeTile(Chunk& chunk, unsigned int tileIndex) override;
    virtual void cloneTo(const Chunk& chunk, unsigned int tileIndex, Tile target) override;

private:
    void init(Chunk& chunk, unsigned int tileIndex, TileId::t wireId, Direction::t direction, State::t state1, State::t state2);

    friend class ::Tile;
};

} // namespace tiles
