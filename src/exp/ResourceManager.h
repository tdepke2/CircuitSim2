#pragma once

#include <map>
#include <SFML/Graphics.hpp>

class ResourceManager {
public:
    sf::Texture& getTexture(const std::string& filename, bool initEmpty = false);
    sf::Font& getFont(const std::string& filename);

private:
    std::map<std::string, sf::Texture> textures_;
    std::map<std::string, sf::Font> fonts_;
};
