#include <Entity.h>

Entity::Entity(Chunk& chunk, unsigned int tileIndex) :
    chunk_(chunk),
    tileIndex_(tileIndex) {
}

Chunk& Entity::getChunk() {
    return chunk_;
}

const Chunk& Entity::getChunk() const {
    return chunk_;
}

unsigned int Entity::getIndex() const {
    return tileIndex_;
}
