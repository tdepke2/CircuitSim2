#include <Entity.h>

Entity::Entity(Chunk& chunk, unsigned int tileIndex) :
    chunk_(&chunk),
    tileIndex_(tileIndex) {
}

void Entity::setChunkAndIndex(Chunk& chunk, unsigned int tileIndex) {
    chunk_ = &chunk;
    tileIndex_ = tileIndex;
}

Chunk& Entity::getChunk() {
    return *chunk_;
}

const Chunk& Entity::getChunk() const {
    return *chunk_;
}

unsigned int Entity::getIndex() const {
    return tileIndex_;
}

bool operator==(const Entity& lhs, const Entity& rhs) {
    // This method for polymorphic equality allows derived `Entity` classes to
    // be compared directly or through pointers/references. Derived classes just
    // need to implement `equals()` for it to work. Note that if we wanted to
    // compare data in this `Entity` base class, the `equals()` function could
    // be implemented here (it would then need to be called within the derived
    // classes).
    // 
    // Based on similar implementations found here:
    // https://stackoverflow.com/questions/1765122/equality-test-for-derived-classes-in-c
    // https://brevzin.github.io/c++/2025/03/12/polymorphic-equals/

    return typeid(lhs) == typeid(rhs) && lhs.equals(rhs);
}

bool operator!=(const Entity& lhs, const Entity& rhs) {
    return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& out, const Entity& entity) {
    entity.print(out);
    return out;
}
