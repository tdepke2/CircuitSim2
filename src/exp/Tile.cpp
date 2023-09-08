#include <Chunk.h>
#include <Tile.h>
#include <tiles/Blank.h>
#include <tiles/Gate.h>
#include <tiles/Input.h>
#include <tiles/Led.h>
#include <tiles/Wire.h>
#include <TileType.h>

Tile::Tile(TileType* type, Chunk& chunk, unsigned int tileIndex) :
    tileType_(type), chunk_(chunk), tileIndex_(tileIndex) {
}

void Tile::setType(tiles::Blank* type) {
    tileType_ = type;
    type->init(chunk_, tileIndex_);
}

void Tile::setType(tiles::Gate* type, TileId::t gateId, Direction::t direction, State::t state) {
    tileType_ = type;
    type->init(chunk_, tileIndex_, gateId, direction, state);
}

void Tile::setType(tiles::Input* type, TileId::t inputId, State::t state, char keycode) {
    tileType_ = type;
    type->init(chunk_, tileIndex_, inputId, state, keycode);
}

void Tile::setType(tiles::Led* type, State::t state) {
    tileType_ = type;
    type->init(chunk_, tileIndex_, state);
}

void Tile::setType(tiles::Wire* type, TileId::t wireId, Direction::t direction, State::t state1, State::t state2) {
    tileType_ = type;
    type->init(chunk_, tileIndex_, wireId, direction, state1, state2);
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
