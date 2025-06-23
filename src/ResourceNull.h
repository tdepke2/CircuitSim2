#pragma once

#include <Filesystem.h>
#include <ResourceBase.h>

#include <map>
#include <SFML/Graphics.hpp>

/**
 * Provides null access to SFML resources (allows unit testing to work without
 * file system access).
 */
class ResourceNull : public ResourceBase {
public:
    ResourceNull();
    virtual ~ResourceNull() = default;
    ResourceNull(const ResourceNull& rhs) = delete;
    ResourceNull& operator=(const ResourceNull& rhs) = delete;

    virtual sf::Texture& getTexture(const fs::path& filename, uint32_t flags = 0) override;
    virtual sf::Font& getFont(const fs::path& filename) override;
    virtual sf::Shader& getShader(const fs::path& vertFilename, const fs::path& fragFilename) override;
    virtual sf::Shader& getShader(const fs::path& vertFilename, const fs::path& geomFilename, const fs::path& fragFilename) override;

private:
    std::map<fs::path, sf::Texture> textures_;
    std::map<fs::path, sf::Font> fonts_;
    sf::Image missingTexture_;
};
