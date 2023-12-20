#include <DebugScreen.h>
#include <OffsetView.h>
#include <SubBoard.h>
#include <Tile.h>
#include <tiles/Wire.h>

#include <cmath>
#include <spdlog/spdlog.h>
#include <sstream>
#include <string>
#include <utility>

unsigned int SubBoard::tileWidth_;

void SubBoard::setup(unsigned int tileWidth) {
    tileWidth_ = tileWidth;
}

SubBoard::SubBoard() :
    size_(),
    chunks_(),
    chunkDrawables_(),
    levelOfDetail_(0),
    lastVisibleArea_(0, 0, 0, 0),
    visibleAreaChanged_(false),
    texture_(),
    vertices_(sf::Triangles, 6) {

    auto coords = ChunkCoords::pack(0, 0);
    auto chunk = chunks_.emplace(std::piecewise_construct, std::forward_as_tuple(coords), std::forward_as_tuple(nullptr, coords)).first;
    chunkDrawables_[coords].setChunk(&chunk->second);
    chunk->second.accessTile(0).setType(tiles::Wire::instance(), TileId::wireTee, Direction::north, State::high);
}

void SubBoard::setSize(const sf::Vector2u& size) {
    if (size_ != size) {
        size_ = size;
        lastVisibleArea_ = {0, 0, 0, 0};
        visibleAreaChanged_ = true;
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
        visibleAreaChanged_ = true;
    }

    if (texture_.getSize() != textureSize) {
        spdlog::warn("Resizing SubBoard area to {} by {} chunks.", pow2ChunkArea.x, pow2ChunkArea.y);    // FIXME set to warn for test.
        lastVisibleArea_ = {0, 0, 0, 0};
        visibleAreaChanged_ = true;

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
    if (!visibleAreaChanged_) {
        return;
    }
    spdlog::debug("SubBoard area changed, redrawing.");
    visibleAreaChanged_ = false;
    states.transform.scale(
        1.0f / (1 << levelOfDetail_),
        1.0f / (1 << levelOfDetail_)
    );
    texture_.draw(chunkDrawables_.at(ChunkCoords::pack(0, 0)), states);

    texture_.display();
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
    ChunkCoordsRange visibleArea(topLeft.x, topLeft.y, size.x, size.y);

    if (lastVisibleArea_ == visibleArea) {
        return;
    }

    lastVisibleArea_ = visibleArea;
    visibleAreaChanged_ = true;

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
