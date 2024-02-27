#include <Chunk.h>
#include <entities/Label.h>
#include <Locator.h>
#include <LodRenderer.h>
#include <ResourceBase.h>

#include <spdlog/spdlog.h>

namespace entities {

Label::Label(Chunk& chunk, unsigned int tileIndex) :
    Entity(chunk, tileIndex),
    str_() {

    spdlog::debug("Label entity has been created.");
    chunk.getLodRenderer()->addDecoration(chunk.getCoords(), tileIndex, this);
}

Label::~Label() {
    spdlog::debug("Label entity has been destroyed.");
    getChunk().getLodRenderer()->removeDecoration(getChunk().getCoords(), getIndex());
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

void Label::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    sf::Text text;    // FIXME: make text a member.
    text.setFont(Locator::getResource()->getFont("resources/consolas.ttf"));
    text.setString(str_);
    target.draw(text, states);
}

} // namespace entities
