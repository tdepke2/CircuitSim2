#pragma once

#include <Chunk.h>

class TileType;
class Blank;
class Wire;

class Tile {
public:
    Tile(TileType* type, Chunk& chunk, unsigned int tileIndex);
    void setType(Blank* type);
    void setType(Wire* type, TileId::t tileId = TileId::wireStraight, Direction::t direction = Direction::north, State::t state1 = State::low, State::t state2 = State::low);
    TileType* getType();
    Chunk& getChunk();
    unsigned int getIndex() const;

    void setDirection(Direction::t direction);
    void setHighlight(bool highlight);
    void setState(State::t state);
    Direction::t getDirection() const;
    bool getHighlight() const;
    State::t getState() const;
    void flip(bool acrossHorizontal);
    void alternativeTile();

private:
    TileType* tileType_;
    Chunk& chunk_;
    unsigned int tileIndex_;
};
