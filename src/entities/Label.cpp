#include <Chunk.h>
#include <entities/Label.h>
#include <Locator.h>
#include <LodRenderer.h>
#include <MakeUnique.h>
#include <ResourceBase.h>

#include <spdlog/spdlog.h>

namespace entities {

Label::Label(Chunk& chunk, unsigned int tileIndex) :
    Entity(chunk, tileIndex),
    text_("", Locator::getResource()->getFont("resources/consolas.ttf"), 30) {

    spdlog::debug("Label entity has been created.");
    if (chunk.getLodRenderer() != nullptr) {
        chunk.getLodRenderer()->addDecoration(chunk.getCoords(), tileIndex, this);
    }
}

Label::Label(Chunk& chunk, unsigned int tileIndex, const Label& rhs) :
    Entity(chunk, tileIndex),
    text_(rhs.text_) {

    spdlog::debug("Label entity (copy) has been created.");
    if (chunk.getLodRenderer() != nullptr) {
        chunk.getLodRenderer()->addDecoration(chunk.getCoords(), tileIndex, this);
    }
}

Label::~Label() {
    spdlog::debug("Label entity has been destroyed.");
    if (getChunk().getLodRenderer() != nullptr) {
        getChunk().getLodRenderer()->removeDecoration(getChunk().getCoords(), getIndex(), this);
    }
}

void Label::setChunkAndIndex(Chunk& chunk, unsigned int tileIndex) {
    spdlog::debug("Label::setChunkAndIndex for chunk {} index {} (label text = {}).", static_cast<void*>(&chunk), tileIndex, text_.getString().toAnsiString());
    if (getChunk().getLodRenderer() != nullptr) {
        getChunk().getLodRenderer()->removeDecoration(getChunk().getCoords(), getIndex(), this);
    }
    Entity::setChunkAndIndex(chunk, tileIndex);
    if (chunk.getLodRenderer() != nullptr) {
        chunk.getLodRenderer()->addDecoration(chunk.getCoords(), tileIndex, this);
    }
}

void Label::setString(const sf::String& str) {
    text_.setString(str);
}

const sf::String& Label::getString() const {
    return text_.getString();
}

std::vector<char> Label::serialize() const {
    std::string s = text_.getString().toAnsiString();    // FIXME: may want to use toUtf32() instead, so that we don't try to convert the chars and wide chars still work?
    return {s.begin(), s.end()};
}

void Label::deserialize(const std::vector<char>& data) {
    text_.setString(std::string(data.begin(), data.end()));
}

std::unique_ptr<Entity> Label::clone(Chunk& chunk, unsigned int tileIndex) const {
    return details::make_unique<Label>(chunk, tileIndex, *this);
}

void Label::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(text_, states);
}

bool Label::equals(const Entity& rhs) const {
    auto& rhsLabel = static_cast<const Label&>(rhs);
    return text_.getString() == rhsLabel.text_.getString();
}

void Label::print(std::ostream& out) const {
    out << "Label (text = \"" << text_.getString().toAnsiString() << "\")";
}

} // namespace entities
