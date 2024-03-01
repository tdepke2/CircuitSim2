#include <TileType.h>

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
