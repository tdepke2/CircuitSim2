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

class OffsetView;
class ResourceManager;
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
class Board : public sf::Drawable, LodRenderer {
public:
    static void setupTextures(ResourceManager& resource, const fs::path& filenameGrid, const fs::path& filenameNoGrid, unsigned int tileWidth);

    Board();
    ~Board() = default;
    Board(const Board& rhs) = delete;
    Board& operator=(const Board& rhs) = delete;

    void setRenderArea(const OffsetView& offsetView, float zoom);
    void setMaxSize(const sf::Vector2u& size);
    void setExtraLogicStates(bool extraLogicStates);
    void setNotesString(const sf::String& notes);
    const sf::Vector2u& getMaxSize() const;
    bool getExtraLogicStates() const;
    const sf::String& getNotesString() const;
    const std::unordered_map<ChunkCoords::repr, Chunk>& getLoadedChunks() const;

    bool isChunkLoaded(ChunkCoords::repr coords) const;
    void loadChunk(Chunk&& chunk);
    Chunk& accessChunk(ChunkCoords::repr coords);
    Tile accessTile(int x, int y);
    Tile accessTile(const sf::Vector2i& pos);
    void removeAllHighlights();
    void loadFromFile(const fs::path& filename);
    void saveToFile();
    void debugPrintChunk(ChunkCoords::repr i) {
        chunks_.at(i).debugPrintChunk();
    }
    void debugSetDrawChunkBorder(bool enabled);
    unsigned int debugGetChunksDrawn() const;

private:
    static sf::Texture* tilesetGrid_;
    static sf::Texture* tilesetNoGrid_;
    static unsigned int tileWidth_;

    void pruneChunkDrawables();
    void updateRender();
    virtual void markChunkDrawDirty(ChunkCoords::repr coords) override;
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    std::unique_ptr<FileStorage> fileStorage_;
    sf::Vector2u maxSize_;
    bool extraLogicStates_;
    sf::Text notesText_;
    std::unordered_map<ChunkCoords::repr, Chunk> chunks_;
    std::unique_ptr<Chunk> emptyChunk_;
    FlatMap<ChunkCoords::repr, ChunkDrawable> chunkDrawables_;
    std::array<ChunkRender, ChunkRender::LEVELS_OF_DETAIL> chunkRenderCache_;
    ChunkCoordsRange lastVisibleArea_;
    mutable sf::VertexArray debugChunkBorder_;
    bool debugDrawChunkBorder_;
};
