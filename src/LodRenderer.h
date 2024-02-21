#pragma once

#include <ChunkCoords.h>

#include <SFML/Graphics.hpp>

class OffsetView;

class LodRenderer {
public:
    // Max number of detail levels, the first level is at zero.
    static constexpr int LEVELS_OF_DETAIL = 5;
    // Special coordinates for the empty chunk used in derived classes.
    static constexpr ChunkCoords::repr EMPTY_CHUNK_COORDS = 0;

    LodRenderer();
    virtual ~LodRenderer() noexcept = default;
    // FIXME should be private nonvirtual? probably should add virtual to other derived classes that missed it.
    LodRenderer(const LodRenderer& rhs) = default;
    LodRenderer& operator=(const LodRenderer& rhs) = default;

    virtual void markChunkDrawDirty(ChunkCoords::repr coords) = 0;

protected:
    int getLevelOfDetail() const;
    // The lod is clamped to the maximum/minimum bounds while setting.
    void setLevelOfDetail(int lod);
    sf::Vector2u getMaxVisibleChunkArea(const OffsetView& offsetView, float zoom) const;

private:
    int levelOfDetail_;
};
