#pragma once

#include <array>
#include <bitset>
#include <SFML/Graphics.hpp>

class Chunk;

class ChunkDrawable : public sf::Drawable {
public:
    static void setupTextureData(const sf::Vector2u& textureSize, unsigned int tileWidth);

    ChunkDrawable(const Chunk* chunk);
    void forceRedraw();

private:
    static unsigned int textureWidth_, tileWidth_;
    static uint8_t textureLookup_[512];
    static unsigned int textureHighlightStart_;

    void redrawTile(unsigned int x, unsigned int y);
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    const Chunk* chunk_;
    sf::VertexArray vertices_;
    std::array<int, 8> renderBlocks_;
    std::bitset<8> renderDirty_;
};
