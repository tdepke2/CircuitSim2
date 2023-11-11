#pragma once

#include <Tile.h>
#include <TileType.h>

class Chunk;

namespace tiles {

class Blank : public TileType {
public:
    static Blank* instance();
    virtual ~Blank() = default;
    Blank(const Blank& blank) = delete;
    Blank(Blank&& blank) = delete;
    Blank& operator=(const Blank& blank) = delete;
    Blank& operator=(Blank&& blank) = delete;

    virtual void flip(Chunk& chunk, unsigned int tileIndex, bool acrossHorizontal) override;
    virtual void alternativeTile(Chunk& chunk, unsigned int tileIndex) override;

private:
    Blank();

    void init(Chunk& chunk, unsigned int tileIndex);

    friend class ::Tile;
};

} // namespace tiles
