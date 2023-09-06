#include <Tile.h>
#include <TileType.h>
#include <tiles/Blank.h>
#include <tiles/Wire.h>

Tile::Tile(TileType* type, Chunk& chunk, unsigned int tileIndex) :
    tileType_(type), chunk_(chunk), tileIndex_(tileIndex) {
}

void Tile::setType(Blank* type) {
    tileType_ = type;
    type->init(chunk_, tileIndex_);
}

void Tile::setType(Wire* type, TileId::t tileId, Direction::t direction, State::t state1, State::t state2) {
    tileType_ = type;
    type->init(chunk_, tileIndex_, tileId, direction, state1, state2);
}

TileType* Tile::getType() {
    return tileType_;
}

Chunk& Tile::getChunk() {
    return chunk_;
}

unsigned int Tile::getIndex() const {
    return tileIndex_;
}

void Tile::setDirection(Direction::t direction) {
    tileType_->setDirection(chunk_, tileIndex_, direction);
}

void Tile::setHighlight(bool highlight) {
    tileType_->setHighlight(chunk_, tileIndex_, highlight);
}

void Tile::setState(State::t state) {
    tileType_->setState(chunk_, tileIndex_, state);
}

Direction::t Tile::getDirection() const {
    return tileType_->getDirection(chunk_, tileIndex_);
}

bool Tile::getHighlight() const {
    return tileType_->getHighlight(chunk_, tileIndex_);
}

State::t Tile::getState() const {
    return tileType_->getState(chunk_, tileIndex_);
}

void Tile::flip(bool acrossHorizontal) {
    tileType_->flip(chunk_, tileIndex_, acrossHorizontal);
}

void Tile::alternativeTile() {
    tileType_->alternativeTile(chunk_, tileIndex_);
}
