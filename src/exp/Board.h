#pragma once

#include <Chunk.h>

#include <array>
#include <map>
#include <memory>
#include <SFML/Graphics.hpp>
#include <string>

class ResourceManager;
class Tile;
struct TileSymbol;

class Board : public sf::Drawable, public sf::Transformable {
public:
    static void setupTextures(ResourceManager& resource, const std::string& filenameGrid, const std::string& filenameNoGrid, unsigned int tileWidth);

    Board();
    void setView(const sf::View& view);
    const sf::View& getView() const;
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
    Chunk& getChunk(int x, int y);
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    sf::Vector2u maxSize_;
    std::map<uint64_t, Chunk> chunks_;
    sf::View currentView_;
    Chunk emptyChunk_;    // FIXME used for testing now to render empty chunk, should be changed to just be a deterministic section of the chunk rendertarget cache.
    sf::RenderTexture testChunkRenderCache_;
    mutable unsigned int debugChunksDrawn_;
};
