#include <Chunk.h>
#include <LodRenderer.h>
#include <OffsetView.h>

#include <cmath>

constexpr ChunkCoords::repr LodRenderer::EMPTY_CHUNK_COORDS;

LodRenderer::LodRenderer() :
    levelOfDetail_(0) {
}

int LodRenderer::getLevelOfDetail() const {
    return levelOfDetail_;
}

void LodRenderer::setLevelOfDetail(int lod) {
    levelOfDetail_ = lod;
}

sf::Vector2u LodRenderer::getMaxVisibleChunkArea(const OffsetView& offsetView, float zoom, unsigned int tileWidth) const {
    const sf::Vector2f maxViewSize = offsetView.getSize() / zoom * static_cast<float>(1 << (levelOfDetail_ + 1));
    const int chunkWidthTexels = Chunk::WIDTH * static_cast<int>(tileWidth);
    return {
        static_cast<unsigned int>(std::ceil(std::round(maxViewSize.x) / chunkWidthTexels)) + 1,
        static_cast<unsigned int>(std::ceil(std::round(maxViewSize.y) / chunkWidthTexels)) + 1
    };
}
