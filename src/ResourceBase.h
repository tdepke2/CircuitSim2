#pragma once

#include <Filesystem.h>

#include <SFML/Graphics.hpp>

class ResourceBase {
public:
    virtual ~ResourceBase() = default;

    virtual sf::Texture& getTexture(const fs::path& filename, bool initEmpty = false) = 0;
    virtual sf::Font& getFont(const fs::path& filename) = 0;
    virtual sf::Shader& getShader(const fs::path& vertFilename, const fs::path& fragFilename) = 0;
    virtual sf::Shader& getShader(const fs::path& vertFilename, const fs::path& geomFilename, const fs::path& fragFilename) = 0;
};
