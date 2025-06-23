#pragma once

#include <Filesystem.h>

#include <SFML/Graphics.hpp>

/**
 * Base class for a resource manager (for the service locator pattern).
 */
class ResourceBase {
public:
    enum TextureFlags : uint32_t {
        initEmpty = 1 << 0
    };

    virtual ~ResourceBase() = default;

    virtual sf::Texture& getTexture(const fs::path& filename, uint32_t flags = 0) = 0;
    virtual sf::Font& getFont(const fs::path& filename) = 0;
    virtual sf::Shader& getShader(const fs::path& vertFilename, const fs::path& fragFilename) = 0;
    virtual sf::Shader& getShader(const fs::path& vertFilename, const fs::path& geomFilename, const fs::path& fragFilename) = 0;
};
