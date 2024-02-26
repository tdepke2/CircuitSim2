#pragma once

#include <Entity.h>

#include <memory>
#include <SFML/Graphics.hpp>

namespace entities {

class Label : public Entity {
public:
    Label(Chunk& chunk, unsigned int tileIndex);
    virtual ~Label();

    void setText(const sf::String& str);
    const sf::String& getText() const;

    std::unique_ptr<Label> clone(Chunk& chunk, unsigned int tileIndex) const;

private:
    sf::String str_;
};

} // namespace entities
