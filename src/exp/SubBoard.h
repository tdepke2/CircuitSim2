#pragma once

#include <Chunk.h>
#include <ChunkCoords.h>
#include <ChunkCoordsRange.h>
#include <ChunkDrawable.h>
#include <FlatMap.h>

#include <memory>
#include <SFML/Graphics.hpp>
#include <unordered_map>

class OffsetView;
class Tile;

class SubBoard : public sf::Drawable, public sf::Transformable {
public:
    static void setup(unsigned int tileWidth);

    SubBoard();
    ~SubBoard() = default;
    SubBoard(const SubBoard& rhs) = delete;
    SubBoard& operator=(const SubBoard& rhs) = delete;

    void setSize(const sf::Vector2u& size);
    void setRenderArea(const OffsetView& offsetView, float zoom);
    void drawChunks(sf::RenderStates states);
    Chunk& accessChunk(ChunkCoords::repr coords);
    Tile accessTile(int x, int y);
    Tile accessTile(const sf::Vector2i& pos);

private:
    static unsigned int tileWidth_;

    void updateVisibleArea(const OffsetView& offsetView);
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    sf::Vector2u size_;
    std::unordered_map<ChunkCoords::repr, Chunk> chunks_;
    std::unique_ptr<Chunk> emptyChunk_;
    FlatMap<ChunkCoords::repr, ChunkDrawable> chunkDrawables_;
    int levelOfDetail_;
    ChunkCoordsRange visibleArea_;
    ChunkCoordsRange lastVisibleArea_;
    sf::RenderTexture texture_;
    sf::VertexArray vertices_;
};
