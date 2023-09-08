#pragma once

#include <Tile.h>
#include <TileType.h>





#include <iostream>

class Chunk;

namespace tiles {

class Led : public TileType {
public:
    static Led* instance();
    Led(const Led& led) = delete;
    Led(Led&& led) = delete;
    Led& operator=(const Led& led) = delete;
    Led& operator=(Led&& led) = delete;

    virtual void setState(Chunk& chunk, unsigned int tileIndex, State::t state) override;
    virtual void flip(Chunk& chunk, unsigned int tileIndex, bool acrossHorizontal) override;
    virtual void alternativeTile(Chunk& chunk, unsigned int tileIndex) override;

private:
    Led() {
        std::cout << "Led class has been constructed.\n";
    }

    void init(Chunk& chunk, unsigned int tileIndex, State::t state);

    friend class ::Tile;
};

} // namespace tiles
