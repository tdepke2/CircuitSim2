#include <Chunk.h>
#include <ChunkDrawable.h>
#include <ChunkRender.h>
#include <DebugScreen.h>

#include <algorithm>
#include <limits>
#include <numeric>
#include <spdlog/spdlog.h>
#include <string>

unsigned int ChunkRender::tileWidth_;

void ChunkRender::setupTextureData(unsigned int tileWidth) {
    tileWidth_ = tileWidth;
}

ChunkRender::ChunkRender() :
    levelOfDetail_(0),
    maxChunkArea_(0, 0),
    lastVisibleArea_(0, 0, 0, 0),
    texture_(),
    textureDirty_(false),
    buffer_(sf::Triangles),
    bufferVertices_(),
    renderIndexPool_(),
    renderBlocks_() {
}

void ChunkRender::setLod(int levelOfDetail) {
    levelOfDetail_ = levelOfDetail;
}

int ChunkRender::getLod() const {
    return levelOfDetail_;
}

void ChunkRender::resize(FlatMap<ChunkCoords::repr, ChunkDrawable>& chunkDrawables, const sf::Vector2u& maxChunkArea) {
    const int chunkWidthTexels = Chunk::WIDTH * static_cast<int>(tileWidth_);
    const int textureSubdivisionSize = chunkWidthTexels / (1 << levelOfDetail_);

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

    spdlog::debug("Resizing LOD {} area to {} by {} chunks.", levelOfDetail_, adjustedMaxChunkArea.x, adjustedMaxChunkArea.y);
    maxChunkArea_ = adjustedMaxChunkArea;
    lastVisibleArea_ = {0, 0, 0, 0};

    if (!texture_.create(textureSize.x, textureSize.y)) {
        spdlog::error("Failed to create texture for LOD {} (size {} by {}).", levelOfDetail_, textureSize.x, textureSize.y);
    }
    texture_.clear(sf::Color::Black);
    texture_.setSmooth(true);
    textureDirty_ = true;
    DebugScreen::instance()->registerTexture("chunkRender LOD " + std::to_string(levelOfDetail_), &texture_.getTexture());

    const unsigned int bufferSize = maxChunkArea_.x * maxChunkArea_.y * 6;
    bufferVertices_.resize(bufferSize);
    if (!buffer_.create(bufferSize)) {
        spdlog::error("Failed to create vertex buffer for LOD {} (size {}).", levelOfDetail_, bufferSize);
    }
    for (unsigned int y = 0; y < maxChunkArea_.y; ++y) {
        for (unsigned int x = 0; x < maxChunkArea_.x; ++x) {
            sf::Vertex* tileVertices = &bufferVertices_[(y * maxChunkArea_.x + x) * 6];

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

    for (auto& renderBlock : renderBlocks_) {
        chunkDrawables.at(renderBlock.coords).setRenderIndex(levelOfDetail_, -1);
    }
    renderIndexPool_.resize(maxChunkArea_.x * maxChunkArea_.y);
    std::iota(renderIndexPool_.begin(), renderIndexPool_.end(), 0);
    renderBlocks_.clear();
    renderBlocks_.reserve(renderIndexPool_.size());
}

void ChunkRender::allocateBlock(FlatMap<ChunkCoords::repr, ChunkDrawable>& chunkDrawables, ChunkCoords::repr coords, const sf::IntRect& visibleArea) {
    if (renderBlocks_.size() < renderIndexPool_.size()) {
        unsigned int poolIndex = renderBlocks_.size();
        renderBlocks_.emplace_back(coords, poolIndex);
        chunkDrawables.at(coords).setRenderIndex(levelOfDetail_, renderIndexPool_[poolIndex]);
        spdlog::debug("Allocated new block (LOD {} at chunk {}, {}) with render index {}.",
            levelOfDetail_, ChunkCoords::x(coords), ChunkCoords::y(coords), renderIndexPool_[poolIndex]
        );
        return;
    }

    for (auto renderBlock = renderBlocks_.rbegin(); renderBlock != renderBlocks_.rend(); ++renderBlock) {
        int x = ChunkCoords::x(renderBlock->coords);
        int y = ChunkCoords::y(renderBlock->coords);
        if (x < visibleArea.left || x >= visibleArea.left + visibleArea.width || y < visibleArea.top || y >= visibleArea.top + visibleArea.height) {

            spdlog::debug("Swapping block (LOD {} at chunk {}, {}) with render index {} for chunk at {}, {}.",
                levelOfDetail_, ChunkCoords::x(renderBlock->coords), ChunkCoords::y(renderBlock->coords), renderIndexPool_[renderBlock->poolIndex],
                ChunkCoords::x(coords), ChunkCoords::y(coords)
            );
            chunkDrawables.at(renderBlock->coords).setRenderIndex(levelOfDetail_, -1);
            renderBlock->coords = coords;
            chunkDrawables.at(coords).setRenderIndex(levelOfDetail_, renderIndexPool_[renderBlock->poolIndex]);
            return;

            // FIXME it seems silly to store renderIndexPool_ if we never deallocate the blocks.
        }
    }
    // We should never run into a case where the visible chunk area exceeds the number of blocks we have.
    assert(false);
}

void ChunkRender::drawChunk(const ChunkDrawable& chunkDrawable, sf::RenderStates states) {
    const int textureSubdivisionSize = Chunk::WIDTH * static_cast<int>(tileWidth_) / (1 << levelOfDetail_);
    states.transform.translate(
        getChunkTexCoords(chunkDrawable.getRenderIndex(levelOfDetail_), textureSubdivisionSize)
    );
    states.transform.scale(
        1.0f / (1 << levelOfDetail_),
        1.0f / (1 << levelOfDetail_)
    );
    spdlog::debug("Redrawing LOD {} render index {}.", levelOfDetail_, chunkDrawable.getRenderIndex(levelOfDetail_));
    texture_.draw(chunkDrawable, states);
    textureDirty_ = true;
    chunkDrawable.renderDirty_.reset(levelOfDetail_);
}

void ChunkRender::display() {
    if (textureDirty_) {
        texture_.display();
        textureDirty_ = false;
    }
}

void ChunkRender::updateVisibleArea(const FlatMap<ChunkCoords::repr, ChunkDrawable>& chunkDrawables, const sf::IntRect& visibleArea) {
    if (lastVisibleArea_ == visibleArea) {
        return;
    }

    lastVisibleArea_ = visibleArea;
    spdlog::debug("Chunk area changed, updating buffer.");
    const int textureSubdivisionSize = Chunk::WIDTH * static_cast<int>(tileWidth_) / (1 << levelOfDetail_);
    const auto& emptyChunk = chunkDrawables.at(EMPTY_CHUNK_COORDS);
    for (int y = 0; y < visibleArea.height; ++y) {
        auto chunkDrawable = chunkDrawables.upper_bound(ChunkCoords::pack(visibleArea.left - 1, visibleArea.top + y));
        for (int x = 0; x < visibleArea.width; ++x) {
            sf::Vertex* tileVertices = &bufferVertices_[(y * maxChunkArea_.x + x) * 6];
            int renderIndex;
            if (chunkDrawable == chunkDrawables.end() || chunkDrawable->first != ChunkCoords::pack(visibleArea.left + x, visibleArea.top + y)) {
                renderIndex = emptyChunk.getRenderIndex(levelOfDetail_);
            } else {
                renderIndex = chunkDrawable->second.getRenderIndex(levelOfDetail_);
                ++chunkDrawable;
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
    buffer_.update(bufferVertices_.data());

    sortRenderBlocks();
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

void ChunkRender::sortRenderBlocks() {
    sf::Vector2f centerPosition = {
        lastVisibleArea_.left + (lastVisibleArea_.width - 1) / 2.0f,
        lastVisibleArea_.top + (lastVisibleArea_.height - 1) / 2.0f
    };
    for (auto& renderBlock : renderBlocks_) {
        if (renderBlock.coords != EMPTY_CHUNK_COORDS) {
            renderBlock.adjustedChebyshev = std::max(
                std::abs(ChunkCoords::x(renderBlock.coords) - centerPosition.x) / lastVisibleArea_.width,
                std::abs(ChunkCoords::y(renderBlock.coords) - centerPosition.y) / lastVisibleArea_.height
            );
        } else {
            renderBlock.adjustedChebyshev = 0.0f;
        }
    }
    std::sort(renderBlocks_.begin(), renderBlocks_.end());
    /*spdlog::debug("Sorted chunk render blocks ({}):", renderBlocks_.size());
    for (auto& renderBlock : renderBlocks_) {
        spdlog::debug("  pos ({}, {}), renderIndex {}, adjChebyshev {}",
            ChunkCoords::x(renderBlock.coords), ChunkCoords::y(renderBlock.coords), renderIndexPool_[renderBlock.poolIndex], renderBlock.adjustedChebyshev
        );
    }*/
}

void ChunkRender::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    states.texture = &texture_.getTexture();
    target.draw(buffer_, states);
}