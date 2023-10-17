#pragma once

#include <ChunkRender.h>

#include <array>
#include <bitset>
#include <SFML/Graphics.hpp>

class Chunk;

class ChunkDrawable : public sf::Drawable {
public:
    static void setupTextureData(const sf::Vector2u& textureSize, unsigned int tileWidth);

    ChunkDrawable();
    void setChunk(const Chunk* chunk);
    const Chunk* getChunk() const;
    void setRenderIndex(int currentLod, int renderIndex);
    int getRenderIndex(int currentLod) const;
    bool hasAnyRenderIndex() const;
    void markDirty();
    bool isRenderDirty(int currentLod) const;

private:
    static unsigned int textureWidth_, tileWidth_;
    static uint8_t textureLookup_[512];
    static unsigned int textureHighlightStart_;

    void redrawTile(unsigned int x, unsigned int y) const;
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    const Chunk* chunk_;
    mutable sf::VertexArray vertices_;
    std::array<int, ChunkRender::LEVELS_OF_DETAIL> renderIndices_;
    int renderIndicesSum_;
    mutable std::bitset<ChunkRender::LEVELS_OF_DETAIL> renderDirty_;

    friend class ChunkRender;
};
