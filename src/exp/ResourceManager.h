#pragma once

#include <map>
#include <SFML/Graphics.hpp>

class ResourceManager {
public:
    sf::Texture& getTexture(const std::string& filename, bool initEmpty = false);

private:
    std::map<std::string, sf::Texture> textures_;
};
