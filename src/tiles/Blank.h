#pragma once

#include <Tile.h>
#include <TileType.h>

class Chunk;

namespace tiles {

class Blank : public TileType {
private:
    struct Private {
        explicit Private() = default;
    };

public:
    static Blank* instance();
    Blank(Private);
    virtual ~Blank() = default;
    Blank(const Blank& blank) = delete;
    Blank(Blank&& blank) = delete;
    Blank& operator=(const Blank& blank) = delete;
    Blank& operator=(Blank&& blank) = delete;

    virtual void alternativeTile(Chunk& chunk, unsigned int tileIndex) override;
    virtual void cloneTo(const Chunk& chunk, unsigned int tileIndex, Tile target) override;

private:
    void init(Chunk& chunk, unsigned int tileIndex);

    friend class ::Tile;
};

} // namespace tiles
