#pragma once

#include <Filesystem.h>

#include <map>
#include <SFML/Graphics.hpp>

class ResourceManager {
public:
    ResourceManager();
    ~ResourceManager() = default;
    ResourceManager(const ResourceManager& rhs) = delete;
    ResourceManager& operator=(const ResourceManager& rhs) = delete;

    sf::Texture& getTexture(const fs::path& filename, bool initEmpty = false);
    sf::Font& getFont(const fs::path& filename);

private:
    std::map<fs::path, sf::Texture> textures_;
    std::map<fs::path, sf::Font> fonts_;
};
