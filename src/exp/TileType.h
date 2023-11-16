#pragma once

#include <Chunk.h>
#include <Tile.h>

struct TileData;

class TileType {
public:
    TileType() = default;
    TileType(const TileType& rhs) = delete;
    TileType& operator=(const TileType& rhs) = delete;

    virtual void setDirection(Chunk& chunk, unsigned int tileIndex, Direction::t direction);
    virtual void setHighlight(Chunk& chunk, unsigned int tileIndex, bool highlight);
    virtual void setState(Chunk& chunk, unsigned int tileIndex, State::t state);
    virtual TileId::t getId(Chunk& chunk, unsigned int tileIndex) const;
    virtual Direction::t getDirection(Chunk& chunk, unsigned int tileIndex) const;
    virtual bool getHighlight(Chunk& chunk, unsigned int tileIndex) const;
    virtual State::t getState(Chunk& chunk, unsigned int tileIndex) const;
    virtual void flip(Chunk& chunk, unsigned int tileIndex, bool acrossHorizontal) = 0;
    virtual void alternativeTile(Chunk& chunk, unsigned int tileIndex) = 0;

protected:
    ~TileType() = default;

    inline TileData& getTileData(Chunk& chunk, unsigned int tileIndex) {
        return chunk.tiles_[tileIndex];
    }
    inline const TileData& getTileData(Chunk& chunk, unsigned int tileIndex) const {
        return chunk.tiles_[tileIndex];
    }
};
