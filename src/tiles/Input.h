#pragma once

#include <Tile.h>
#include <TileType.h>

class Chunk;

namespace tiles {

class Input : public TileType {
public:
    static Input* instance();
    virtual ~Input() = default;
    Input(const Input& input) = delete;
    Input(Input&& input) = delete;
    Input& operator=(const Input& input) = delete;
    Input& operator=(Input&& input) = delete;

    void setKeycode(Chunk& chunk, unsigned int tileIndex, char keycode);
    char getKeycode(const Chunk& chunk, unsigned int tileIndex) const;

    virtual void setState(Chunk& chunk, unsigned int tileIndex, State::t state) override;
    virtual void alternativeTile(Chunk& chunk, unsigned int tileIndex) override;
    virtual void cloneTo(const Chunk& chunk, unsigned int tileIndex, Tile target) override;

private:
    Input();

    void init(Chunk& chunk, unsigned int tileIndex, TileId::t inputId, State::t state, char keycode);

    friend class ::Tile;
};

} // namespace tiles
