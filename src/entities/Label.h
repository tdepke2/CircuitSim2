#pragma once

#include <Entity.h>

#include <SFML/Graphics.hpp>

namespace entities {

class Label : public Entity {
public:

private:
    sf::String text_;
};

} // namespace entities
