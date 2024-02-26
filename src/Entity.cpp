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

void Entity::render() const {


    // FIXME: left off here.
    // it may not make sense to have this function in Entity, may be best to separate.
    // was considering having entities::Label inherit sf::Drawable, but we also need to do something fancy to get the text in the Label to have a font.
    // actually, the Label can just grab the font by going through the Locator... then it might make sense to change the type for renderableEntities_ to something that is drawable-specific and not just an entity.


}
