#pragma once

#include <Tile.h>

class Chunk;
struct TileData;

class TileType {
public:
    virtual void setDirection(Chunk& chunk, unsigned int tileIndex, Direction::t direction);
    virtual void setHighlight(Chunk& chunk, unsigned int tileIndex, bool highlight);
    virtual void setState(Chunk& chunk, unsigned int tileIndex, State::t state);
    virtual Direction::t getDirection(Chunk& chunk, unsigned int tileIndex) const;
    virtual bool getHighlight(Chunk& chunk, unsigned int tileIndex) const;
    virtual State::t getState(Chunk& chunk, unsigned int tileIndex) const;
    virtual void flip(Chunk& chunk, unsigned int tileIndex, bool acrossHorizontal) = 0;
    virtual void alternativeTile(Chunk& chunk, unsigned int tileIndex) = 0;

protected:
    TileData& getTileData(Chunk& chunk, unsigned int tileIndex);
};
