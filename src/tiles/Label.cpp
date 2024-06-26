#include <Chunk.h>
#include <entities/Label.h>
#include <MakeUnique.h>
#include <tiles/Label.h>

#include <memory>
#include <spdlog/spdlog.h>

namespace tiles {

Label* Label::instance() {
    static auto label = details::make_unique<Label>(Private());
    return label.get();
}

Label::Label(Private) {
    spdlog::debug("Label class has been constructed.");
}

const entities::Label* Label::getEntity(const Chunk& chunk, unsigned int tileIndex) const {
    return static_cast<const entities::Label*>(TileType::getEntity(chunk, tileIndex));
}

entities::Label* Label::modifyEntity(Chunk& chunk, unsigned int tileIndex) {
    return static_cast<entities::Label*>(TileType::modifyEntity(chunk, tileIndex));
}

bool Label::isTileEntity() const {
    return true;
}

void Label::alternativeTile(Chunk& /*chunk*/, unsigned int /*tileIndex*/) {}

void Label::cloneTo(const Chunk& chunk, unsigned int tileIndex, Tile target) {
    init(target.getChunk(), target.getIndex(), getEntity(chunk, tileIndex));
}

void Label::init(Chunk& chunk, unsigned int tileIndex, const entities::Label* labelToCopy) {
    auto& tileData = modifyTileData(chunk, tileIndex);
    tileData.id = TileId::label;
    tileData.state1 = State::disconnected;
    tileData.state2 = State::disconnected;
    tileData.dir = Direction::north;
    if (labelToCopy == nullptr) {
        allocateEntity(chunk, tileIndex, details::make_unique<entities::Label>(chunk, tileIndex));
    } else {
        allocateEntity(chunk, tileIndex, labelToCopy->clone(chunk, tileIndex));
    }
}

void Label::destroy(Chunk& chunk, unsigned int tileIndex) {
    freeEntity(chunk, tileIndex);
}

} // namespace tiles
