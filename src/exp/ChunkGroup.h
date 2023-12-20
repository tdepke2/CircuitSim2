#pragma once

#include <Chunk.h>
#include <ChunkCoords.h>
#include <ChunkCoordsRange.h>
#include <ChunkDrawable.h>
#include <FlatMap.h>

#include <SFML/Graphics.hpp>
#include <unordered_map>

class OffsetView;

// FIXME rename to SubBoard?

class ChunkGroup : public sf::Drawable, public sf::Transformable {
public:
    static void setup(unsigned int tileWidth);

    ChunkGroup();
    ~ChunkGroup() = default;
    ChunkGroup(const ChunkGroup& rhs) = delete;
    ChunkGroup& operator=(const ChunkGroup& rhs) = delete;

    void setSize(const sf::Vector2u& size);
    void setRenderArea(const OffsetView& offsetView, float zoom);
    void drawChunks(sf::RenderStates states);

private:
    static unsigned int tileWidth_;

    void updateVisibleArea(const OffsetView& offsetView);
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    sf::Vector2u size_;
    std::unordered_map<ChunkCoords::repr, Chunk> chunks_;
    FlatMap<ChunkCoords::repr, ChunkDrawable> chunkDrawables_;
    int levelOfDetail_;
    ChunkCoordsRange lastVisibleArea_;
    bool visibleAreaChanged_;
    sf::RenderTexture texture_;
    sf::VertexArray vertices_;
};
