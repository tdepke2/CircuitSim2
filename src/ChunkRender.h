#pragma once

#include <ChunkCoords.h>
#include <ChunkCoordsRange.h>
#include <FlatMap.h>

#include <cstdint>
#include <SFML/Graphics.hpp>
#include <vector>

class ChunkDrawable;

/**
 * Cache for rendered chunks at a specific level-of-detail (LOD).
 * 
 * The `ChunkRender` allocates a render texture that can accommodate all visible
 * chunks at the furthest zoom for the LOD. The texture is also divided up into
 * blocks that can be allocated as needed for new chunks that enter the visible
 * area (each tracked by a `RenderBlock`). By using blocks, changing the visible
 * area does not mean redrawing the whole texture, instead we just need to
 * update the vertex buffer to point to the texture coordinates of each block.
 * 
 * The render blocks are also kept sorted by their Chebyshev distance to the
 * coordinates at the center. When a block needs to be swapped out to make room
 * for more, the highest Chebyshev distance will be chosen (which should be the
 * block furthest off screen).
 */
class ChunkRender : public sf::Drawable {
public:
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
    void updateVisibleArea(const FlatMap<ChunkCoords::repr, ChunkDrawable>& chunkDrawables, const ChunkCoordsRange& visibleArea, ChunkCoords::repr topLeft, const sf::Transform& viewProjection);

private:
    struct StaticInit {
        StaticInit();
        sf::Shader* chunkShader;
    };
    static StaticInit* staticInit_;

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
    ChunkCoords::repr lastTopLeft_;
    sf::Transform lastViewProjection_;
    sf::RenderTexture texture_;
    bool textureDirty_;
    sf::VertexBuffer buffer_;
    std::vector<sf::Vertex> bufferVertices_;
    bool bufferDirty_;
    std::vector<int> renderIndexPool_;
    std::vector<RenderBlock> renderBlocks_;
};
