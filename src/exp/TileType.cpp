#include <TileType.h>

void TileType::setDirection(Chunk& /*chunk*/, unsigned int /*tileIndex*/, Direction::t /*direction*/) {}

void TileType::setHighlight(Chunk& chunk, unsigned int tileIndex, bool highlight) {
    chunk.tiles_[tileIndex].highlight = highlight;
}

void TileType::setState(Chunk& /*chunk*/, unsigned int /*tileIndex*/, State::t /*state*/) {}

Direction::t TileType::getDirection(Chunk& chunk, unsigned int tileIndex) const {
    return chunk.tiles_[tileIndex].dir;
}

bool TileType::getHighlight(Chunk& chunk, unsigned int tileIndex) const {
    return chunk.tiles_[tileIndex].highlight;
}

State::t TileType::getState(Chunk& chunk, unsigned int tileIndex) const {
    return chunk.tiles_[tileIndex].state1;
}

TileData& TileType::getTileData(Chunk& chunk, unsigned int tileIndex) {
    return chunk.tiles_[tileIndex];
}
