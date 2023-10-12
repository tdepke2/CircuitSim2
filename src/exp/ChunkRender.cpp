#include <Chunk.h>
#include <ChunkDrawable.h>
#include <ChunkRender.h>

#include <numeric>
#include <spdlog/spdlog.h>

inline int unpackChunkCoordsX(ChunkCoords coords) {
    return static_cast<int32_t>(coords);
}

inline int unpackChunkCoordsY(ChunkCoords coords) {
    return static_cast<int32_t>(coords >> 32);
}

unsigned int ChunkRender::tileWidth_;

void ChunkRender::setupTextureData(unsigned int tileWidth) {
    tileWidth_ = tileWidth;
}

ChunkRender::ChunkRender() :
    chunkArea_(0, 0),
    texture_(),
    buffer_(sf::Triangles),
    renderIndexPool_(),
    renderBlocks_() {
}

void ChunkRender::resize(int currentLod, const sf::Vector2u& chunkArea) {
    if (chunkArea_ != chunkArea) {
        chunkArea_ = chunkArea;
        spdlog::debug("Resizing LOD {} area to {} by {} chunks.", currentLod, chunkArea.x, chunkArea.y);
        const sf::Vector2u textureSize = chunkArea * (static_cast<unsigned int>(Chunk::WIDTH) * tileWidth_) / (1u << currentLod);
        if (!texture_.create(textureSize.x, textureSize.y)) {
            spdlog::error("Failed to create texture for LOD {} (size {} by {}).", currentLod, textureSize.x, textureSize.y);
        }
        const unsigned int bufferSize = chunkArea.x * chunkArea.y * 6;
        if (!buffer_.create(bufferSize)) {
            spdlog::error("Failed to create vertex buffer for LOD {} (size {}).", currentLod, bufferSize);
        }
        renderIndexPool_.resize(chunkArea.x * chunkArea.y);
        std::iota(renderIndexPool_.begin(), renderIndexPool_.end(), 0);
        renderBlocks_.clear();
        renderBlocks_.reserve(renderIndexPool_.size());

        // FIXME doesn't clearing the blocks imply that we need to reset the references in chunkDrawables?
        // ##############################################################################################
    }
}

void ChunkRender::allocateBlock(int currentLod, FlatMap<ChunkCoords, ChunkDrawable>& chunkDrawables, ChunkCoords coords, const sf::IntRect& visibleArea) {
    if (renderBlocks_.size() < renderIndexPool_.size()) {
        unsigned int poolIndex = renderBlocks_.size();
        renderBlocks_.emplace_back(coords, poolIndex);
        chunkDrawables.at(coords).setRenderIndex(currentLod, renderIndexPool_[poolIndex]);
        return;
    }

    for (auto renderBlock = renderBlocks_.rbegin(); renderBlock != renderBlocks_.rend(); ++renderBlock) {
        if (!visibleArea.contains(unpackChunkCoordsX(renderBlock->coords), unpackChunkCoordsY(renderBlock->coords))) {

            auto chunkDrawableIter = chunkDrawables.find(renderBlock->coords);
            chunkDrawableIter->second.setRenderIndex(currentLod, -1);
            if (chunkDrawableIter->second.getChunk() == nullptr && !chunkDrawableIter->second.hasAnyRenderIndex()) {
                chunkDrawables.erase(chunkDrawableIter);
            }

            renderBlock->coords = coords;
            chunkDrawables.at(coords).setRenderIndex(currentLod, renderIndexPool_[renderBlock->poolIndex]);
            return;

            // FIXME it seems silly to store renderIndexPool_ if we never deallocate the blocks.
        }
    }
    // We should never run into a case where the visible chunk area exceeds the number of blocks we have.
    assert(false);
}

void ChunkRender::drawChunk(int currentLod, const ChunkDrawable& chunkDrawable, sf::RenderStates states) {
    int textureSubdivisionSize = Chunk::WIDTH * static_cast<int>(tileWidth_) / (1 << currentLod);
    states.transform.translate(
        static_cast<float>(static_cast<int>(chunkDrawable.getRenderIndex(currentLod) % static_cast<int>(chunkArea_.x)) * textureSubdivisionSize),
        static_cast<float>(static_cast<int>(chunkDrawable.getRenderIndex(currentLod) / static_cast<int>(chunkArea_.x)) * textureSubdivisionSize)
    );
    texture_.draw(chunkDrawable, states);
}

bool operator<(const ChunkRender::RenderBlock& lhs, const ChunkRender::RenderBlock& rhs) {
    return lhs.adjustedChebyshev < rhs.adjustedChebyshev;
}

void ChunkRender::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    sf::Sprite sprite(texture_.getTexture());
    target.draw(sprite, states);
    return;

    // FIXME 

    states.texture = &texture_.getTexture();
    target.draw(buffer_, states);
}
