#pragma once

#include <ChunkRender.h>

#include <array>
#include <bitset>
#include <SFML/Graphics.hpp>

class Chunk;
class ResourceManager;

/**
 * The drawable component of a `Chunk`.
 * 
 * This class is fairly lightweight compared to the rest of the chunk and is
 * more suitable to store in a cache-friendly data structure (such as a
 * `FlatMap`). The render index functions refer to the index of a `RenderBlock`
 * within a `ChunkRender` for each level-of-detail.
 */
class ChunkDrawable : public sf::Drawable {
public:
    static void setupTextureData(ResourceManager& resource, const sf::Vector2u& textureSize, unsigned int tileWidth);

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
    void markAsDrawn(int levelOfDetail) const;

private:
    static unsigned int textureWidth_, tileWidth_;
    static std::array<uint8_t, 512> textureLookup_;
    static unsigned int textureHighlightStart_;
    static std::array<sf::Text, 21> labelCache_;

    void updateTileGeometry(unsigned int tileIndex) const;
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    const Chunk* chunk_;
    mutable sf::VertexArray vertices_;
    std::array<int, ChunkRender::LEVELS_OF_DETAIL> renderIndices_;
    int renderIndicesSum_;
    mutable std::bitset<ChunkRender::LEVELS_OF_DETAIL> renderDirty_;
};
