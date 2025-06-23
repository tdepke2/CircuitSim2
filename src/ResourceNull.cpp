#include <ResourceNull.h>
#include <TileWidth.h>

#include <spdlog/spdlog.h>
#include <stdexcept>

ResourceNull::ResourceNull() :
    textures_(),
    fonts_(),
    missingTexture_() {

    /*missingTexture_.create(TileWidth::TEXELS * 10, TileWidth::TEXELS * 13);
    for (unsigned int y = 0; y < missingTexture_.getSize().y; ++y) {
        for (unsigned int x = 0; x < missingTexture_.getSize().x; ++x) {
            missingTexture_.setPixel(x, y, {
                static_cast<uint8_t>(static_cast<double>(x) / missingTexture_.getSize().x * 255),
                static_cast<uint8_t>(static_cast<double>(y) / missingTexture_.getSize().y * 255),
                0,
                255
            });
        }
    }*/

    missingTexture_.create(TileWidth::TEXELS, TileWidth::TEXELS);
    for (unsigned int y = 0; y < missingTexture_.getSize().y; ++y) {
        for (unsigned int x = 0; x < missingTexture_.getSize().x; ++x) {
            if ((x < missingTexture_.getSize().x / 2) == (y < missingTexture_.getSize().y / 2)) {
                missingTexture_.setPixel(x, y, {255, 0, 255});
            } else {
                missingTexture_.setPixel(x, y, {0, 0, 0});
            }
        }
    }
}

sf::Texture& ResourceNull::getTexture(const fs::path& filename, uint32_t flags) {
    auto tex = textures_.find(filename);
    if (tex != textures_.end()) {
        return tex->second;
    }
    spdlog::debug("ResourceNull adding missing texture {} (initEmpty {}).", filename, (flags & TextureFlags::initEmpty) > 0);
    sf::Texture& newTex = textures_[filename];
    if ((flags & TextureFlags::initEmpty) == 0) {
        newTex.loadFromImage(missingTexture_);
    }
    return newTex;
}

sf::Font& ResourceNull::getFont(const fs::path& filename) {
    auto font = fonts_.find(filename);
    if (font != fonts_.end()) {
        return font->second;
    }
    spdlog::debug("ResourceNull adding missing font {}.", filename);
    sf::Font& newFont = fonts_[filename];
    return newFont;
}

sf::Shader& ResourceNull::getShader(const fs::path& vertFilename, const fs::path& fragFilename) {
    return getShader(vertFilename, "", fragFilename);
}

sf::Shader& ResourceNull::getShader(const fs::path& /*vertFilename*/, const fs::path& /*geomFilename*/, const fs::path& /*fragFilename*/) {
    throw std::runtime_error("Shaders are not supported in this configuration.");
}
