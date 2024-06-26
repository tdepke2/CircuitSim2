#include <Chunk.h>
#include <MakeUnique.h>
#include <tiles/Gate.h>

#include <cassert>
#include <memory>
#include <spdlog/spdlog.h>

namespace tiles {

Gate* Gate::instance() {
    static auto gate = details::make_unique<Gate>(Private());
    return gate.get();
}

Gate::Gate(Private) {
    spdlog::debug("Gate class has been constructed.");
}

void Gate::setDirection(Chunk& chunk, unsigned int tileIndex, Direction::t direction) {
    auto& tileData = modifyTileData(chunk, tileIndex);
    tileData.dir = direction;
}

void Gate::setState(Chunk& chunk, unsigned int tileIndex, State::t state) {
    auto& tileData = modifyTileData(chunk, tileIndex);
    tileData.state1 = state;
}

void Gate::flip(Chunk& chunk, unsigned int tileIndex, bool acrossVertical) {
    auto& tileData = modifyTileData(chunk, tileIndex);
    if ((acrossVertical && tileData.dir % 2 == 1) || (!acrossVertical && tileData.dir % 2 == 0)) {
        tileData.dir = static_cast<Direction::t>((tileData.dir + 2) % 4);
    }
}

void Gate::alternativeTile(Chunk& chunk, unsigned int tileIndex) {
    auto& tileData = modifyTileData(chunk, tileIndex);
    if (tileData.id != TileId::gateDiode) {
        if ((tileData.id - TileId::gateBuffer) % 2 == 0) {
            tileData.id = static_cast<TileId::t>(tileData.id + 1);
        } else {
            tileData.id = static_cast<TileId::t>(tileData.id - 1);
        }
    }
}

void Gate::cloneTo(const Chunk& chunk, unsigned int tileIndex, Tile target) {
    const auto& tileData = getTileData(chunk, tileIndex);
    init(target.getChunk(), target.getIndex(), tileData.id, tileData.dir, tileData.state1);
}

void Gate::init(Chunk& chunk, unsigned int tileIndex, TileId::t gateId, Direction::t direction, State::t state) {
    auto& tileData = modifyTileData(chunk, tileIndex);
    assert(gateId >= TileId::gateDiode && gateId <= TileId::gateXnor);
    tileData.id = gateId;
    tileData.state1 = state;
    tileData.state2 = State::disconnected;
    tileData.dir = direction;
    tileData.meta = 0;
}

} // namespace tiles
