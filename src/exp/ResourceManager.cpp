#include <ResourceManager.h>

#include <stdexcept>

sf::Texture& ResourceManager::getTexture(const std::string& filename, bool initEmpty) {
    auto tex = textures_.find(filename);
    if (tex != textures_.end()) {
        return tex->second;
    }
    sf::Texture& newTex = textures_[filename];
    if (!initEmpty && !newTex.loadFromFile(filename)) {
        throw std::runtime_error("\"" + filename + "\": unable to load texture file.");
    }
    return newTex;
}

sf::Font& ResourceManager::getFont(const std::string& filename) {
    auto font = fonts_.find(filename);
    if (font != fonts_.end()) {
        return font->second;
    }
    sf::Font& newFont = fonts_[filename];
    if (!newFont.loadFromFile(filename)) {
        throw std::runtime_error("\"" + filename + "\": unable to load font file.");
    }
    return newFont;
}
