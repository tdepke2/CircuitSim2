#include <Chunk.h>
#include <ChunkDrawable.h>
#include <ChunkRender.h>
#include <DebugScreen.h>

#include <numeric>
#include <spdlog/spdlog.h>
#include <string>

inline ChunkCoords packChunkCoords(int x, int y) {
    return static_cast<uint64_t>(y) << 32 | static_cast<uint32_t>(x);
}

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
    maxChunkArea_(0, 0),
    lastVisibleArea_(0, 0, 0, 0),
    texture_(),
    textureDirty_(false),
    buffer_(sf::Triangles),
    renderIndexPool_(),
    renderBlocks_() {
}

void ChunkRender::resize(int currentLod, const sf::Vector2u& maxChunkArea) {
    const int chunkWidthTexels = Chunk::WIDTH * static_cast<int>(tileWidth_);
    const int textureSubdivisionSize = chunkWidthTexels / (1 << currentLod);

    // Apply padding so that chunks in the texture have some border space to avoid texture bleed.
    const sf::Vector2f paddedChunkArea = {
        std::ceil(maxChunkArea.x * static_cast<float>(textureSubdivisionSize + CHUNK_PADDING) / textureSubdivisionSize),
        std::ceil(maxChunkArea.y * static_cast<float>(textureSubdivisionSize + CHUNK_PADDING) / textureSubdivisionSize)
    };

    // Round up to power of 2 for padded area to ensure POT texture.
    const sf::Vector2u pow2PaddedChunkArea = {
        1u << static_cast<unsigned int>(std::ceil(std::log2(paddedChunkArea.x))),
        1u << static_cast<unsigned int>(std::ceil(std::log2(paddedChunkArea.y)))
    };
    const sf::Vector2u textureSize = pow2PaddedChunkArea * static_cast<unsigned int>(textureSubdivisionSize);

    // Strip the padding off to find the usable chunk area in the texture, mathematically this should not be less than the original maxChunkArea.
    const sf::Vector2u adjustedMaxChunkArea = {
        static_cast<unsigned int>(std::floor(pow2PaddedChunkArea.x / static_cast<float>(textureSubdivisionSize + CHUNK_PADDING) * textureSubdivisionSize)),
        static_cast<unsigned int>(std::floor(pow2PaddedChunkArea.y / static_cast<float>(textureSubdivisionSize + CHUNK_PADDING) * textureSubdivisionSize))
    };

    DebugScreen::instance()->getField("lodRange").setString(fmt::format("Range: {}, {} (adjusted {}, {})", maxChunkArea.x, maxChunkArea.y, adjustedMaxChunkArea.x, adjustedMaxChunkArea.y));
    if (texture_.getSize() == textureSize) {
        return;
    }

    spdlog::debug("Resizing LOD {} area to {} by {} chunks.", currentLod, adjustedMaxChunkArea.x, adjustedMaxChunkArea.y);
    spdlog::debug("================= maxChunkArea = {}, {} paddedChunkArea = {}, {}", maxChunkArea.x, maxChunkArea.y, paddedChunkArea.x, paddedChunkArea.y);
    spdlog::debug("================= textureSubdivisionSize = {}, textureSize = {}, {}", textureSubdivisionSize, textureSize.x, textureSize.y);
    maxChunkArea_ = adjustedMaxChunkArea;
    if (!texture_.create(textureSize.x, textureSize.y)) {
        spdlog::error("Failed to create texture for LOD {} (size {} by {}).", currentLod, textureSize.x, textureSize.y);
    }
    texture_.clear(sf::Color::Black);
    texture_.setSmooth(true);
    textureDirty_ = true;
    DebugScreen::instance()->registerTexture("chunkRender LOD " + std::to_string(currentLod), &texture_.getTexture());

    const unsigned int bufferSize = maxChunkArea_.x * maxChunkArea_.y * 6;
    buffer_.resize(bufferSize);
    //if (!buffer_.create(bufferSize)) {
    //    spdlog::error("Failed to create vertex buffer for LOD {} (size {}).", currentLod, bufferSize);
    //}
    for (unsigned int y = 0; y < maxChunkArea_.y; ++y) {
        for (unsigned int x = 0; x < maxChunkArea_.x; ++x) {
            sf::Vertex* tileVertices = &buffer_[(y * maxChunkArea_.x + x) * 6];

            float px = static_cast<float>(x * chunkWidthTexels);
            float py = static_cast<float>(y * chunkWidthTexels);
            tileVertices[0].position = {px, py};
            tileVertices[1].position = {px + chunkWidthTexels, py};
            tileVertices[2].position = {px + chunkWidthTexels, py + chunkWidthTexels};
            tileVertices[3].position = {px + chunkWidthTexels, py + chunkWidthTexels};
            tileVertices[4].position = {px, py + chunkWidthTexels};
            tileVertices[5].position = {px, py};
        }
    }

    renderIndexPool_.resize(maxChunkArea_.x * maxChunkArea_.y);
    std::iota(renderIndexPool_.begin(), renderIndexPool_.end(), 0);
    renderBlocks_.clear();
    renderBlocks_.reserve(renderIndexPool_.size());

    // FIXME doesn't clearing the blocks imply that we need to reset the references in chunkDrawables?
    // ##############################################################################################
}

