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

const entities::Label* Label::getEntity(const Chunk& chunk, unsigned int tileIndex) const {
    return static_cast<const entities::Label*>(TileType::getEntity(chunk, tileIndex));
}

entities::Label* Label::modifyEntity(Chunk& chunk, unsigned int tileIndex) {
    return static_cast<entities::Label*>(TileType::modifyEntity(chunk, tileIndex));
}

bool Label::isTileEntity() const {
    return true;
}

void Label::flip(Chunk& /*chunk*/, unsigned int /*tileIndex*/, bool /*acrossHorizontal*/) {}

void Label::alternativeTile(Chunk& /*chunk*/, unsigned int /*tileIndex*/) {}

void Label::cloneTo(const Chunk& chunk, unsigned int tileIndex, Tile target) {
    init(target.getChunk(), target.getIndex(), getEntity(chunk, tileIndex));
}

Label::Label() {
    spdlog::debug("Label class has been constructed.");
}

void Label::init(Chunk& chunk, unsigned int tileIndex, const entities::Label* labelToCopy) {
    auto& tileData = modifyTileData(chunk, tileIndex);
    tileData.id = TileId::label;
    tileData.state1 = State::disconnected;
    tileData.state2 = State::disconnected;
    tileData.dir = Direction::north;
    if (labelToCopy == nullptr) {
        allocateEntity(chunk, tileIndex, std::unique_ptr<Entity>(new entities::Label(chunk, tileIndex)));
    } else {
        allocateEntity(chunk, tileIndex, labelToCopy->clone(chunk, tileIndex));
    }
}

void Label::destroy(Chunk& chunk, unsigned int tileIndex) {
    freeEntity(chunk, tileIndex);
}

} // namespace tiles