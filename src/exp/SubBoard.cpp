#include <DebugScreen.h>
#include <OffsetView.h>
#include <SubBoard.h>
#include <Tile.h>

#include <cmath>
#include <sstream>
#include <string>
#include <utility>

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

unsigned int SubBoard::tileWidth_;

namespace {

constexpr int constLog2(int x) {
    return x == 1 ? 0 : 1 + constLog2(x / 2);
}

}

void SubBoard::setup(unsigned int tileWidth) {
    tileWidth_ = tileWidth;
}

SubBoard::SubBoard() :
    size_(),
    chunks_(),
    emptyChunk_(new Chunk(nullptr, ChunkRender::EMPTY_CHUNK_COORDS)),
    chunkDrawables_(),
    levelOfDetail_(0),
    visibleArea_(0, 0, 0, 0),
    lastVisibleArea_(0, 0, 0, 0),
    texture_(),
    vertices_(sf::Triangles, 6) {

    chunkDrawables_[ChunkRender::EMPTY_CHUNK_COORDS].setChunk(emptyChunk_.get());
}

void SubBoard::setSize(const sf::Vector2u& size) {
    if (size_ != size) {
        size_ = size;
        lastVisibleArea_ = {0, 0, 0, 0};
    }
}

void SubBoard::setRenderArea(const OffsetView& offsetView, float zoom) {
    // from Board ##############################################################
    int currentLod = static_cast<int>(std::max(std::floor(std::log2(zoom)), 0.0f));
    const sf::Vector2f maxViewSize = offsetView.getSize() / zoom * static_cast<float>(1 << (currentLod + 1));
    const int chunkWidthTexels = Chunk::WIDTH * static_cast<int>(tileWidth_);
    const sf::Vector2u maxChunkArea = {
        static_cast<unsigned int>(std::ceil(std::round(maxViewSize.x) / chunkWidthTexels)) + 1,
        static_cast<unsigned int>(std::ceil(std::round(maxViewSize.y) / chunkWidthTexels)) + 1
    };

    // from ChunkRender ########################################################
    //const int chunkWidthTexels = Chunk::WIDTH * static_cast<int>(tileWidth_);
    const int textureSubdivisionSize = chunkWidthTexels / (1 << currentLod);
    const sf::Vector2u pow2ChunkArea = {
        1u << static_cast<unsigned int>(std::ceil(std::log2(maxChunkArea.x))),
        1u << static_cast<unsigned int>(std::ceil(std::log2(maxChunkArea.y)))
    };
    const sf::Vector2u textureSize = pow2ChunkArea * static_cast<unsigned int>(textureSubdivisionSize);

    if (levelOfDetail_ != currentLod) {
        levelOfDetail_ = currentLod;
        lastVisibleArea_ = {0, 0, 0, 0};
    }

    if (texture_.getSize() != textureSize) {
        spdlog::warn("Resizing SubBoard area to {} by {} chunks.", pow2ChunkArea.x, pow2ChunkArea.y);    // FIXME set to warn for test.
        lastVisibleArea_ = {0, 0, 0, 0};

        if (!texture_.create(textureSize.x, textureSize.y)) {
            spdlog::error("Failed to create SubBoard texture (size {} by {}).", textureSize.x, textureSize.y);
        }
        texture_.clear(sf::Color::Black);
        texture_.setSmooth(true);

        std::stringstream address;
        address << static_cast<const void*>(this);
        DebugScreen::instance()->registerTexture("subBoard " + address.str(), &texture_.getTexture());
    }

    updateVisibleArea(offsetView);
}

