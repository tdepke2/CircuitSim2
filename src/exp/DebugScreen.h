#pragma once

#include <array>
#include <memory>
#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>
#include <vector>

class DebugScreen : public sf::Drawable {
public:
    enum class Mode {
        def = 0,
        textures,
        count
    };

    enum class Field {
        frameTime = 0,
        view,
        zoom,
        chunk,
        count
    };

    static void init(const sf::Font& font, unsigned int characterSize, const sf::Vector2u& windowSize);
    static DebugScreen* instance();
    DebugScreen(const DebugScreen& rhs) = delete;
    DebugScreen(DebugScreen&& rhs) = delete;
    DebugScreen& operator=(const DebugScreen& rhs) = delete;
    DebugScreen& operator=(DebugScreen&& rhs) = delete;

    void processEvent(const sf::Event& event);
    Mode getMode() const;
    std::string getModeString() const;
    void setVisible(bool visible);
    bool isVisible() const;
    sf::Text& getField(Field field);
    sf::Text& getField(const std::string& customName);
    void registerTexture(const std::string& name, const sf::Texture* texture);

private:
    static std::unique_ptr<DebugScreen> instance_;

    struct NamedTexture;

    DebugScreen(const sf::Font& font, unsigned int characterSize, const sf::Vector2u& windowSize);
    sf::Text addField(bool isCustom);
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    Mode mode_;
    std::array<int, static_cast<size_t>(Mode::count)> modeStates_;
    bool visible_;
    const sf::Font& font_;
    unsigned int characterSize_;
    sf::Vector2u windowSize_;
    sf::Vector2f nextFieldPos_;
    std::vector<sf::Text> fields_;
    std::unordered_map<std::string, sf::Text> customFields_;
    std::vector<NamedTexture> textures_;
};
