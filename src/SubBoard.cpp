#include <Board.h>
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
    position_(),
    size_(),
    lastSize_(),
    chunks_(),
    emptyChunk_(new Chunk(nullptr, LodRenderer::EMPTY_CHUNK_COORDS)),
    chunkDrawables_(),
    visibleArea_(0, 0, 0, 0),
    lastVisibleArea_(0, 0, 0, 0),
    lastTileset_(nullptr),
    texture_(),
    vertices_(sf::Triangles, 6) {

    chunkDrawables_[LodRenderer::EMPTY_CHUNK_COORDS].setChunk(emptyChunk_.get());
}

void SubBoard::setRenderArea(const OffsetView& offsetView, float zoom, const sf::Vector2i& tilePosition) {
    int lastLevelOfDetail = getLevelOfDetail();
    setLevelOfDetail(static_cast<int>(std::floor(std::log2(zoom))));
    bool levelOfDetailChanged = (lastLevelOfDetail != getLevelOfDetail());
    const sf::Vector2u maxChunkArea = getMaxVisibleChunkArea(offsetView, zoom, tileWidth_);

    const int chunkWidthTexels = Chunk::WIDTH * static_cast<int>(tileWidth_);
    const int textureSubdivisionSize = chunkWidthTexels / (1 << getLevelOfDetail());
    const sf::Vector2u pow2ChunkArea = {
        1u << static_cast<unsigned int>(std::ceil(std::log2(maxChunkArea.x))),
        1u << static_cast<unsigned int>(std::ceil(std::log2(maxChunkArea.y)))
    };
    const sf::Vector2u textureSize = pow2ChunkArea * static_cast<unsigned int>(textureSubdivisionSize);

    if (texture_.getSize() != textureSize) {
        spdlog::warn("Resizing SubBoard area to {} by {} chunks.", pow2ChunkArea.x, pow2ChunkArea.y);    // FIXME set to warn for test.
        if (!texture_.create(textureSize.x, textureSize.y)) {
            spdlog::error("Failed to create SubBoard texture (size {} by {}).", textureSize.x, textureSize.y);
        }
        resetChunkDraw();
        texture_.setSmooth(true);

        std::stringstream address;
        address << static_cast<const void*>(this);
        DebugScreen::instance()->registerTexture("subBoard " + address.str(), &texture_.getTexture());
    } else if (levelOfDetailChanged) {
        resetChunkDraw();
    }

    updateVisibleArea(offsetView, tilePosition);
}

void SubBoard::setVisibleSize(const sf::Vector2u& size) {
    size_ = size;
}

const sf::Vector2f& SubBoard::getRenderPosition() const {
    return position_;
}

const sf::Vector2u& SubBoard::getVisibleSize() const {
    return size_;
}

void SubBoard::drawChunks(const sf::Texture* tileset) {
    if (lastVisibleArea_.getFirst() != visibleArea_.getFirst() || lastTileset_ != tileset) {
        resetChunkDraw();
    }

    auto drawChunk = [this](const ChunkDrawable& chunkDrawable, const sf::Texture* tileset, bool& textureDirty, int x, int y) {
        const int chunkWidthTexels = Chunk::WIDTH * static_cast<int>(tileWidth_);
        const int textureSubdivisionSize = chunkWidthTexels / (1 << getLevelOfDetail());
        sf::RenderStates states(tileset);
        states.transform.translate(
            static_cast<float>((x - visibleArea_.left) * textureSubdivisionSize),
            static_cast<float>((y - visibleArea_.top) * textureSubdivisionSize)
        );
        states.transform.scale(
            1.0f / (1 << getLevelOfDetail()),
            1.0f / (1 << getLevelOfDetail())
        );
        spdlog::warn("Redrawing SubBoard chunk at {}, {}.", x, y);
        texture_.draw(chunkDrawable, states);
        textureDirty = true;
        chunkDrawable.markAsDrawn(getLevelOfDetail());
    };

    bool textureDirty = false;
    const auto& emptyChunkDrawable = chunkDrawables_.at(LodRenderer::EMPTY_CHUNK_COORDS);
    for (int y = visibleArea_.top; y < visibleArea_.top + visibleArea_.height && y * Chunk::WIDTH < static_cast<int>(size_.y); ++y) {
        auto chunkDrawable = chunkDrawables_.upper_bound(ChunkCoords::pack(visibleArea_.left - 1, y));
        for (int x = visibleArea_.left; x < visibleArea_.left + visibleArea_.width && x * Chunk::WIDTH < static_cast<int>(size_.x); ++x) {
            if (chunkDrawable == chunkDrawables_.end() || chunkDrawable->first != ChunkCoords::pack(x, y)) {
                if (!lastVisibleArea_.contains(x, y)) {
                    drawChunk(emptyChunkDrawable, tileset, textureDirty, x, y);
                }
            } else {
                if (chunkDrawable->second.isRenderDirty(getLevelOfDetail()) || !lastVisibleArea_.contains(x, y)) {
                    drawChunk(chunkDrawable->second, tileset, textureDirty, x, y);
                }
                ++chunkDrawable;
            }
        }
    }

    if (textureDirty) {
        texture_.display();
    }
    lastVisibleArea_ = visibleArea_;
    lastTileset_ = tileset;
}

