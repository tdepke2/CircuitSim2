#pragma once

#include <gui/Signal.h>
#include <gui/Style.h>
#include <gui/Widget.h>

#include <deque>
#include <memory>
#include <SFML/Graphics.hpp>

namespace gui {
    class Gui;
    class Theme;
}

namespace gui {

/**
 * Visual styling for `ChatBox`.
 * 
 * One instance is shared between objects that use the same style, private
 * members in this class operate as flyweights.
 */
class ChatBoxStyle : public Style {
public:
    ChatBoxStyle(const Gui& gui);
    virtual ~ChatBoxStyle() = default;

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

    void setTextPadding(const sf::Vector3f& padding);
    const sf::Vector3f& getTextPadding() const;

    std::shared_ptr<ChatBoxStyle> clone() const;

private:
    sf::RectangleShape rect_;
    sf::Text text_;
    sf::Vector3f textPadding_;

    friend class ChatBox;
};


/**
 * FIXME
 */
class ChatBox : public Widget {
    using baseClass = Widget;

public:
    static std::shared_ptr<ChatBox> create(const Theme& theme, const sf::String& name = "");
    static std::shared_ptr<ChatBox> create(std::shared_ptr<ChatBoxStyle> style, const sf::String& name = "");
    virtual ~ChatBox() = default;

    void setSizeCharacters(const sf::Vector2<size_t>& sizeCharacters);
    void setMaxLines(size_t maxLines);
    const sf::Vector2f& getSize() const;
    const sf::Vector2<size_t>& getSizeCharacters() const;
    size_t getMaxLines() const;

    void setStyle(std::shared_ptr<ChatBoxStyle> style);
    // Getting the style makes a local copy. Changes to this style will therefore not effect the theme.
    // To get the global style, get it from the theme.
    std::shared_ptr<ChatBoxStyle> getStyle();

    virtual sf::FloatRect getLocalBounds() const override;

protected:
    ChatBox(std::shared_ptr<ChatBoxStyle> style, const sf::String& name);

private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    std::shared_ptr<ChatBoxStyle> style_;
    bool styleCopied_;

    sf::Vector2<size_t> sizeCharacters_;
    size_t maxLines_;
    sf::Vector2f size_;
    std::deque<sf::String> lines_;
    size_t verticalScroll_;
};

} // namespace gui
