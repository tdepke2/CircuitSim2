#pragma once

#include <FlatMap.h>

#include <cstdint>
#include <SFML/Graphics.hpp>
#include <vector>

class ChunkDrawable;

using ChunkCoords = uint64_t;

class ChunkRender : public sf::Drawable {
public:
    static constexpr int LEVELS_OF_DETAIL = 5;
    static constexpr ChunkCoords EMPTY_CHUNK_COORDS = 0x7fffffff7fffffff;
    static void setupTextureData(unsigned int tileWidth);

    ChunkRender();
    void resize(int currentLod, const sf::Vector2u& maxChunkArea);    // FIXME have currentLod be a member?
    void allocateBlock(int currentLod, FlatMap<ChunkCoords, ChunkDrawable>& chunkDrawables, ChunkCoords coords, const sf::IntRect& visibleArea);
    void drawChunk(int currentLod, const ChunkDrawable& chunkDrawable, sf::RenderStates states);
    void display();
    void updateVisibleArea(int currentLod, const FlatMap<ChunkCoords, ChunkDrawable>& chunkDrawables, const sf::IntRect& visibleArea);

private:
    static unsigned int tileWidth_;

    struct RenderBlock {
        RenderBlock(ChunkCoords coords, unsigned int poolIndex) :
            coords(coords),
            poolIndex(poolIndex),
            adjustedChebyshev(0.0f) {
        }

        ChunkCoords coords;
        unsigned int poolIndex;
        float adjustedChebyshev;
    };
    friend bool operator<(const ChunkRender::RenderBlock& lhs, const ChunkRender::RenderBlock& rhs);

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    sf::Vector2u maxChunkArea_;
    sf::IntRect lastVisibleArea_;
    sf::RenderTexture texture_;
    bool textureDirty_;
    sf::VertexArray buffer_;    // FIXME this should probably be a vbuf but need to test.
    std::vector<int> renderIndexPool_;
    std::vector<RenderBlock> renderBlocks_;
};
