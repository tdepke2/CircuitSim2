#include <Chunk.h>
#include <tiles/Wire.h>

#include <cassert>
#include <memory>
#include <spdlog/spdlog.h>

namespace tiles {

Wire* Wire::instance() {
    static std::unique_ptr<Wire> wire(new Wire());
    return wire.get();
}

void Wire::setDirection(Chunk& chunk, unsigned int tileIndex, Direction::t direction) {
    auto& tileData = getTileData(chunk, tileIndex);
    if (tileData.id == TileId::wireStraight) {
        tileData.dir = static_cast<Direction::t>(direction % 2);
    } else if (tileData.id == TileId::wireJunction) {
    } else if (tileData.id != TileId::wireCrossover) {
        tileData.dir = direction;
    } else if (direction % 2 == 1) {
        // When a crossover wire rotates an odd number of times, the two states just flip.
        auto tempState = tileData.state1;
        tileData.state1 = tileData.state2;
        tileData.state2 = tempState;
    }
}

void Wire::setState(Chunk& chunk, unsigned int tileIndex, State::t state) {
    auto& tileData = getTileData(chunk, tileIndex);
    tileData.state1 = state;
    if (tileData.id == TileId::wireCrossover) {
        tileData.state2 = state;
    }
}

void Wire::flip(Chunk& chunk, unsigned int tileIndex, bool acrossHorizontal) {
    auto& tileData = getTileData(chunk, tileIndex);
    if (tileData.id == TileId::wireCorner) {
        if (!acrossHorizontal) {
            tileData.dir = static_cast<Direction::t>(3 - tileData.dir);
        } else if (tileData.dir % 2 == 0) {
            tileData.dir = static_cast<Direction::t>(tileData.dir + 1);
        } else {
            tileData.dir = static_cast<Direction::t>(tileData.dir - 1);
        }
    } else if (tileData.id == TileId::wireTee && ((!acrossHorizontal && tileData.dir % 2 == 0) || (acrossHorizontal && tileData.dir % 2 == 1))) {
        tileData.dir = static_cast<Direction::t>((tileData.dir + 2) % 4);
    }
}

void Wire::alternativeTile(Chunk& chunk, unsigned int tileIndex) {
    auto& tileData = getTileData(chunk, tileIndex);
    if (tileData.id == TileId::wireJunction) {
        tileData.id = TileId::wireCrossover;
        tileData.state2 = tileData.state1;
    } else if (tileData.id == TileId::wireCrossover) {
        tileData.id = TileId::wireJunction;
        tileData.state2 = State::low;
    }
}

Wire::Wire() {
    spdlog::debug("Wire class has been constructed.");
}

void Wire::init(Chunk& chunk, unsigned int tileIndex, TileId::t wireId, Direction::t direction, State::t state1, State::t state2) {
    auto& tileData = getTileData(chunk, tileIndex);
    tileData.id = wireId;
    tileData.state1 = state1;
    if (wireId == TileId::wireCrossover) {
        tileData.state2 = state2;
    } else {
        tileData.state2 = State::low;
    }

    if (wireId == TileId::wireStraight) {
        tileData.dir = static_cast<Direction::t>(direction % 2);
    } else if (wireId == TileId::wireJunction || wireId == TileId::wireCrossover) {
        tileData.dir = Direction::north;
    } else if (wireId == TileId::wireCorner || wireId == TileId::wireTee) {
        tileData.dir = direction;
    } else {
        assert(false);
    }
    tileData.meta = 0;
}

}
