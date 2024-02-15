#include <ResourceManager.h>

#include <spdlog/spdlog.h>
#include <stdexcept>

ResourceManager::ResourceManager() :
    textures_(),
    fonts_(),
    shaders_() {
}

sf::Texture& ResourceManager::getTexture(const fs::path& filename, bool initEmpty) {
    auto tex = textures_.find(filename);
    if (tex != textures_.end()) {
        return tex->second;
    }
    spdlog::debug("ResourceManager loading texture {} (initEmpty {}).", filename, initEmpty);
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
    spdlog::debug("ResourceManager loading font {}.", filename);
    sf::Font& newFont = fonts_[filename];
    if (!newFont.loadFromFile(filename.string())) {
        throw std::runtime_error("\"" + filename.string() + "\": unable to load font file.");
    }
    return newFont;
}

sf::Shader& ResourceManager::getShader(const fs::path& vertFilename, const fs::path& fragFilename) {
    return getShader(vertFilename, "", fragFilename);
}

sf::Shader& ResourceManager::getShader(const fs::path& vertFilename, const fs::path& geomFilename, const fs::path& fragFilename) {
    fs::path combinedName = vertFilename;
    combinedName.concat("; ");
    combinedName.concat(geomFilename);
    combinedName.concat("; ");
    combinedName.concat(fragFilename);

    auto shader = shaders_.find(combinedName);
    if (shader != shaders_.end()) {
        return shader->second;
    }
    spdlog::debug("ResourceManager loading shader {}.", combinedName);
    if (!sf::Shader::isAvailable()) {
        throw std::runtime_error("Shaders are not supported on this platform.");
    } else if (!geomFilename.empty() && !sf::Shader::isGeometryAvailable()) {
        throw std::runtime_error("Geometry shaders are not supported on this platform.");
    }

    sf::Shader& newShader = shaders_[combinedName];
    if (geomFilename.empty()) {
        if (!newShader.loadFromFile(vertFilename.string(), fragFilename.string())) {
            throw std::runtime_error(
                "\"" + vertFilename.string() + "\" and \"" +
                fragFilename.string() + "\": unable to load shader program."
            );
        }
    } else {
        if (!newShader.loadFromFile(vertFilename.string(), geomFilename.string(), fragFilename.string())) {
            throw std::runtime_error(
                "\"" + vertFilename.string() + "\" and \"" +
                geomFilename.string() + "\" and \"" +
                fragFilename.string() + "\": unable to load shader program."
            );
        }
    }
    return newShader;
}
