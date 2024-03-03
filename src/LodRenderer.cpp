#include <Chunk.h>
#include <LodRenderer.h>
#include <OffsetView.h>
#include <TileWidth.h>

#include <algorithm>
#include <cmath>

constexpr int LodRenderer::LEVELS_OF_DETAIL;
constexpr ChunkCoords::repr LodRenderer::EMPTY_CHUNK_COORDS;

LodRenderer::LodRenderer() :
    decorations_(),
    levelOfDetail_(0) {
}

void LodRenderer::addDecoration(ChunkCoords::repr coords, unsigned int tileIndex, const sf::Drawable* drawable) {
    decorations_[coords][tileIndex] = drawable;
}

void LodRenderer::removeDecoration(ChunkCoords::repr coords, unsigned int tileIndex, const sf::Drawable* drawable) {
    auto chunkDeco = decorations_.find(coords);
    auto deco = chunkDeco->second.find(tileIndex);
    if (deco->second != drawable) {
        // Do nothing if there is a different drawable at the selected coordinates.
        return;
    }
    chunkDeco->second.erase(deco);
    if (chunkDeco->second.empty()) {
        decorations_.erase(chunkDeco);
    }
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

void LodRenderer::drawDecorations(sf::RenderTarget& target, sf::RenderStates states, const ChunkCoordsRange& visibleArea) const {
    constexpr int chunkWidthTexels = Chunk::WIDTH * static_cast<int>(TileWidth::TEXELS);

    for (int y = 0; y < visibleArea.height; ++y) {
        auto chunkDeco = decorations_.upper_bound(ChunkCoords::pack(visibleArea.left - 1, visibleArea.top + y));
        float yChunkPos = static_cast<float>(y * chunkWidthTexels);
        for (int x = 0; x < visibleArea.width; ++x) {
            float xChunkPos = static_cast<float>(x * chunkWidthTexels);
            if (chunkDeco != decorations_.end() && chunkDeco->first == ChunkCoords::pack(visibleArea.left + x, visibleArea.top + y)) {
                for (const auto& deco : chunkDeco->second) {
                    sf::RenderStates states2 = states;
                    states2.transform.translate(
                        xChunkPos + static_cast<unsigned int>(deco.first % Chunk::WIDTH) * TileWidth::TEXELS,
                        yChunkPos + static_cast<unsigned int>(deco.first / Chunk::WIDTH) * TileWidth::TEXELS
                    );
                    target.draw(*deco.second, states2);
                }
                ++chunkDeco;
            }
        }
    }
}
