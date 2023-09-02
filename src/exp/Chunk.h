#pragma once

#include <SFML/Graphics.hpp>

class Chunk : public sf::Drawable {
public:
    static constexpr unsigned int WIDTH = 8;

    Chunk(unsigned int textureWidth, unsigned int tileWidth);

private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    uint8_t tileStates_[WIDTH * WIDTH];
    sf::VertexArray vertices_;
    unsigned int textureWidth_, tileWidth_;
};