void SubBoard::drawChunks(sf::RenderStates states) {
    if (lastVisibleArea_ == visibleArea_) {
        return;
    }
    spdlog::debug("SubBoard area changed, redrawing.");
    if (lastVisibleArea_.getFirst() != visibleArea_.getFirst()) {
        lastVisibleArea_ = {0, 0, 0, 0};
    }





    // FIXME copied this from Board, we need to check if lastVisibleArea_ contains each chunk or it's render dirty

    auto drawChunk = [this](const ChunkDrawable& chunkDrawable, sf::RenderStates states, int x, int y) {
        const int chunkWidthTexels = Chunk::WIDTH * static_cast<int>(tileWidth_);
        const int textureSubdivisionSize = chunkWidthTexels / (1 << levelOfDetail_);
        states.transform.translate(
            static_cast<float>(x * textureSubdivisionSize),
            static_cast<float>(y * textureSubdivisionSize)
        );
        states.transform.scale(
            1.0f / (1 << levelOfDetail_),
            1.0f / (1 << levelOfDetail_)
        );
        spdlog::debug("Redrawing SubBoard chunk at relative coords {}, {}.", x, y);
        texture_.draw(chunkDrawable, states);
    };

    const auto& emptyChunkDrawable = chunkDrawables_.at(ChunkRender::EMPTY_CHUNK_COORDS);
    for (int y = visibleArea_.top; y < visibleArea_.top + visibleArea_.height; ++y) {
        auto chunkDrawable = chunkDrawables_.upper_bound(ChunkCoords::pack(visibleArea_.left - 1, y));
        for (int x = visibleArea_.left; x < visibleArea_.left + visibleArea_.width; ++x) {
            if (chunkDrawable == chunkDrawables_.end() || chunkDrawable->first != ChunkCoords::pack(x, y)) {
                drawChunk(emptyChunkDrawable, states, x - visibleArea_.left, y - visibleArea_.top);
            } else {
                //if (chunkDrawable->second.isRenderDirty(levelOfDetail_)) {    // FIXME how to check for render dirty?
                    drawChunk(chunkDrawable->second, states, x - visibleArea_.left, y - visibleArea_.top);
                //}
                ++chunkDrawable;
            }
        }
    }

    texture_.display();
    lastVisibleArea_ = visibleArea_;
}

Chunk& SubBoard::accessChunk(ChunkCoords::repr coords) {
    auto chunk = chunks_.find(coords);
    if (chunk != chunks_.end()) {
        return chunk->second;
    }

    spdlog::debug("SubBoard allocating new chunk at {}.", ChunkCoords::toPair(coords));
    chunk = chunks_.emplace(std::piecewise_construct, std::forward_as_tuple(coords), std::forward_as_tuple(nullptr, coords)).first;
    chunkDrawables_[coords].setChunk(&chunk->second);
    return chunk->second;
}

Tile SubBoard::accessTile(int x, int y) {
    constexpr int widthLog2 = constLog2(Chunk::WIDTH);
    auto& chunk = accessChunk(ChunkCoords::pack(x >> widthLog2, y >> widthLog2));
    return chunk.accessTile((x & (Chunk::WIDTH - 1)) + (y & (Chunk::WIDTH - 1)) * Chunk::WIDTH);
}

Tile SubBoard::accessTile(const sf::Vector2i& pos) {
    return accessTile(pos.x, pos.y);
}

void SubBoard::updateVisibleArea(const OffsetView& offsetView) {
    // from Board ##############################################################
    const int chunkWidthTexels = Chunk::WIDTH * static_cast<int>(tileWidth_);
    sf::Vector2i topLeft = {
        static_cast<int>(std::floor((offsetView.getCenter().x - offsetView.getSize().x / 2.0f) / chunkWidthTexels)),
        static_cast<int>(std::floor((offsetView.getCenter().y - offsetView.getSize().y / 2.0f) / chunkWidthTexels))
    };
    topLeft += offsetView.getCenterOffset();
    sf::Vector2i size = {
        static_cast<int>(std::floor((offsetView.getCenter().x + offsetView.getSize().x / 2.0f) / chunkWidthTexels)),
        static_cast<int>(std::floor((offsetView.getCenter().y + offsetView.getSize().y / 2.0f) / chunkWidthTexels))
    };
    size += offsetView.getCenterOffset() - topLeft + sf::Vector2i(1, 1);
    visibleArea_ = ChunkCoordsRange(topLeft.x, topLeft.y, size.x, size.y);

    if (lastVisibleArea_ == visibleArea_) {
        return;
    }

    const sf::Vector2f p1(0.0f, 0.0f);
    const sf::Vector2f p2(p1.x + chunkWidthTexels, p1.y + chunkWidthTexels);
    vertices_[0].position = p1;
    vertices_[1].position = {p2.x, p1.y};
    vertices_[2].position = p2;
    vertices_[3].position = p2;
    vertices_[4].position = {p1.x, p2.y};
    vertices_[5].position = p1;

    const sf::Vector2f t1 = p1;
    const sf::Vector2f t2(p1.x + static_cast<int>(chunkWidthTexels / (1 << levelOfDetail_)), p1.y + static_cast<int>(chunkWidthTexels / (1 << levelOfDetail_)));
    vertices_[0].texCoords = t1;
    vertices_[1].texCoords = {t2.x, t1.y};
    vertices_[2].texCoords = t2;
    vertices_[3].texCoords = t2;
    vertices_[4].texCoords = {t1.x, t2.y};
    vertices_[5].texCoords = t1;
}

void SubBoard::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    states.transform *= getTransform();
    states.texture = &texture_.getTexture();
    target.draw(vertices_, states);
}
