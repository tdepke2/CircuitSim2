#pragma once

#include <gui/Signal.h>
#include <gui/Widget.h>

#include <memory>
#include <SFML/Graphics.hpp>

namespace gui {
    class Theme;
}

namespace gui {

class TextBoxStyle {
public:
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

    void setCaretSize(const sf::Vector2f& size);
    void setCaretFillColor(const sf::Color& color);
    void setTextPadding(const sf::Vector2f& padding);
    const sf::Vector2f& getCaretSize() const;
    const sf::Color& getCaretFillColor() const;
    const sf::Vector2f& getTextPadding() const;

    std::shared_ptr<TextBoxStyle> clone() const;

private:
    sf::RectangleShape box_, caret_;
    sf::Text text_;
    sf::Vector2f textPadding_;

    friend class TextBox;
};

class TextBox : public Widget {
public:
    static std::shared_ptr<TextBox> create(std::shared_ptr<Theme> theme);
    static std::shared_ptr<TextBox> create(std::shared_ptr<TextBoxStyle> style);
    virtual ~TextBox() noexcept = default;

    void setSize(size_t characterWidth);
    void setText(const sf::String& text);
    const sf::Vector2u& getSize() const;
    const sf::String& getText() const;

    void setStyle(std::shared_ptr<TextBoxStyle> style);
    // Getting the style makes a local copy. Changes to this style will therefore not effect the theme.
    std::shared_ptr<TextBoxStyle> getStyle();

    virtual sf::FloatRect getLocalBounds() const override;
    virtual void handleMousePress(sf::Mouse::Button button, const sf::Vector2f& mouseLocal) override;
    virtual void handleMouseRelease(sf::Mouse::Button button, const sf::Vector2f& mouseLocal) override;
    virtual void handleTextEntered(uint32_t unicode) override;

    Signal<Widget*, sf::Mouse::Button, const sf::Vector2f&> onMousePress;
    Signal<Widget*, sf::Mouse::Button, const sf::Vector2f&> onMouseRelease;
    Signal<Widget*, const sf::Vector2f&> onClick;

protected:
    TextBox(std::shared_ptr<TextBoxStyle> style);

private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    std::shared_ptr<TextBoxStyle> style_;
    bool styleCopied_;

    sf::Vector2u size_;
    sf::Vector2f boxSize_;
    sf::String boxString_;
    size_t caretPosition_;
    sf::Vector2f caretDrawPosition_;
};

}
