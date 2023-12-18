#pragma once

#include <Chunk.h>
#include <ChunkCoords.h>
#include <ChunkDrawable.h>
#include <FlatMap.h>

#include <SFML/Graphics.hpp>
#include <unordered_map>

class OffsetView;

class ChunkGroup : public sf::Drawable {
public:
    ChunkGroup();
    ~ChunkGroup() = default;
    ChunkGroup(const ChunkGroup& rhs) = delete;
    ChunkGroup& operator=(const ChunkGroup& rhs) = delete;

    void setRenderArea(const OffsetView& offsetView, float zoom);

private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    std::unordered_map<ChunkCoords::repr, Chunk> chunks_;
    FlatMap<ChunkCoords::repr, ChunkDrawable> chunkDrawables_;
};
