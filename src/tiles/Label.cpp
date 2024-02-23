#include <Chunk.h>
#include <tiles/Label.h>

#include <memory>
#include <spdlog/spdlog.h>

namespace tiles {

Label* Label::instance() {
    static std::unique_ptr<Label> label(new Label());
    return label.get();
}

void Label::flip(Chunk& /*chunk*/, unsigned int /*tileIndex*/, bool /*acrossHorizontal*/) {}

void Label::alternativeTile(Chunk& /*chunk*/, unsigned int /*tileIndex*/) {}

void Label::cloneTo(const Chunk& /*chunk*/, unsigned int /*tileIndex*/, Tile target) {
    init(target.getChunk(), target.getIndex());
}

Label::Label() {
    spdlog::debug("Label class has been constructed.");
}

void Label::init(Chunk& chunk, unsigned int tileIndex) {
    auto& tileData = modifyTileData(chunk, tileIndex);
    tileData.id = TileId::label;
    tileData.state1 = State::disconnected;
    tileData.state2 = State::disconnected;
    tileData.dir = Direction::north;
    tileData.meta = 0;
}

} // namespace tiles
