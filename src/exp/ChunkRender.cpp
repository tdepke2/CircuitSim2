#include <Chunk.h>
#include <ChunkRender.h>

#include <spdlog/spdlog.h>

void ChunkRender::setupTextureData(unsigned int tileWidth) {
    tileWidth_ = tileWidth;
}

ChunkRender::ChunkRender() :
    texture_(),
    buffer_(sf::Triangles),
    blocks_() {
}

void ChunkRender::resize(int currentLod, const sf::Vector2u& chunkArea) {
    const sf::Vector2u textureSize = chunkArea * (static_cast<unsigned int>(Chunk::WIDTH) * tileWidth_) / (1u << currentLod);

    if (texture_.getSize() != textureSize) {
        spdlog::debug("Resizing LOD {} area to {} by {} chunks.", currentLod, chunkArea.x, chunkArea.y);
        if (!texture_.create(textureSize.x, textureSize.y)) {
            spdlog::error("Failed to create texture for LOD {} (size {} by {}).", currentLod, textureSize.x, textureSize.y);
        }
        const unsigned int bufferSize = textureSize.x * textureSize.y * 6;
        if (!buffer_.create(bufferSize)) {
            spdlog::error("Failed to create vertex buffer for LOD {} (size {}).", currentLod, bufferSize);
        }
        blocks_.clear();
        blocks_.resize(chunkArea.x * chunkArea.y);
    }
}

void ChunkRender::allocateBlock(int currentLod, std::vector<ChunkDrawable>& chunkDrawables, size_t index) {
    
}

bool operator<(const ChunkRender::RenderBlock& lhs, const ChunkRender::RenderBlock& rhs) {
    return lhs.adjustedChebyshev < rhs.adjustedChebyshev;
}
