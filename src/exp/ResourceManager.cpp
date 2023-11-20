#include <ResourceManager.h>

#include <stdexcept>

ResourceManager::ResourceManager() :
    textures_(),
    fonts_() {
}

sf::Texture& ResourceManager::getTexture(const fs::path& filename, bool initEmpty) {
    auto tex = textures_.find(filename);
    if (tex != textures_.end()) {
        return tex->second;
    }
    sf::Texture& newTex = textures_[filename];
    if (!initEmpty && !newTex.loadFromFile(filename.string())) {
        throw std::runtime_error("\"" + filename.string() + "\": unable to load texture file.");
    }
    return newTex;
}

sf::Font& ResourceManager::getFont(const fs::path& filename) {
    auto font = fonts_.find(filename);
    if (font != fonts_.end()) {
        return font->second;
    }
    sf::Font& newFont = fonts_[filename];
    if (!newFont.loadFromFile(filename.string())) {
        throw std::runtime_error("\"" + filename.string() + "\": unable to load font file.");
    }
    return newFont;
}
