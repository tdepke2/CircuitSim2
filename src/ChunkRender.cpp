#include <Chunk.h>
#include <ChunkDrawable.h>
#include <ChunkRender.h>
#include <DebugScreen.h>
#include <Locator.h>
#include <LodRenderer.h>
#include <ResourceBase.h>
#include <TileWidth.h>

#include <algorithm>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <string>

// Disable a false-positive warning issue with gcc:
#if defined(__GNUC__) && !defined(__clang__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wdangling-reference"
#endif
    #include <spdlog/fmt/ranges.h>
    #include <spdlog/spdlog.h>
#if defined(__GNUC__) && !defined(__clang__)
    #pragma GCC diagnostic pop
#endif

ChunkRender::StaticInit* ChunkRender::staticInit_ = nullptr;

ChunkRender::StaticInit::StaticInit() {
    spdlog::debug("ChunkRender::StaticInit initializing.");
    ResourceBase* resource = Locator::getResource();

    try {
        chunkShader = &resource->getShader("resources/shaders/chunk_vert.glsl", "resources/shaders/chunk_geom.glsl", "resources/shaders/chunk_frag.glsl");
        chunkShader->setUniform("texture", sf::Shader::CurrentTexture);
    } catch (const std::exception& ex) {
        spdlog::error(ex.what());
        spdlog::warn("Switching to old chunk rendering method.");
        chunkShader = nullptr;
    }
}

ChunkRender::ChunkRender() :
    levelOfDetail_(0),
    maxChunkArea_(0, 0),
    lastVisibleArea_(0, 0, 0, 0),
    lastTopLeft_(0),
    lastViewProjection_(),
    texture_(),
    textureDirty_(false),
    buffer_(),
    bufferVertices_(),
    bufferDirty_(false),
    renderIndexPool_(),
    renderBlocks_() {

    static StaticInit staticInit;
    staticInit_ = &staticInit;

    if (staticInit_->chunkShader != nullptr) {
        buffer_.setPrimitiveType(sf::Points);
    } else {
        buffer_.setPrimitiveType(sf::Triangles);
    }
}

void ChunkRender::setLod(int levelOfDetail) {
    levelOfDetail_ = levelOfDetail;
}

int ChunkRender::getLod() const {
    return levelOfDetail_;
}

void ChunkRender::resize(FlatMap<ChunkCoords::repr, ChunkDrawable>& chunkDrawables, const sf::Vector2u& maxChunkArea) {
    constexpr int chunkWidthTexels = Chunk::WIDTH * static_cast<int>(TileWidth::TEXELS);
    const int textureSubdivisionSize = chunkWidthTexels / (1 << levelOfDetail_);

    // Round up to power of 2 to ensure POT texture.
    const sf::Vector2u pow2ChunkArea = {
        1u << static_cast<unsigned int>(std::ceil(std::log2(maxChunkArea.x))),
        1u << static_cast<unsigned int>(std::ceil(std::log2(maxChunkArea.y)))
    };
    const sf::Vector2u textureSize = pow2ChunkArea * static_cast<unsigned int>(textureSubdivisionSize);

    DebugScreen::instance()->getField("lodRange").setString(fmt::format("Range: {}, {} (pow2 {}, {})", maxChunkArea.x, maxChunkArea.y, pow2ChunkArea.x, pow2ChunkArea.y));
    if (texture_.getSize() == textureSize) {
        return;
    }

    spdlog::debug("Resizing LOD {} area to {} by {} chunks.", levelOfDetail_, pow2ChunkArea.x, pow2ChunkArea.y);
    maxChunkArea_ = pow2ChunkArea;
    lastVisibleArea_ = {0, 0, 0, 0};
    lastTopLeft_ = 0;

    if (!texture_.create(textureSize.x, textureSize.y)) {
        spdlog::error("Failed to create texture for LOD {} (size {} by {}).", levelOfDetail_, textureSize.x, textureSize.y);
    }
    texture_.clear(sf::Color::Black);
    texture_.setSmooth(true);
    textureDirty_ = true;
    DebugScreen::instance()->registerTexture("chunkRender LOD " + std::to_string(levelOfDetail_), &texture_.getTexture());

    const unsigned int bufferSize = maxChunkArea_.x * maxChunkArea_.y * (staticInit_->chunkShader != nullptr ? 1 : 6);
    if (!buffer_.create(bufferSize)) {
        spdlog::error("Failed to create vertex buffer for LOD {} (size {}).", levelOfDetail_, bufferSize);
    }

    for (auto& renderBlock : renderBlocks_) {
        chunkDrawables.at(renderBlock.coords).setRenderIndex(levelOfDetail_, -1);
    }
    renderIndexPool_.resize(maxChunkArea_.x * maxChunkArea_.y);
    std::iota(renderIndexPool_.begin(), renderIndexPool_.end(), 0);
    renderBlocks_.clear();
    renderBlocks_.reserve(renderIndexPool_.size());
}

