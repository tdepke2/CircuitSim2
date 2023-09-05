#pragma once

class Chunk;

class TileType {
public:
    virtual bool getHighlight(Chunk& chunk, unsigned int tileIndex) const = 0;
    virtual void setHighlight(Chunk& chunk, unsigned int tileIndex, bool highlight) = 0;
};
