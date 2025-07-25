#pragma once

#include <Chunk.h>
#include <ChunkCoords.h>
#include <ChunkCoordsRange.h>
#include <ChunkDrawable.h>
#include <ChunkRender.h>
#include <FileStorage.h>
#include <Filesystem.h>
#include <FlatMap.h>
#include <LodRenderer.h>

#include <array>
#include <memory>
#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>
#include <utility>

class OffsetView;
class Tile;

/**
 * A circuit board with a grid of circuit tiles.
 * 
 * The grid may be fixed size, or "unlimited" (the max size of a 32-bit signed
 * integer in each direction). Boards can be drawn to an `sf::RenderTarget` and
 * will load/unload chunks as needed depending on what's visible. Drawing also
 * uses different levels-of-detail based on the zoom level. Boards can also
 * save/load to a file and work with different file formats.
 */
class Board : public sf::Drawable, private LodRenderer {
public:
    Board();
    ~Board() = default;
    Board(const Board& rhs) = delete;
    Board& operator=(const Board& rhs) = delete;

    void setRenderArea(const OffsetView& offsetView, float zoom);
    // The max size is rounded up to a multiple of `Chunk::WIDTH` when setting.
    void setMaxSize(const sf::Vector2u& size);
    void setExtraLogicStates(bool extraLogicStates);
    void setNotesString(const sf::String& notes);
    const fs::path& getFilename() const;
    bool isNewBoard() const;
    fs::path getDefaultFileExtension() const;
    // A max size of zero indicates no size limit.
    const sf::Vector2u& getMaxSize() const;
    sf::Vector2i getTileLowerBound() const;
    sf::Vector2i getTileUpperBound() const;
    bool getExtraLogicStates() const;
    const sf::String& getNotesString() const;
    const std::unordered_map<ChunkCoords::repr, Chunk>& getLoadedChunks() const;

    void forceLoadAllChunks();
    bool isChunkLoaded(ChunkCoords::repr coords) const;
    void loadChunk(Chunk&& chunk);
    Chunk& accessChunk(ChunkCoords::repr coords);
    Tile accessTile(int x, int y);
    Tile accessTile(const sf::Vector2i& pos);
    void removeAllHighlights();
    /**
     * Returns the lower and upper bound (inclusive) of the minimum rectangular
     * area that contains all highlighted tiles. If nothing is highlighted, the
     * returned lower bound will be greater than the upper bound.
     */
    std::pair<sf::Vector2i, sf::Vector2i> getHighlightedBounds();
    void newBoard(const sf::Vector2u& size = {64, 64});
    bool loadFromFile(const fs::path& filename);
    bool saveToFile();
    bool saveAsFile(const fs::path& filename);
    void rename();
    void resize();
    void debugPrintChunk(ChunkCoords::repr i) {
        chunks_.at(i).debugPrintChunk();
    }
    void debugSetDrawChunkBorder(bool enabled);
    unsigned int debugGetChunksDrawn() const;

private:
    struct StaticInit {
        StaticInit();
        sf::Texture* tilesetGrid;
        sf::Texture* tilesetNoGrid;
    };
    static StaticInit* staticInit_;

    void clearChunks();
    void pruneChunkDrawables();
    void updateRender();
    virtual void markChunkDrawDirty(ChunkCoords::repr coords) override;
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    std::unique_ptr<FileStorage> fileStorage_;
    const fs::path workingDirectory_;
    sf::Vector2u maxSize_;
    bool extraLogicStates_;
    sf::Text notesText_;
    std::unordered_map<ChunkCoords::repr, Chunk> chunks_;
    std::unique_ptr<Chunk> emptyChunk_;
    FlatMap<ChunkCoords::repr, ChunkDrawable> chunkDrawables_;
    std::array<ChunkRender, LodRenderer::LEVELS_OF_DETAIL> chunkRenderCache_;
    ChunkCoordsRange lastVisibleArea_;
    ChunkCoords::repr lastTopLeft_;
    mutable sf::VertexArray debugChunkBorder_;
    bool debugDrawChunkBorder_;
};
