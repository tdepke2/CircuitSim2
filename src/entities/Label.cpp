#include <Chunk.h>
#include <entities/Label.h>
#include <LodRenderer.h>

#include <spdlog/spdlog.h>

namespace entities {

Label::Label(Chunk& chunk, unsigned int tileIndex) :
    Entity(chunk, tileIndex),
    str_() {

    spdlog::debug("Label entity has been created.");
    chunk.getLodRenderer()->addRenderable(this);
}

Label::~Label() {
    spdlog::debug("Label entity has been destroyed.");
    getChunk().getLodRenderer()->removeRenderable(this);
}

void Label::setText(const sf::String& str) {
    str_ = str;
}

const sf::String& Label::getText() const {
    return str_;
}

std::unique_ptr<Label> Label::clone(Chunk& chunk, unsigned int tileIndex) const {
    // FIXME unfinished, need to actually make a copy.
    return std::unique_ptr<Label>(new Label(chunk, tileIndex));
}

} // namespace entities
