#pragma once

#include <Chunk.h>
#include <ChunkCoords.h>
#include <ChunkCoordsRange.h>
#include <ChunkDrawable.h>
#include <commands/PlaceTiles.h>
#include <FlatMap.h>
#include <LodRenderer.h>

#include <memory>
#include <SFML/Graphics.hpp>
#include <unordered_map>

class Board;
class OffsetView;
class Tile;

class SubBoard : public sf::Drawable, LodRenderer {
public:
    SubBoard();
    ~SubBoard() = default;
    SubBoard(const SubBoard& rhs) = delete;
    SubBoard& operator=(const SubBoard& rhs) = delete;

    void setRenderArea(const OffsetView& offsetView, float zoom, const sf::Vector2i& tilePosition);
    void setVisibleSize(const sf::Vector2u& size);
    const sf::Vector2f& getRenderPosition() const;
    const sf::Vector2u& getVisibleSize() const;
    void drawChunks(const sf::Texture* tileset);
    Chunk& accessChunk(ChunkCoords::repr coords);
    Tile accessTile(int x, int y);
    Tile accessTile(const sf::Vector2i& pos);
    void rotate(bool clockwise);
    void flip(bool vertical);
    void clear();
    void copyFromBoard(Board& board, sf::Vector2i first, sf::Vector2i second, bool highlightsOnly = false);
    void pasteToBoard(commands::PlaceTiles& command, const sf::Vector2i& pos, bool ignoreBlanks = false);

private:
    void resetChunkDraw();
    void updateVisibleArea(const OffsetView& offsetView, const sf::Vector2i& tilePosition);
    virtual void markChunkDrawDirty(ChunkCoords::repr coords) override;
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    sf::Vector2f position_;
    sf::Vector2u size_, lastSize_;
    std::unordered_map<ChunkCoords::repr, Chunk> chunks_;
    std::unique_ptr<Chunk> emptyChunk_;
    FlatMap<ChunkCoords::repr, ChunkDrawable> chunkDrawables_;
    ChunkCoordsRange visibleArea_, lastVisibleArea_;
    const sf::Texture* lastTileset_;
    sf::RenderTexture texture_;
    sf::VertexArray vertices_;
};
