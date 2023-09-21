#pragma once

#include <Chunk.h>

#include <array>
#include <map>
#include <memory>
#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>




#include <DebugScreen.h>



class ResourceManager;
class Tile;
struct TileSymbol;

class Board : public sf::Drawable, public sf::Transformable {
public:
    static constexpr int LEVELS_OF_DETAIL = 5;
    static void setupTextures(ResourceManager& resource, const std::string& filenameGrid, const std::string& filenameNoGrid, unsigned int tileWidth);

    Board();
    void setRenderArea(const sf::View& view, float zoom, DebugScreen& debugScreen);
    Tile accessTile(int x, int y);
    void loadFromFile(const std::string& filename);
    void saveToFile(const std::string& filename);
    void debugPrintChunk(uint64_t i) {
        chunks_.at(i).debugPrintChunk();
    }
    void debugRedrawChunks() {
        for (auto& chunk : chunks_) {
            chunk.second.forceRedraw();
        }
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

    struct ChunkRender {
        sf::RenderTexture texture;
        sf::VertexBuffer buffer;
    };

    void parseFile(const std::string& line, int lineNumber, ParseState& parseState, const std::map<TileSymbol, unsigned int>& symbolLookup);
    Chunk& getChunk(int x, int y);
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    sf::Vector2u maxSize_;
    std::unordered_map<uint64_t, Chunk> chunks_;
    sf::View currentView_;
    float currentZoom_;
    std::vector<ChunkRender> chunkRenderCache_;
    Chunk emptyChunk_;    // FIXME used for testing now to render empty chunk, should be changed to just be a deterministic section of the chunk rendertarget cache.
    mutable sf::VertexArray debugChunkBorder_;
    bool debugDrawChunkBorder_;
    mutable unsigned int debugChunksDrawn_;
};
