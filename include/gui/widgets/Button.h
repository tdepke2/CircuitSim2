#pragma once

#include <gui/Signal.h>
#include <gui/Style.h>
#include <gui/Widget.h>

#include <memory>
#include <SFML/Graphics.hpp>

namespace gui {
    class Gui;
    class Theme;
}

namespace gui {

/**
 * Visual styling for `Button`.
 * 
 * One instance is shared between objects that use the same style, private
 * members in this class operate as flyweights.
 */
class ButtonStyle : public Style {
public:
    ButtonStyle(const Gui& gui);
    virtual ~ButtonStyle() = default;

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

    void setFillColorHover(const sf::Color& color);
    void setFillColorDown(const sf::Color& color);
    void setTextPadding(const sf::Vector3f& padding);
    const sf::Color& getFillColorHover() const;
    const sf::Color& getFillColorDown() const;
    const sf::Vector3f& getTextPadding() const;

    std::shared_ptr<ButtonStyle> clone() const;

private:
    sf::RectangleShape rect_;
    sf::Text text_;
    sf::Color colorUp_, colorHover_, colorDown_;
    sf::Vector3f textPadding_;

    friend class Button;
};


/**
 * A simple button with a text label.
 */
class Button : public Widget {
public:
    static std::shared_ptr<Button> create(const Theme& theme, const sf::String& name = "");
    static std::shared_ptr<Button> create(std::shared_ptr<ButtonStyle> style, const sf::String& name = "");
    virtual ~Button() = default;

    // Setting the size turns off auto-resize.
    void setSize(const sf::Vector2f& size);
    void setLabel(const sf::String& label);
    void setAutoResize(bool autoResize);
    void setPressed(bool pressed);
    const sf::Vector2f& getSize() const;
    const sf::String& getLabel() const;
    bool getAutoResize() const;
    bool isPressed() const;

    void setStyle(std::shared_ptr<ButtonStyle> style);
    // Getting the style makes a local copy. Changes to this style will therefore not effect the theme.
    // To get the global style, get it from the theme.
    std::shared_ptr<ButtonStyle> getStyle();

    virtual sf::FloatRect getLocalBounds() const override;
    virtual void handleMousePress(sf::Mouse::Button button, const sf::Vector2f& mouseParent) override;
    virtual void handleMouseRelease(sf::Mouse::Button button, const sf::Vector2f& mouseParent) override;

    virtual void handleMouseEntered() override;
    virtual void handleMouseLeft() override;

    Signal<Widget*, sf::Mouse::Button, const sf::Vector2f&> onMousePress;
    Signal<Widget*, sf::Mouse::Button, const sf::Vector2f&> onMouseRelease;
    Signal<Widget*, const sf::Vector2f&> onClick;

protected:
    Button(std::shared_ptr<ButtonStyle> style, const sf::String& name);

private:
    virtual void computeResize() const;
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    std::shared_ptr<ButtonStyle> style_;
    bool styleCopied_;

    sf::String label_;
    bool autoResize_;
    bool isPressed_;

protected:
    mutable sf::Vector2f size_;
};

} // namespace gui