void ChunkRender::allocateBlock(FlatMap<ChunkCoords::repr, ChunkDrawable>& chunkDrawables, ChunkCoords::repr coords, const ChunkCoordsRange& visibleArea) {
    if (visibleArea.contains(coords)) {
        bufferDirty_ = true;
    }

    if (renderBlocks_.size() < renderIndexPool_.size()) {
        unsigned int poolIndex = renderBlocks_.size();
        renderBlocks_.emplace_back(coords, poolIndex);
        chunkDrawables.at(coords).setRenderIndex(levelOfDetail_, renderIndexPool_[poolIndex]);
        spdlog::debug("Allocated new block (LOD {} at chunk {}) with render index {}.",
            levelOfDetail_, ChunkCoords::toPair(coords), renderIndexPool_[poolIndex]
        );
        return;
    }

    for (auto renderBlock = renderBlocks_.rbegin(); renderBlock != renderBlocks_.rend(); ++renderBlock) {
        if (!visibleArea.contains(renderBlock->coords)) {

            spdlog::debug("Swapping block (LOD {} at chunk {}) with render index {} for chunk at {}.",
                levelOfDetail_, ChunkCoords::toPair(renderBlock->coords), renderIndexPool_[renderBlock->poolIndex], ChunkCoords::toPair(coords)
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
    // FIXME: this probably shouldn't be an assert, log an error and continue.
}

void ChunkRender::drawChunk(const ChunkDrawable& chunkDrawable, sf::RenderStates states) {
    const int textureSubdivisionSize = Chunk::WIDTH * static_cast<int>(TileWidth::TEXELS) / (1 << levelOfDetail_);
    states.transform.translate(
        getChunkTexCoords(chunkDrawable.getRenderIndex(levelOfDetail_), textureSubdivisionSize)
    );
    states.transform.scale(
        1.0f / (1 << levelOfDetail_),
        1.0f / (1 << levelOfDetail_)
    );
    //spdlog::debug("Redrawing LOD {} render index {}.", levelOfDetail_, chunkDrawable.getRenderIndex(levelOfDetail_));
    texture_.draw(chunkDrawable, states);
    textureDirty_ = true;
    chunkDrawable.markAsDrawn(levelOfDetail_);
}

void ChunkRender::display() {
    if (textureDirty_) {
        texture_.display();
        textureDirty_ = false;
    }
}

void ChunkRender::updateVisibleArea(const FlatMap<ChunkCoords::repr, ChunkDrawable>& chunkDrawables, const ChunkCoordsRange& visibleArea, ChunkCoords::repr topLeft, const sf::Transform& viewProjection) {
    lastViewProjection_ = viewProjection;
    if (lastVisibleArea_ == visibleArea && lastTopLeft_ == topLeft && !bufferDirty_) {
        return;
    }

    lastVisibleArea_ = visibleArea;
    lastTopLeft_ = topLeft;
    bufferDirty_ = false;
    spdlog::debug("Chunk area changed or buffer dirty, updating buffer.");
    constexpr int chunkWidthTexels = Chunk::WIDTH * static_cast<int>(TileWidth::TEXELS);
    const int textureSubdivisionSize = chunkWidthTexels / (1 << levelOfDetail_);
    const auto& emptyChunk = chunkDrawables.at(LodRenderer::EMPTY_CHUNK_COORDS);
    const sf::Vector2i positionOffset = {
        visibleArea.left - ChunkCoords::x(topLeft),
        visibleArea.top - ChunkCoords::y(topLeft)
    };
    spdlog::debug("positionOffset = {}, {}", positionOffset.x, positionOffset.y);

    bufferVertices_.clear();
    for (int y = 0; y < visibleArea.height; ++y) {
        auto chunkDrawable = chunkDrawables.upper_bound(ChunkCoords::pack(visibleArea.left - 1, visibleArea.top + y));
        for (int x = 0; x < visibleArea.width; ++x) {
            int renderIndex;
            if (chunkDrawable == chunkDrawables.end() || chunkDrawable->first != ChunkCoords::pack(visibleArea.left + x, visibleArea.top + y)) {
                renderIndex = emptyChunk.getRenderIndex(levelOfDetail_);
            } else {
                renderIndex = chunkDrawable->second.getRenderIndex(levelOfDetail_);
                ++chunkDrawable;
            }
            spdlog::trace("Buffer ({}, {}) set to render index {}.", x, y, renderIndex);
            const sf::Vector2f p = static_cast<sf::Vector2f>((sf::Vector2i(x, y) + positionOffset) * chunkWidthTexels);

            if (staticInit_->chunkShader != nullptr) {
                const sf::Vector2f t = getChunkTexCoords(renderIndex, textureSubdivisionSize);
                bufferVertices_.emplace_back(p, t);
            } else {
                // Apply a small bias to the texture coords to hide the chunk seams.
                const float texBias = 0.5f * (levelOfDetail_ + 1);
                const sf::Vector2f t = getChunkTexCoords(renderIndex, textureSubdivisionSize) + sf::Vector2f(texBias / 2.0f, texBias / 2.0f);
                const float texOffset = textureSubdivisionSize - texBias;

                bufferVertices_.emplace_back(p, t);
                bufferVertices_.emplace_back(sf::Vector2f(p.x + chunkWidthTexels, p.y), sf::Vector2f(t.x + texOffset, t.y));
                bufferVertices_.emplace_back(sf::Vector2f(p.x + chunkWidthTexels, p.y + chunkWidthTexels), sf::Vector2f(t.x + texOffset, t.y + texOffset));
                bufferVertices_.emplace_back(sf::Vector2f(p.x + chunkWidthTexels, p.y + chunkWidthTexels), sf::Vector2f(t.x + texOffset, t.y + texOffset));
                bufferVertices_.emplace_back(sf::Vector2f(p.x, p.y + chunkWidthTexels), sf::Vector2f(t.x, t.y + texOffset));
                bufferVertices_.emplace_back(p, t);
            }
        }
    }
    spdlog::debug("Buffer now has {} vertices.", bufferVertices_.size());
    if (bufferVertices_.size() > 0) {
        buffer_.update(bufferVertices_.data(), bufferVertices_.size(), 0);
        sortRenderBlocks();
    }
}

bool operator<(const ChunkRender::RenderBlock& lhs, const ChunkRender::RenderBlock& rhs) {
    return lhs.adjustedChebyshev < rhs.adjustedChebyshev;
}

sf::Vector2f ChunkRender::getChunkTexCoords(int renderIndex, int textureSubdivisionSize) const {
    return {
        static_cast<float>(static_cast<int>(renderIndex % static_cast<int>(maxChunkArea_.x)) * textureSubdivisionSize),
        static_cast<float>(static_cast<int>(renderIndex / static_cast<int>(maxChunkArea_.x)) * textureSubdivisionSize)
    };
}

void ChunkRender::sortRenderBlocks() {
    sf::Vector2f centerPosition = {
        lastVisibleArea_.left + (lastVisibleArea_.width - 1) / 2.0f,
        lastVisibleArea_.top + (lastVisibleArea_.height - 1) / 2.0f
    };
    for (auto& renderBlock : renderBlocks_) {
        if (renderBlock.coords != LodRenderer::EMPTY_CHUNK_COORDS) {
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
        spdlog::debug("  pos {}, renderIndex {}, adjChebyshev {}",
            ChunkCoords::toPair(renderBlock.coords), renderIndexPool_[renderBlock.poolIndex], renderBlock.adjustedChebyshev
        );
    }*/
}

void ChunkRender::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    states.texture = &texture_.getTexture();
    if (staticInit_->chunkShader != nullptr) {
        states.shader = staticInit_->chunkShader;
        constexpr int chunkWidthTexels = Chunk::WIDTH * static_cast<int>(TileWidth::TEXELS);
        const int textureSubdivisionSize = chunkWidthTexels / (1 << levelOfDetail_);

        sf::Transform mvp = lastViewProjection_ * states.transform;
        const sf::Vector2f positionOffset = {
            mvp.getMatrix()[0] * chunkWidthTexels,
            mvp.getMatrix()[5] * chunkWidthTexels
        };
        /*spdlog::debug("mat mvp =");
        for (int i = 0; i < 4; i += 1) {
            auto a = mvp.getMatrix();
            spdlog::debug("{}  {}  {}  {}", a[i + 0], a[i + 4], a[i + 8], a[i + 12]);
        }
        spdlog::debug("positionOffset= {}, {}", positionOffset.x, positionOffset.y);*/
        staticInit_->chunkShader->setUniform("positionOffset", positionOffset);

        const sf::Vector2f texCoordsOffset = {
            static_cast<float>(textureSubdivisionSize) / texture_.getSize().x,
            static_cast<float>(textureSubdivisionSize) / texture_.getSize().y
        };
        staticInit_->chunkShader->setUniform("texCoordsOffset", texCoordsOffset);
        staticInit_->chunkShader->setUniform("bufferSize", static_cast<sf::Vector2f>(maxChunkArea_));
        staticInit_->chunkShader->setUniform("zoomPow2", static_cast<float>(1 << levelOfDetail_));
    }
    if (bufferVertices_.size() > 0) {
        target.draw(buffer_, 0, bufferVertices_.size(), states);
    }
}
