#include <Chunk.h>
#include <entities/Label.h>
#include <tiles/Label.h>

#include <memory>
#include <spdlog/spdlog.h>

namespace tiles {

Label* Label::instance() {
    static std::unique_ptr<Label> label(new Label());
    return label.get();
}

const entities::Label* Label::getEntity(Chunk& chunk, unsigned int tileIndex) const {
    return reinterpret_cast<const entities::Label*>(TileType::getEntity(chunk, tileIndex));
}

entities::Label* Label::modifyEntity(Chunk& chunk, unsigned int tileIndex) {
    return reinterpret_cast<entities::Label*>(TileType::modifyEntity(chunk, tileIndex));
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
    allocateEntity(chunk, tileIndex, std::unique_ptr<Entity>(new entities::Label(chunk, tileIndex)));
}

void Label::destroy(Chunk& chunk, unsigned int tileIndex) {
    freeEntity(chunk, tileIndex);
}

} // namespace tiles
