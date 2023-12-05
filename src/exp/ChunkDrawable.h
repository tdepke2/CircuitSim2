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
    ~ChunkDrawable() = default;
    ChunkDrawable(const ChunkDrawable& rhs) = delete;
    ChunkDrawable(ChunkDrawable&& rhs) noexcept = default;
    ChunkDrawable& operator=(const ChunkDrawable& rhs) = delete;
    ChunkDrawable& operator=(ChunkDrawable&& rhs) noexcept = default;

    void setChunk(const Chunk* chunk);
    const Chunk* getChunk() const;
    void setRenderIndex(int levelOfDetail, int renderIndex);
    int getRenderIndex(int levelOfDetail) const;
    bool hasAnyRenderIndex() const;
    void markDirty();
    bool isRenderDirty(int levelOfDetail) const;

private:
    static unsigned int textureWidth_, tileWidth_;
    static uint8_t textureLookup_[512];
    static unsigned int textureHighlightStart_;

    void redrawTile(unsigned int tileIndex) const;
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    const Chunk* chunk_;
    mutable sf::VertexArray vertices_;
    std::array<int, ChunkRender::LEVELS_OF_DETAIL> renderIndices_;
    int renderIndicesSum_;
    mutable std::bitset<ChunkRender::LEVELS_OF_DETAIL> renderDirty_;

    friend class ChunkRender;
};
