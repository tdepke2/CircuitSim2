#pragma once

#include "SFML/System/Vector2.hpp"
#include "SFML/Window/Mouse.hpp"
#include <gui/Signal.h>
#include <gui/Widget.h>

#include <memory>
#include <SFML/Graphics.hpp>

namespace gui {
    class Theme;
}

namespace gui {

class ButtonStyle {
public:
    ButtonStyle() = default;

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
    void setTextPadding(const sf::Vector2f& padding);
    const sf::Color& getFillColorDown() const;
    const sf::Vector2f& getTextPadding() const;

    std::shared_ptr<ButtonStyle> clone() const;

private:
    sf::RectangleShape rect_;
    sf::Text text_;
    sf::Color colorUp_, colorDown_;
    sf::Vector2f textPadding_;

    friend class Button;
};

class Button : public Widget {
public:
    static std::shared_ptr<Button> create(std::shared_ptr<Theme> theme);
    static std::shared_ptr<Button> create(std::shared_ptr<ButtonStyle> style);
    virtual ~Button() noexcept = default;

    void setSize(const sf::Vector2f& size);
    void setLabel(const sf::String& label);
    void setAutoResize(bool autoResize);
    const sf::Vector2f& getSize() const;
    const sf::String& getLabel() const;
    bool getAutoResize() const;

    void setStyle(std::shared_ptr<ButtonStyle> style);
    // Getting the style makes a local copy. Changes to this style will therefore not effect the theme.
    std::shared_ptr<ButtonStyle> getStyle();

    virtual sf::FloatRect getLocalBounds() const override;
    virtual void handleMousePress(sf::Mouse::Button button, const sf::Vector2f& mouseLocal) override;
    virtual void handleMouseRelease(sf::Mouse::Button button, const sf::Vector2f& mouseLocal) override;

    Signal<Widget*, sf::Mouse::Button, const sf::Vector2f&> onMousePress;
    Signal<Widget*, sf::Mouse::Button, const sf::Vector2f&> onMouseRelease;
    Signal<Widget*, const sf::Vector2f&> onClick;

protected:
    Button(std::shared_ptr<ButtonStyle> style);

private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    std::shared_ptr<ButtonStyle> style_;
    bool styleCopied_;



/*
FIXME stuck here, how do we reference the style as either the local one or global one? also should we pass the style in ctor instead of the theme?
imagine we have a bunch of bold buttons, they all should share a style but not need to be allocated an entire theme.

should be resolved now after changes
*/



    sf::Vector2f size_;
    sf::String label_;
    bool autoResize_;
    bool isPressed_;
};

}
