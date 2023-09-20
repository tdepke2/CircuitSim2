#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <vector>

class DebugScreen : public sf::Drawable {
public:
    enum class Field {
        frameTime, view, chunk, count
    };

    DebugScreen(const sf::Font& font, unsigned int characterSize);
    void setVisible(bool visible);
    bool isVisible() const;
    sf::Text& getField(Field field);
    sf::Text& getField(const std::string& customName);

private:
    sf::Text addField(bool isCustom);
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    bool visible_;
    const sf::Font& font_;
    unsigned int characterSize_;
    sf::Vector2f nextFieldPos_;
    std::vector<sf::Text> fields_;
    std::unordered_map<std::string, sf::Text> customFields_;
};
