#pragma once

#include <cstdint>
#include <SFML/Graphics.hpp>
#include <vector>

class ChunkDrawable;

using ChunkCoords = uint64_t;

class ChunkRender {
public:
    static void setupTextureData(unsigned int tileWidth);

    ChunkRender();
    void resize(int currentLod, const sf::Vector2u& chunkArea);    // FIXME have currentLod be a member?
    void allocateBlock(int currentLod, std::vector<ChunkDrawable>& chunkDrawables, size_t index);

private:
    static unsigned int tileWidth_;

    struct RenderBlock {
        static constexpr ChunkCoords emptyChunkCoords = 0x7fffffff7fffffff;

        ChunkCoords coords = 0;
        int textureIndex = -1;
        float adjustedChebyshev = 0.0f;
    };
    friend bool operator<(const ChunkRender::RenderBlock& lhs, const ChunkRender::RenderBlock& rhs);

    sf::RenderTexture texture_;
    sf::VertexBuffer buffer_;
    std::vector<RenderBlock> blocks_;
};
