#pragma once

class Chunk;
class TileType;

class Tile {
public:
    // should we make a pack() function here that converts to uint32_t, or does that belong in the Chunk?
    // Board needs to build a Tile from chunk data when requested.
    // A Tile needs to be packed and saved into a chunk through the board.
    // We should limit exposure to the way chunks work in the board, interface should not allow manipulation of chunks.

    Tile(TileType* type, Chunk& chunk, unsigned int tileIndex);
    bool getHighlight() const;
    void setHighlight(bool highlight);

private:
    TileType* tileType_;
    Chunk& chunk_;
    unsigned int tileIndex_;
};
