#pragma once

#include <Tile.h>
#include <TileType.h>





#include <iostream>

class Chunk;

namespace WireType {
    enum t : int {
        straight = 0, corner, tee, junction, crossover
    };
}

/*
class Wire : public Tile {
public:
    Wire(WireType::t type = WireType::straight, Direction::t direction = Direction::north, State::t state1 = State::low, State::t state2 = State::low);

private:
    WireType::t type_;
    Direction::t direction_;
    State::t state1_, state2_;
};*/

class Wire : public TileType {
public:
    static Wire* instance();
    Wire(const Wire& wire) = delete;
    Wire(Wire&& wire) = delete;
    Wire& operator=(const Wire& wire) = delete;
    Wire& operator=(Wire&& wire) = delete;

    virtual bool getHighlight(Chunk& chunk, unsigned int tileIndex) const override;
    virtual void setHighlight(Chunk& chunk, unsigned int tileIndex, bool highlight) override;

private:
    Wire() {
        std::cout << "Wire class has been constructed.\n";
    }
};
