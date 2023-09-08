#pragma once

#include <cstdint>

class Chunk;
class TileType;

namespace tiles {
    class Blank;
    class Gate;
    class Input;
    class Led;
    class Wire;
}

namespace TileId {
    enum t : uint8_t {
        blank = 0,
        wireStraight, wireCorner, wireTee, wireJunction, wireCrossover,
        inSwitch, inButton, outLed,
        gateDiode, gateBuffer, gateNot, gateAnd, gateNand, gateOr, gateNor, gateXor, gateXnor,
        count
    };
}

namespace State {
    enum t : uint8_t {
        disconnected = 0, low, high, middle
    };
}

namespace Direction {
    enum t : uint8_t {
        north = 0, east, south, west
    };
}

class Tile {
public:
    Tile(TileType* type, Chunk& chunk, unsigned int tileIndex);
    void setType(tiles::Blank* type);
    void setType(tiles::Gate* type, TileId::t gateId = TileId::gateDiode, Direction::t direction = Direction::north, State::t state = State::low);
    void setType(tiles::Input* type, TileId::t inputId = TileId::inSwitch, State::t state = State::low, char keycode = ' ');
    void setType(tiles::Led* type, State::t state = State::low);
    void setType(tiles::Wire* type, TileId::t wireId = TileId::wireStraight, Direction::t direction = Direction::north, State::t state1 = State::low, State::t state2 = State::low);
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
