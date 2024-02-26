#include <Chunk.h>
#include <LodRenderer.h>
#include <OffsetView.h>
#include <TileWidth.h>

#include <algorithm>
#include <cassert>
#include <cmath>

constexpr int LodRenderer::LEVELS_OF_DETAIL;
constexpr ChunkCoords::repr LodRenderer::EMPTY_CHUNK_COORDS;

LodRenderer::LodRenderer() :
    renderableEntities_(),
    levelOfDetail_(0) {
}

void LodRenderer::addRenderable(const Entity* entity) {
    assert(renderableEntities_.emplace(entity, true).second);
}

void LodRenderer::removeRenderable(const Entity* entity) {
    assert(renderableEntities_.erase(entity) == 1);
}

int LodRenderer::getLevelOfDetail() const {
    return levelOfDetail_;
}

void LodRenderer::setLevelOfDetail(int lod) {
    levelOfDetail_ = std::min(std::max(lod, 0), LEVELS_OF_DETAIL - 1);
}

sf::Vector2u LodRenderer::getMaxVisibleChunkArea(const OffsetView& offsetView, float zoom) const {
    const sf::Vector2f maxViewSize = offsetView.getSize() / zoom * static_cast<float>(1 << (levelOfDetail_ + 1));
    constexpr int chunkWidthTexels = Chunk::WIDTH * static_cast<int>(TileWidth::TEXELS);
    return {
        static_cast<unsigned int>(std::ceil(std::round(maxViewSize.x) / chunkWidthTexels)) + 1,
        static_cast<unsigned int>(std::ceil(std::round(maxViewSize.y) / chunkWidthTexels)) + 1
    };
}
