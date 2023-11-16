#pragma once

#include <Chunk.h>
#include <ChunkCoords.h>
#include <ChunkDrawable.h>
#include <ChunkRender.h>
#include <FileStorage.h>
#include <FlatMap.h>

#include <array>
#include <memory>
#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>

class OffsetView;
class ResourceManager;
class Tile;
struct TileSymbol;

class Board : public sf::Drawable {
public:
    static void setupTextures(ResourceManager& resource, const std::string& filenameGrid, const std::string& filenameNoGrid, unsigned int tileWidth);

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
    Tile accessTile(int x, int y);
    void loadFromFile(const std::string& filename);
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

    Chunk& getChunk(ChunkCoords::repr coords);
    void pruneChunkDrawables();
    void updateRender();
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    std::unique_ptr<FileStorage> fileStorage_;
    sf::Vector2u maxSize_;
    bool extraLogicStates_;
    sf::Text notesText_;
    std::unordered_map<ChunkCoords::repr, Chunk> chunks_;
    FlatMap<ChunkCoords::repr, ChunkDrawable> chunkDrawables_;
    int currentLod_;
    std::array<ChunkRender, ChunkRender::LEVELS_OF_DETAIL> chunkRenderCache_;
    sf::IntRect lastVisibleArea_;
    mutable sf::VertexArray debugChunkBorder_;
    bool debugDrawChunkBorder_;
};
