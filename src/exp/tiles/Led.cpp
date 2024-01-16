#include <Chunk.h>
#include <tiles/Led.h>

#include <memory>
#include <spdlog/spdlog.h>

namespace tiles {

Led* Led::instance() {
    static std::unique_ptr<Led> led(new Led());
    return led.get();
}

void Led::setState(Chunk& chunk, unsigned int tileIndex, State::t state) {
    auto& tileData = modifyTileData(chunk, tileIndex);
    tileData.state1 = state;
}

void Led::flip(Chunk& /*chunk*/, unsigned int /*tileIndex*/, bool /*acrossHorizontal*/) {}

void Led::alternativeTile(Chunk& /*chunk*/, unsigned int /*tileIndex*/) {}

void Led::cloneTo(const Chunk& chunk, unsigned int tileIndex, Tile target) {
    const auto& tileData = getTileData(chunk, tileIndex);
    init(target.getChunk(), target.getIndex(), tileData.state1);
}

Led::Led() {
    spdlog::debug("Led class has been constructed.");
}

void Led::init(Chunk& chunk, unsigned int tileIndex, State::t state) {
    auto& tileData = modifyTileData(chunk, tileIndex);
    tileData.id = TileId::outLed;
    tileData.state1 = state;
    tileData.state2 = State::disconnected;
    tileData.dir = Direction::north;
    tileData.meta = 0;
}

}
