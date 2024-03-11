#include <TileType.h>

#include <spdlog/spdlog.h>
#include <utility>

void TileType::destroy(Chunk& /*chunk*/, unsigned int /*tileIndex*/) {}

void TileType::setDirection(Chunk& /*chunk*/, unsigned int /*tileIndex*/, Direction::t /*direction*/) {}

void TileType::setHighlight(Chunk& chunk, unsigned int tileIndex, bool highlight) {
    // Special case for highlighting as highlights do not mark the chunk as unsaved.
    chunk.markHighlightDirty(tileIndex);
    chunk.tiles_[tileIndex].highlight = highlight;
}

void TileType::setState(Chunk& /*chunk*/, unsigned int /*tileIndex*/, State::t /*state*/) {}

TileId::t TileType::getId(const Chunk& chunk, unsigned int tileIndex) const {
    return getTileData(chunk, tileIndex).id;
}

Direction::t TileType::getDirection(const Chunk& chunk, unsigned int tileIndex) const {
    return getTileData(chunk, tileIndex).dir;
}

bool TileType::getHighlight(const Chunk& chunk, unsigned int tileIndex) const {
    return getTileData(chunk, tileIndex).highlight;
}

State::t TileType::getState(const Chunk& chunk, unsigned int tileIndex) const {
    return getTileData(chunk, tileIndex).state1;
}

TileData TileType::getRawData(const Chunk& chunk, unsigned int tileIndex) const {
    return chunk.tiles_[tileIndex];
}

bool TileType::isTileEntity() const {
    return false;
}

void TileType::flip(Chunk& /*chunk*/, unsigned int /*tileIndex*/, bool /*acrossVertical*/) {}

void TileType::swapWith(Chunk& chunk, unsigned int tileIndex, Tile target) {
    spdlog::debug("TileType::swapWith called on chunk {} tile {} with target chunk {} tile {}.", static_cast<void*>(&chunk), tileIndex, static_cast<void*>(&target.getChunk()), target.getIndex());

    using std::swap;
    if (isTileEntity()) {
        if (target.getType()->isTileEntity()) {
            spdlog::debug("Both are tile entities.");
            auto& first = chunk.entities_[chunk.tiles_[tileIndex].meta];
            auto& second = target.getChunk().entities_[target.getChunk().tiles_[target.getIndex()].meta];
            first->setChunkAndIndex(target.getChunk(), target.getIndex());
            second->setChunkAndIndex(chunk, tileIndex);
            swap(first, second);

            const auto firstMeta = chunk.tiles_[tileIndex].meta;
            chunk.tiles_[tileIndex].meta = target.getChunk().tiles_[target.getIndex()].meta;
            target.getChunk().tiles_[target.getIndex()].meta = firstMeta;

            swap(modifyTileData(chunk, tileIndex), modifyTileData(target.getChunk(), target.getIndex()));
        } else {
            spdlog::debug("Only the first tile has an entity.");
            auto first = std::move(chunk.entities_[chunk.tiles_[tileIndex].meta]);
            first->setChunkAndIndex(target.getChunk(), target.getIndex());
            chunk.freeEntity(tileIndex);
            swap(modifyTileData(chunk, tileIndex), modifyTileData(target.getChunk(), target.getIndex()));
            target.getChunk().allocateEntity(target.getIndex(), std::move(first));
        }
    } else if (target.getType()->isTileEntity()) {
        spdlog::debug("Only the second tile has an entity.");
        auto second = std::move(target.getChunk().entities_[target.getChunk().tiles_[target.getIndex()].meta]);
        second->setChunkAndIndex(chunk, tileIndex);
        target.getChunk().freeEntity(target.getIndex());
        swap(modifyTileData(chunk, tileIndex), modifyTileData(target.getChunk(), target.getIndex()));
        chunk.allocateEntity(tileIndex, std::move(second));
    } else {
        swap(modifyTileData(chunk, tileIndex), modifyTileData(target.getChunk(), target.getIndex()));
    }
}
