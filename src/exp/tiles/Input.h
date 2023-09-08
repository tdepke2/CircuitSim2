#pragma once

#include <Tile.h>
#include <TileType.h>





#include <iostream>

class Chunk;

namespace tiles {

class Input : public TileType {
public:
    static Input* instance();
    Input(const Input& input) = delete;
    Input(Input&& input) = delete;
    Input& operator=(const Input& input) = delete;
    Input& operator=(Input&& input) = delete;

    virtual void setState(Chunk& chunk, unsigned int tileIndex, State::t state) override;
    virtual void flip(Chunk& chunk, unsigned int tileIndex, bool acrossHorizontal) override;
    virtual void alternativeTile(Chunk& chunk, unsigned int tileIndex) override;

private:
    Input() {
        std::cout << "Input class has been constructed.\n";
    }

    void init(Chunk& chunk, unsigned int tileIndex, TileId::t inputId, State::t state, char keycode);

    friend class ::Tile;
};

} // namespace tiles
