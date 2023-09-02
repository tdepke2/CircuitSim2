#pragma once

#include <Chunk.h>

#include <map>
#include <SFML/Graphics.hpp>
#include <string>

class Board : public sf::Drawable, public sf::Transformable {
public:
    Board(sf::Texture* tilesetGrid);
    void loadFromFile(const std::string& filename);
    void saveToFile(const std::string& filename);

private:
    struct ParseState {
        std::string lastField = "";
        std::string filename = "";
        float fileVersion = 1.0;
        unsigned int width = 0;
        unsigned int height = 0;
        bool enableExtraLogicStates = false;
        std::string notesString = "";
        unsigned int x = 0;
        unsigned int y = 0;
    };
    void parseFile(const std::string& line, int lineNumber, ParseState& parseState);
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    sf::Vector2u maxSize_;
    std::map<uint64_t, Chunk> chunks_;
    sf::Texture* tilesetGrid_;
};
