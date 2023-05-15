#pragma once

#include <gui/Widget.h>

#include <memory>
#include <SFML/Graphics.hpp>

namespace gui {
    class Theme;
}

namespace gui {

class ButtonStyle {
public:
    ButtonStyle();

    // sf::Shape interface.
    void setTexture(const sf::Texture* texture, bool resetRect = false);
    void setTextureRect(const sf::IntRect& rect);
    void setFillColor(const sf::Color& color);
    void setOutlineColor(const sf::Color& color);
    void setOutlineThickness(float thickness);
    const sf::Texture* getTexture() const;
    const sf::IntRect& getTextureRect() const;
    const sf::Color& getFillColor() const;
    const sf::Color& getOutlineColor() const;
    float getOutlineThickness() const;

    // sf::Text interface.
    void setFont(const sf::Font& font);
    void setCharacterSize(unsigned int size);
    void setLineSpacing(float spacingFactor);
    void setLetterSpacing(float spacingFactor);
    void setTextStyle(uint32_t style);
    void setTextFillColor(const sf::Color& color);
    const sf::Font* getFont() const;
    unsigned int getCharacterSize() const;
    float getLineSpacing() const;
    float getLetterSpacing() const;
    uint32_t getTextStyle() const;
    const sf::Color& getTextFillColor() const;

    void setFillColorDown(const sf::Color& color);
    void setTextPadding(float padding);
    const sf::Color& getFillColorDown() const;
    float getTextPadding() const;

    std::shared_ptr<ButtonStyle> clone() const;

private:
    sf::RectangleShape rect_;
    sf::Text text_;
    sf::Color colorUp_, colorDown_;
    float textPadding_;

    friend class Button;
};

class Button : public Widget, public sf::Transformable {
public:
    static std::shared_ptr<Button> create(std::shared_ptr<Theme> theme);
    static std::shared_ptr<Button> create(std::shared_ptr<ButtonStyle> style);
    virtual ~Button() = default;

    void setSize(const sf::Vector2f& size);
    void setLabel(const sf::String& label);
    const sf::Vector2f& getSize() const;
    const sf::String& getLabel() const;

    void setStyle(std::shared_ptr<ButtonStyle> style);
    // Getting the style makes a local copy. Changes to this style will therefore not effect the theme.
    std::shared_ptr<ButtonStyle> getStyle();

protected:
    Button(std::shared_ptr<Theme> theme);
    Button(std::shared_ptr<ButtonStyle> style);

private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    std::shared_ptr<ButtonStyle> style_;
    bool styleCopied_;



/*
FIXME stuck here, how do we reference the style as either the local one or global one? also should we pass the style in ctor instead of the theme?
imagine we have a bunch of bold buttons, they all should share a style but not need to be allocated an entire theme.

*/



    sf::Vector2f size_;
    sf::String label_;
};

}
