#include <Chunk.h>
#include <tiles/Blank.h>

#include <memory>
#include <spdlog/spdlog.h>

namespace tiles {

Blank* Blank::instance() {
    static std::unique_ptr<Blank> blank(new Blank());
    return blank.get();
}

void Blank::alternativeTile(Chunk& /*chunk*/, unsigned int /*tileIndex*/) {}

void Blank::cloneTo(const Chunk& /*chunk*/, unsigned int /*tileIndex*/, Tile target) {
    init(target.getChunk(), target.getIndex());
}

Blank::Blank() {
    spdlog::debug("Blank class has been constructed.");
}

void Blank::init(Chunk& chunk, unsigned int tileIndex) {
    auto& tileData = modifyTileData(chunk, tileIndex);
    tileData.id = TileId::blank;
    tileData.state1 = State::disconnected;
    tileData.state2 = State::disconnected;
    tileData.dir = Direction::north;
    tileData.meta = 0;
}

} // namespace tiles
