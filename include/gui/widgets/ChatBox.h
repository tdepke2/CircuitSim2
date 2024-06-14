#pragma once

#include <gui/Signal.h>
#include <gui/Style.h>
#include <gui/Widget.h>

#include <deque>
#include <functional>
#include <memory>
#include <SFML/Graphics.hpp>
#include <utility>
#include <vector>

namespace gui {
    class Gui;
    class Theme;
}

namespace gui {

/**
 * A single line in the `ChatBox`, containing the text, color, and
 * `sf::Text::Style` information. The `id` is internally used for visible line
 * numbering.
 */
struct ChatBoxLine {
public:
    ChatBoxLine() :
        str(), color(), style(sf::Text::Regular), id(0) {
    }
    ChatBoxLine(const sf::String& str, const sf::Color& color, uint32_t style, unsigned int id = 0) :
        str(str), color(color), style(style), id(id) {
    }

    sf::String str;
    sf::Color color;
    uint32_t style;
    unsigned int id;
};

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

    void setInvertedTextFillColor(const sf::Color& color);
    void setHighlightFillColor(const sf::Color& color);
    void setTextPadding(const sf::Vector3f& padding);
    const sf::Color& getInvertedTextFillColor() const;
    const sf::Color& getHighlightFillColor() const;
    const sf::Vector3f& getTextPadding() const;

    std::shared_ptr<ChatBoxStyle> clone() const;

private:
    sf::RectangleShape rect_;
    sf::Text text_;
    sf::Color textColor_;
    uint32_t textStyle_;
    sf::Color invertedTextColor_, highlightColor_;
    sf::Vector3f textPadding_;

    friend class ChatBox;
};


/**
 * Shows a box with lines of text. Each line can have a color and text style.
 * Supports setting a line limit, scrolling, and lines can be selected and
 * copied. An auto-hide mode is available to make the box invisible, and have
 * lines hide after a delay.
 */
class ChatBox : public Widget {
    using baseClass = Widget;

public:
    static std::shared_ptr<ChatBox> create(const Theme& theme, const sf::String& name = "");
    static std::shared_ptr<ChatBox> create(std::shared_ptr<ChatBoxStyle> style, const sf::String& name = "");
    virtual ~ChatBox() = default;

    void setSizeCharacters(const sf::Vector2<size_t>& sizeCharacters);
    void setMaxLines(size_t maxLines);
    void setAutoHide(bool autoHide);
    const sf::Vector2f& getSize() const;
    const sf::Vector2<size_t>& getSizeCharacters() const;
    size_t getMaxLines() const;
    bool getAutoHide() const;
    void addLines(const sf::String& str);
    void addLines(const sf::String& str, const sf::Color& color);
    void addLines(const sf::String& str, uint32_t style);
    void addLines(const sf::String& str, const sf::Color& color, uint32_t style);
    const ChatBoxLine& getLine(size_t index) const;
    size_t getNumLines() const;
    bool removeLine(size_t index);
    void removeAllLines();

    void setStyle(std::shared_ptr<ChatBoxStyle> style);
    // Getting the style makes a local copy. Changes to this style will therefore not effect the theme.
    // To get the global style, get it from the theme.
    std::shared_ptr<ChatBoxStyle> getStyle();

    virtual sf::FloatRect getLocalBounds() const override;
    virtual bool isMouseIntersecting(const sf::Vector2f& mouseParent) const override;

    virtual bool handleMouseMove(const sf::Vector2f& mouseParent) override;
    virtual bool handleMouseWheelScroll(sf::Mouse::Wheel wheel, float delta, const sf::Vector2f& mouseParent) override;
    virtual void handleMousePress(sf::Mouse::Button button, const sf::Vector2f& mouseParent) override;
    virtual void handleMouseRelease(sf::Mouse::Button button, const sf::Vector2f& mouseParent) override;
    virtual bool handleKeyPressed(const sf::Event::KeyEvent& key) override;
    virtual void handleFocusChange(bool focused) override;

    Signal<Widget*, sf::Mouse::Button, const sf::Vector2f&> onMousePress;
    Signal<Widget*, sf::Mouse::Button, const sf::Vector2f&> onMouseRelease;
    Signal<Widget*, const sf::Vector2f&> onClick;

protected:
    ChatBox(std::shared_ptr<ChatBoxStyle> style, const sf::String& name);

private:
    void updateVisibleLines();
    void updateSelection(size_t pos, bool continueSelection);
    void updateScroll(int delta);
    size_t findClosestLineToMouse(const sf::Vector2f& mouseLocal) const;
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    std::shared_ptr<ChatBoxStyle> style_;
    bool styleCopied_;

    sf::Vector2<size_t> sizeCharacters_;
    size_t maxLines_;
    bool autoHide_;
    sf::Vector2f size_;
    std::deque<ChatBoxLine> lines_;
    mutable std::vector<std::pair<ChatBoxLine, sf::RectangleShape>> visibleLines_;
    size_t verticalScroll_;
    std::pair<size_t, bool> selectionStart_;
    size_t selectionEnd_;
    std::function<void()> hideCallback_;
    size_t hideCounter_;
};

} // namespace gui