void ChunkRender::allocateBlock(int currentLod, FlatMap<ChunkCoords, ChunkDrawable>& chunkDrawables, ChunkCoords coords, const sf::IntRect& visibleArea) {
    if (renderBlocks_.size() < renderIndexPool_.size()) {
        unsigned int poolIndex = renderBlocks_.size();
        renderBlocks_.emplace_back(coords, poolIndex);
        chunkDrawables.at(coords).setRenderIndex(currentLod, renderIndexPool_[poolIndex]);
        spdlog::debug("Allocated new block (LOD {} at chunk {}, {}) with render index {}.",
            currentLod, unpackChunkCoordsX(coords), unpackChunkCoordsY(coords), renderIndexPool_[poolIndex]
        );
        return;
    }

    for (auto renderBlock = renderBlocks_.rbegin(); renderBlock != renderBlocks_.rend(); ++renderBlock) {
        int x = unpackChunkCoordsX(renderBlock->coords);
        int y = unpackChunkCoordsY(renderBlock->coords);
        if (x < visibleArea.left || x >= visibleArea.left + visibleArea.width || y < visibleArea.top || y >= visibleArea.top + visibleArea.height) {

            spdlog::debug("Swapping block (LOD {} at chunk {}, {}) with render index {} for chunk at {}, {}.",
                currentLod, unpackChunkCoordsX(renderBlock->coords), unpackChunkCoordsY(renderBlock->coords), renderIndexPool_[renderBlock->poolIndex],
                unpackChunkCoordsX(coords), unpackChunkCoordsY(coords)
            );
            chunkDrawables.at(renderBlock->coords).setRenderIndex(currentLod, -1);
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
    const int textureSubdivisionSize = Chunk::WIDTH * static_cast<int>(tileWidth_) / (1 << currentLod);
    states.transform.translate(
        getChunkTexCoords(chunkDrawable.getRenderIndex(currentLod), textureSubdivisionSize)
    );
    states.transform.scale(
        1.0f / (1 << currentLod),
        1.0f / (1 << currentLod)
    );
    spdlog::debug("Redrawing LOD {} render index {}.", currentLod, chunkDrawable.getRenderIndex(currentLod));
    texture_.draw(chunkDrawable, states);
    textureDirty_ = true;
    chunkDrawable.renderDirty_.reset(currentLod);
}

void ChunkRender::display() {
    if (textureDirty_) {
        texture_.display();
        textureDirty_ = false;
    }
}

void ChunkRender::updateVisibleArea(int currentLod, const FlatMap<ChunkCoords, ChunkDrawable>& chunkDrawables, const sf::IntRect& visibleArea) {
    if (lastVisibleArea_ == visibleArea) {
        return;
    }

    lastVisibleArea_ = visibleArea;
    spdlog::debug("Chunk area changed, updating buffer.");
    const int textureSubdivisionSize = Chunk::WIDTH * static_cast<int>(tileWidth_) / (1 << currentLod);
    for (int y = 0; y < visibleArea.height; ++y) {
        for (int x = 0; x < visibleArea.width; ++x) {
            sf::Vertex* tileVertices = &buffer_[(y * maxChunkArea_.x + x) * 6];
            int renderIndex;
            auto chunkDrawable = chunkDrawables.find(packChunkCoords(visibleArea.left + x, visibleArea.top + y));

            // FIXME we could use the existing loop in Board() to do this, bypassing the lookup ###########################
            // along with this, we could update the area before(?) allocateBlock() calls and then remove the visibleArea param from that function.

            if (chunkDrawable == chunkDrawables.end()) {
                renderIndex = chunkDrawables.at(EMPTY_CHUNK_COORDS).getRenderIndex(currentLod);
            } else {
                renderIndex = chunkDrawable->second.getRenderIndex(currentLod);
            }
            spdlog::trace("Buffer ({}, {}) set to render index {}.", x, y, renderIndex);

            // Apply a small bias to the texture coords to hide the chunk seams.
            constexpr float texBias = 0.5f;
            sf::Vector2f t = getChunkTexCoords(renderIndex, textureSubdivisionSize) + sf::Vector2f(texBias, texBias);
            tileVertices[0].texCoords = {t.x, t.y};
            tileVertices[1].texCoords = {t.x + textureSubdivisionSize - texBias, t.y};
            tileVertices[2].texCoords = {t.x + textureSubdivisionSize - texBias, t.y + textureSubdivisionSize - texBias};
            tileVertices[3].texCoords = {t.x + textureSubdivisionSize - texBias, t.y + textureSubdivisionSize - texBias};
            tileVertices[4].texCoords = {t.x, t.y + textureSubdivisionSize - texBias};
            tileVertices[5].texCoords = {t.x, t.y};
        }
    }
}

bool operator<(const ChunkRender::RenderBlock& lhs, const ChunkRender::RenderBlock& rhs) {
    return lhs.adjustedChebyshev < rhs.adjustedChebyshev;
}

sf::Vector2f ChunkRender::getChunkTexCoords(int renderIndex, int textureSubdivisionSize) const {
    return {
        static_cast<float>(static_cast<int>(renderIndex % static_cast<int>(maxChunkArea_.x)) * (textureSubdivisionSize + CHUNK_PADDING)),
        static_cast<float>(static_cast<int>(renderIndex / static_cast<int>(maxChunkArea_.x)) * (textureSubdivisionSize + CHUNK_PADDING))
    };
}

void ChunkRender::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    states.texture = &texture_.getTexture();
    target.draw(buffer_, states);
    sf::CircleShape c(16.0f);
    c.setFillColor(sf::Color::Magenta);
    c.setPosition(Chunk::WIDTH * tileWidth_, Chunk::WIDTH * tileWidth_);
    c.setOrigin(c.getRadius(), c.getRadius());
    target.draw(c, states);
}
