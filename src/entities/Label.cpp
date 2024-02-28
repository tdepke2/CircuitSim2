#include <Chunk.h>
#include <entities/Label.h>
#include <Locator.h>
#include <LodRenderer.h>
#include <ResourceBase.h>

#include <spdlog/spdlog.h>

namespace entities {

Label::Label(Chunk& chunk, unsigned int tileIndex) :
    Entity(chunk, tileIndex),
    text_("", Locator::getResource()->getFont("resources/consolas.ttf"), 30) {

    spdlog::debug("Label entity has been created.");
    chunk.getLodRenderer()->addDecoration(chunk.getCoords(), tileIndex, this);
}

Label::Label(Chunk& chunk, unsigned int tileIndex, const Label& rhs) :
    Entity(chunk, tileIndex),
    text_(rhs.text_) {

    spdlog::debug("Label entity (copy) has been created.");
    chunk.getLodRenderer()->addDecoration(chunk.getCoords(), tileIndex, this);
}

Label::~Label() {
    spdlog::debug("Label entity has been destroyed.");
    getChunk().getLodRenderer()->removeDecoration(getChunk().getCoords(), getIndex());
}

void Label::setString(const sf::String& str) {
    text_.setString(str);
}

const sf::String& Label::getString() const {
    return text_.getString();
}

std::unique_ptr<Label> Label::clone(Chunk& chunk, unsigned int tileIndex) const {
    return std::unique_ptr<Label>(new Label(chunk, tileIndex, *this));
}

void Label::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(text_, states);
}

} // namespace entities
