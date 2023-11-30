#pragma once

#include <ChunkCoords.h>
#include <ChunkCoordsRange.h>
#include <FlatMap.h>

#include <cstdint>
#include <SFML/Graphics.hpp>
#include <vector>

class ChunkDrawable;

class ChunkRender : public sf::Drawable {
public:
    static constexpr int LEVELS_OF_DETAIL = 5;
    static constexpr ChunkCoords::repr EMPTY_CHUNK_COORDS = 0;
    static void setupTextureData(unsigned int tileWidth);

    ChunkRender();
    ~ChunkRender() = default;
    ChunkRender(const ChunkRender& rhs) = delete;
    ChunkRender& operator=(const ChunkRender& rhs) = delete;

    void setLod(int levelOfDetail);
    int getLod() const;
    void resize(FlatMap<ChunkCoords::repr, ChunkDrawable>& chunkDrawables, const sf::Vector2u& maxChunkArea);
    void allocateBlock(FlatMap<ChunkCoords::repr, ChunkDrawable>& chunkDrawables, ChunkCoords::repr coords, const ChunkCoordsRange& visibleArea);
    void drawChunk(const ChunkDrawable& chunkDrawable, sf::RenderStates states);
    void display();
    void updateVisibleArea(const FlatMap<ChunkCoords::repr, ChunkDrawable>& chunkDrawables, const ChunkCoordsRange& visibleArea);

private:
    static constexpr int CHUNK_PADDING = 1;
    static unsigned int tileWidth_;

    struct RenderBlock {
        RenderBlock(ChunkCoords::repr coords, unsigned int poolIndex) :
            coords(coords),
            poolIndex(poolIndex),
            adjustedChebyshev(0.0f) {
        }

        ChunkCoords::repr coords;
        unsigned int poolIndex;
        float adjustedChebyshev;
    };
    friend bool operator<(const ChunkRender::RenderBlock& lhs, const ChunkRender::RenderBlock& rhs);

    sf::Vector2f getChunkTexCoords(int renderIndex, int textureSubdivisionSize) const;
    void sortRenderBlocks();
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    int levelOfDetail_;
    sf::Vector2u maxChunkArea_;
    ChunkCoordsRange lastVisibleArea_;
    sf::RenderTexture texture_;
    bool textureDirty_;
    sf::VertexBuffer buffer_;
    std::vector<sf::Vertex> bufferVertices_;
    std::vector<int> renderIndexPool_;
    std::vector<RenderBlock> renderBlocks_;
};
