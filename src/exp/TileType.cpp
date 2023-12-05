#include <TileType.h>

void TileType::setDirection(Chunk& /*chunk*/, unsigned int /*tileIndex*/, Direction::t /*direction*/) {}

void TileType::setHighlight(Chunk& chunk, unsigned int tileIndex, bool highlight) {
    modifyTileData(chunk, tileIndex).highlight = highlight;
}

void TileType::setState(Chunk& /*chunk*/, unsigned int /*tileIndex*/, State::t /*state*/) {}

TileId::t TileType::getId(Chunk& chunk, unsigned int tileIndex) const {
    return getTileData(chunk, tileIndex).id;
}

Direction::t TileType::getDirection(Chunk& chunk, unsigned int tileIndex) const {
    return getTileData(chunk, tileIndex).dir;
}

bool TileType::getHighlight(Chunk& chunk, unsigned int tileIndex) const {
    return getTileData(chunk, tileIndex).highlight;
}

State::t TileType::getState(Chunk& chunk, unsigned int tileIndex) const {
    return getTileData(chunk, tileIndex).state1;
}
