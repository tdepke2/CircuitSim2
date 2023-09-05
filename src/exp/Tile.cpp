#include <Tile.h>
#include <TileType.h>

Tile::Tile(TileType* type, Chunk& chunk, unsigned int tileIndex) : tileType_(type), chunk_(chunk), tileIndex_(tileIndex) {

}

bool Tile::getHighlight() const {
    return tileType_->getHighlight(chunk_, tileIndex_);
}

void Tile::setHighlight(bool highlight) {
    tileType_->setHighlight(chunk_, tileIndex_, highlight);
}
