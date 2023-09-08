#include <Chunk.h>
#include <tiles/Led.h>

#include <memory>

namespace tiles {

Led* Led::instance() {
    static std::unique_ptr<Led> led(new Led());
    return led.get();
}

void Led::setState(Chunk& chunk, unsigned int tileIndex, State::t state) {
    auto& tileData = getTileData(chunk, tileIndex);
    tileData.state1 = state;
}

void Led::flip(Chunk& /*chunk*/, unsigned int /*tileIndex*/, bool /*acrossHorizontal*/) {}

void Led::alternativeTile(Chunk& /*chunk*/, unsigned int /*tileIndex*/) {}

void Led::init(Chunk& chunk, unsigned int tileIndex, State::t state) {
    auto& tileData = getTileData(chunk, tileIndex);
    tileData.id = TileId::outLed;
    tileData.state1 = state;
    tileData.state2 = State::disconnected;
    tileData.dir = Direction::north;
    tileData.meta = 0;
}

}