Chunk& SubBoard::accessChunk(ChunkCoords::repr coords) {
    auto chunk = chunks_.find(coords);
    if (chunk != chunks_.end()) {
        return chunk->second;
    }

    spdlog::warn("SubBoard allocating new chunk at {}.", ChunkCoords::toPair(coords));
    chunk = chunks_.emplace(std::piecewise_construct, std::forward_as_tuple(coords), std::forward_as_tuple(static_cast<LodRenderer*>(this), coords)).first;
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

void SubBoard::clear() {
    chunks_.clear();
    chunkDrawables_.clear();
    chunkDrawables_[LodRenderer::EMPTY_CHUNK_COORDS].setChunk(emptyChunk_.get());
    setVisibleSize({0, 0});
}

void SubBoard::copyFromBoard(Board& board, sf::Vector2i first, sf::Vector2i second, bool highlightsOnly) {
    sf::Vector2i firstCopy = first;
    first.x = std::min(firstCopy.x, second.x);
    first.y = std::min(firstCopy.y, second.y);
    second.x = std::max(firstCopy.x, second.x);
    second.y = std::max(firstCopy.y, second.y);
    sf::Vector2i sizeSubOne = second - first;

    constexpr int widthLog2 = constLog2(Chunk::WIDTH);
    for (int yChunk = 0; yChunk <= (sizeSubOne.y >> widthLog2); ++yChunk) {
        for (int xChunk = 0; xChunk <= (sizeSubOne.x >> widthLog2); ++xChunk) {
            auto& chunk = accessChunk(ChunkCoords::pack(xChunk, yChunk));
            for (int i = 0; i < Chunk::WIDTH * Chunk::WIDTH; ++i) {
                int x = i % Chunk::WIDTH + xChunk * Chunk::WIDTH;
                int y = i / Chunk::WIDTH + yChunk * Chunk::WIDTH;
                if (x <= sizeSubOne.x && y <= sizeSubOne.y) {
                    const Tile tile = board.accessTile(x + first.x, y + first.y);
                    if (!highlightsOnly || tile.getHighlight()) {
                        tile.cloneTo(chunk.accessTile(i));
                    }
                }
            }
        }
    }
    size_ = static_cast<sf::Vector2u>(sizeSubOne + sf::Vector2i(1, 1));
}

void SubBoard::pasteToBoard(Board& board, const sf::Vector2i& pos, bool ignoreBlanks) {
    using Vector2ll = sf::Vector2<long long>;
    auto first = static_cast<Vector2ll>(pos);
    auto second = first + static_cast<Vector2ll>(size_) - Vector2ll(1ll, 1ll);

    const auto upperBound = static_cast<Vector2ll>(board.getTileUpperBound());
    second.x = std::min(second.x, upperBound.x);
    second.y = std::min(second.y, upperBound.y);

    constexpr int widthLog2 = constLog2(Chunk::WIDTH);
    for (int yChunk = static_cast<int>(first.y >> widthLog2); yChunk <= static_cast<int>(second.y >> widthLog2); ++yChunk) {
        for (int xChunk = static_cast<int>(first.x >> widthLog2); xChunk <= static_cast<int>(second.x >> widthLog2); ++xChunk) {
            auto& chunk = board.accessChunk(ChunkCoords::pack(xChunk, yChunk));
            for (int i = 0; i < Chunk::WIDTH * Chunk::WIDTH; ++i) {
                int x = i % Chunk::WIDTH + xChunk * Chunk::WIDTH;
                int y = i / Chunk::WIDTH + yChunk * Chunk::WIDTH;
                if (x >= first.x && x <= second.x && y >= first.y && y <= second.y) {
                    const Tile tile = accessTile(static_cast<int>(x - first.x), static_cast<int>(y - first.y));
                    if (!ignoreBlanks || tile.getId() != TileId::blank) {
                        tile.cloneTo(chunk.accessTile(i));
                    }
                }
            }
        }
    }
}

void SubBoard::resetChunkDraw() {
    lastVisibleArea_ = {0, 0, 0, 0};
    if (texture_.getSize() != sf::Vector2u(0, 0)) {
        texture_.clear({0, 0, 0, 0});
    }
}

void SubBoard::updateVisibleArea(const OffsetView& offsetView, const sf::Vector2i& tilePosition) {
    const int chunkWidthTexels = Chunk::WIDTH * static_cast<int>(tileWidth_);
    sf::Vector2i areaSize = {
        static_cast<int>(std::floor(offsetView.getSize().x / chunkWidthTexels)),
        static_cast<int>(std::floor(offsetView.getSize().y / chunkWidthTexels))
    };
    visibleArea_ = ChunkCoordsRange(0, 0, areaSize.x + 2, areaSize.y + 2);

    // Set the position relative to the top left of the offset view. The
    // position will be nudged over if the top left chunk would render beyond
    // the top or left side of the screen. Without this fix, the bottom right
    // corner of the texture could become visible showing an artifact where the
    // chunks stop rendering.
    position_ = static_cast<sf::Vector2f>((tilePosition - offsetView.getCenterOffset() * Chunk::WIDTH) * static_cast<int>(tileWidth_));
    sf::Vector2i topLeftTile = {
        offsetView.getCenterOffset().x * Chunk::WIDTH + static_cast<int>(std::floor((offsetView.getCenter().x - offsetView.getSize().x * 0.5f) / tileWidth_)),
        offsetView.getCenterOffset().y * Chunk::WIDTH + static_cast<int>(std::floor((offsetView.getCenter().y - offsetView.getSize().y * 0.5f) / tileWidth_))
    };
    if (topLeftTile.x - tilePosition.x >= Chunk::WIDTH) {
        const int numChunks = (topLeftTile.x - tilePosition.x) / Chunk::WIDTH;
        position_.x += numChunks * chunkWidthTexels;
        visibleArea_.left += numChunks;
    }
    if (topLeftTile.y - tilePosition.y >= Chunk::WIDTH) {
        const int numChunks = (topLeftTile.y - tilePosition.y) / Chunk::WIDTH;
        position_.y += numChunks * chunkWidthTexels;
        visibleArea_.top += numChunks;
    }

    if (lastVisibleArea_ == visibleArea_ && lastSize_ == size_) {
        return;
    }

    spdlog::warn(
        "SubBoard visible area or size changed, now {} to {} with size {} by {}.",
        ChunkCoords::toPair(visibleArea_.getFirst()), ChunkCoords::toPair(visibleArea_.getSecond()),
        size_.x, size_.y
    );
    const sf::Vector2f p1 = {0.0f, 0.0f};
    const sf::Vector2f p2 = {
        static_cast<float>(std::max(std::min(visibleArea_.width * chunkWidthTexels, static_cast<int>(size_.x * tileWidth_) - visibleArea_.left * chunkWidthTexels), 0)),
        static_cast<float>(std::max(std::min(visibleArea_.height * chunkWidthTexels, static_cast<int>(size_.y * tileWidth_) - visibleArea_.top * chunkWidthTexels), 0))
    };
    vertices_[0].position = p1;
    vertices_[1].position = {p2.x, p1.y};
    vertices_[2].position = p2;
    vertices_[3].position = p2;
    vertices_[4].position = {p1.x, p2.y};
    vertices_[5].position = p1;

    const sf::Vector2f t1 = {0.0f, 0.0f};
    const sf::Vector2f t2 = {p2.x / (1 << getLevelOfDetail()), p2.y / (1 << getLevelOfDetail())};
    vertices_[0].texCoords = t1;
    vertices_[1].texCoords = {t2.x, t1.y};
    vertices_[2].texCoords = t2;
    vertices_[3].texCoords = t2;
    vertices_[4].texCoords = {t1.x, t2.y};
    vertices_[5].texCoords = t1;

    const sf::Vector2u chunkSizeSubOne = {Chunk::WIDTH - 1, Chunk::WIDTH - 1};
    if ((lastSize_ + chunkSizeSubOne) / static_cast<unsigned int>(Chunk::WIDTH) != (size_ + chunkSizeSubOne) / static_cast<unsigned int>(Chunk::WIDTH)) {
        spdlog::warn("SubBoard size crossed chunk border, forcing redraw.");
        resetChunkDraw();
    }
    lastSize_ = size_;
}

void SubBoard::markChunkDrawDirty(ChunkCoords::repr coords) {
    chunkDrawables_.at(coords).markDirty();
}

void SubBoard::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    states.transform.translate(position_);
    states.texture = &texture_.getTexture();
    target.draw(vertices_, states);
}