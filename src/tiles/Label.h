#pragma once

#include <Tile.h>
#include <TileType.h>

class Chunk;

namespace tiles {

class Label : public TileType {
public:
    static Label* instance();
    virtual ~Label() = default;
    Label(const Label& label) = delete;
    Label(Label&& label) = delete;
    Label& operator=(const Label& label) = delete;
    Label& operator=(Label&& label) = delete;

    virtual void flip(Chunk& chunk, unsigned int tileIndex, bool acrossHorizontal) override;
    virtual void alternativeTile(Chunk& chunk, unsigned int tileIndex) override;
    virtual void cloneTo(const Chunk& chunk, unsigned int tileIndex, Tile target) override;

private:
    Label();

    void init(Chunk& chunk, unsigned int tileIndex);

    friend class ::Tile;
};

} // namespace tiles
