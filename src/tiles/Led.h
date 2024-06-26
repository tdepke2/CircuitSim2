#pragma once

#include <Tile.h>
#include <TileType.h>

class Chunk;

namespace tiles {

class Led : public TileType {
private:
    struct Private {
        explicit Private() = default;
    };

public:
    static Led* instance();
    Led(Private);
    virtual ~Led() = default;
    Led(const Led& led) = delete;
    Led(Led&& led) = delete;
    Led& operator=(const Led& led) = delete;
    Led& operator=(Led&& led) = delete;

    virtual void setState(Chunk& chunk, unsigned int tileIndex, State::t state) override;
    virtual void alternativeTile(Chunk& chunk, unsigned int tileIndex) override;
    virtual void cloneTo(const Chunk& chunk, unsigned int tileIndex, Tile target) override;

private:
    void init(Chunk& chunk, unsigned int tileIndex, State::t state);

    friend class ::Tile;
};

} // namespace tiles
