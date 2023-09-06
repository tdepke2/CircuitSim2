#pragma once

#include <Chunk.h>

#include <map>
#include <memory>
#include <SFML/Graphics.hpp>
#include <string>

class Tile;

class Board : public sf::Drawable, public sf::Transformable {
public:
    Board(sf::Texture* tilesetGrid);
    Tile accessTile(int x, int y);
    void loadFromFile(const std::string& filename);
    void saveToFile(const std::string& filename);
    void debugPrintChunk(uint64_t i) {
        chunks_.at(i).debugPrintChunk();
    }

private:
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
    void parseFile(const std::string& line, int lineNumber, ParseState& parseState);
    Chunk& getChunk(int x, int y);
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    sf::Vector2u maxSize_;
    std::map<uint64_t, Chunk> chunks_;
    sf::Texture* tilesetGrid_;
};
