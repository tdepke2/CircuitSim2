#pragma once

#include <Entity.h>

#include <memory>
#include <SFML/Graphics.hpp>

namespace entities {

class Label : public Entity, public sf::Drawable {
public:
    Label(Chunk& chunk, unsigned int tileIndex);
    Label(Chunk& chunk, unsigned int tileIndex, const Label& rhs);
    virtual ~Label();

    void setString(const sf::String& str);
    const sf::String& getString() const;

    std::unique_ptr<Label> clone(Chunk& chunk, unsigned int tileIndex) const;
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
    sf::Text text_;
};

} // namespace entities
