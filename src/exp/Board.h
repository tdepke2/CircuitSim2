#pragma once

#include <Chunk.h>
#include <ChunkDrawable.h>
#include <ChunkRender.h>
#include <FlatMap.h>

#include <array>
#include <map>
#include <memory>
#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>

class OffsetView;
class ResourceManager;
class Tile;
struct TileSymbol;

using ChunkCoords = uint64_t;

class Board : public sf::Drawable {
public:
    static void setupTextures(ResourceManager& resource, const std::string& filenameGrid, const std::string& filenameNoGrid, unsigned int tileWidth);

    Board();
    void setRenderArea(const OffsetView& offsetView, float zoom);
    Tile accessTile(int x, int y);
    void loadFromFile(const std::string& filename);
    void saveToFile(const std::string& filename);
    void debugPrintChunk(ChunkCoords i) {
        chunks_.at(i).debugPrintChunk();
    }
    void debugRedrawChunks() {
        //for (auto& chunk : chunks_) {
            //chunk.second.forceRedraw();
        //}
    }
    void debugSetDrawChunkBorder(bool enabled);
    unsigned int debugGetChunksDrawn() const;

private:
    static sf::Texture* tilesetGrid_;
    static sf::Texture* tilesetNoGrid_;
    static unsigned int tileWidth_;

    struct ParseState {
        std::string lastField = "";
        std::string filename = "";
        float fileVersion = 1.0;
        unsigned int width = 0;
        unsigned int height = 0;
        bool enableExtraLogicStates = false;
        std::string notesString = "";
        int x = 0;
        int y = 0;
    };

    void parseFile(const std::string& line, int lineNumber, ParseState& parseState, const std::map<TileSymbol, unsigned int>& symbolLookup);
    Chunk& getChunk(ChunkCoords coords);
    void pruneChunkDrawables();
    void updateRender();
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    sf::Vector2u maxSize_;
    std::unordered_map<ChunkCoords, Chunk> chunks_;
    FlatMap<ChunkCoords, ChunkDrawable> chunkDrawables_;
    int currentLod_;
    std::array<ChunkRender, ChunkRender::LEVELS_OF_DETAIL> chunkRenderCache_;
    sf::IntRect lastVisibleArea_;
    mutable sf::VertexArray debugChunkBorder_;
    bool debugDrawChunkBorder_;
    mutable unsigned int debugChunksDrawn_;
};
