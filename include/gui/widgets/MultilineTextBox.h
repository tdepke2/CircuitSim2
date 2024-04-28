#pragma once

#include <gui/Signal.h>
#include <gui/Widget.h>
#include <gui/widgets/TextBox.h>

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
 * Visual styling for `MultilineTextBox`.
 * 
 * One instance is shared between objects that use the same style, private
 * members in this class operate as flyweights.
 */
class MultilineTextBoxStyle : public TextBoxStyle {
public:
    MultilineTextBoxStyle(const Gui& gui);
    virtual ~MultilineTextBoxStyle() = default;

    void setHighlightFillColor(const sf::Color& color);
    const sf::Color& getHighlightFillColor() const;

    std::shared_ptr<MultilineTextBoxStyle> clone() const;

private:
    sf::Color highlightColor_;

    friend class MultilineTextBox;
};


/**
 * An editable text box for large areas. Supports selections with mouse and
 * arrow keys, copy/paste, and scrolling. A character limit can be set and
 * editing disabled if needed.
 */
class MultilineTextBox : public Widget {
    using baseClass = Widget;

public:
    enum class TabPolicy {
        expandTab, ignoreTab
    };

    static std::shared_ptr<MultilineTextBox> create(const Theme& theme, const sf::String& name = "");
    static std::shared_ptr<MultilineTextBox> create(std::shared_ptr<MultilineTextBoxStyle> style, const sf::String& name = "");
    virtual ~MultilineTextBox() = default;

    void setSizeCharacters(const sf::Vector2<size_t>& sizeCharacters);
    void setSizeWithinBounds(const sf::Vector2f& size);
    void setMaxCharacters(size_t maxCharacters);
    void setMaxLines(size_t maxLines);
    void setReadOnly(bool readOnly);
    void setTabPolicy(TabPolicy tabPolicy);
    void setRegexPattern(const sf::String& regexPattern = ".*");
    void setText(const sf::String& text);
    void setDefaultText(const sf::String& text);
    const sf::Vector2f& getSize() const;
    const sf::Vector2<size_t>& getSizeCharacters() const;
    size_t getMaxCharacters() const;
    size_t getMaxLines() const;
    bool getReadOnly() const;
    TabPolicy getTabPolicy() const;
    sf::String getRegexPattern() const;
    sf::String getText() const;
    sf::String getDefaultText() const;
    void selectAll();
    void deselectAll();

    void setStyle(std::shared_ptr<MultilineTextBoxStyle> style);
    // Getting the style makes a local copy. Changes to this style will therefore not effect the theme.
    // To get the global style, get it from the theme.
    std::shared_ptr<MultilineTextBoxStyle> getStyle();

    virtual sf::FloatRect getLocalBounds() const override;
    virtual bool handleMouseMove(const sf::Vector2f& mouseParent) override;
    virtual bool handleMouseWheelScroll(sf::Mouse::Wheel wheel, float delta, const sf::Vector2f& mouseParent) override;
    virtual void handleMousePress(sf::Mouse::Button button, const sf::Vector2f& mouseParent) override;
    virtual void handleMouseRelease(sf::Mouse::Button button, const sf::Vector2f& mouseParent) override;
    virtual bool handleTextEntered(uint32_t unicode) override;
    virtual bool handleKeyPressed(const sf::Event::KeyEvent& key) override;
    virtual void handleFocusChange(bool focused) override;

    Signal<Widget*, sf::Mouse::Button, const sf::Vector2f&> onMousePress;
    Signal<Widget*, sf::Mouse::Button, const sf::Vector2f&> onMouseRelease;
    Signal<Widget*, const sf::Vector2f&> onClick;
    Signal<Widget*, size_t> onTextChange;

protected:
    MultilineTextBox(std::shared_ptr<MultilineTextBoxStyle> style, const sf::String& name);

private:
    bool insertCharacter(uint32_t unicode, bool suppressSignals = false);
    // Internal. Should always be used to set the caret position as this updates the draw position as well.
    void updateCaretPosition(const sf::Vector2<size_t>& caretPosition, bool continueSelection);
    void updateCaretPosition(size_t caretOffset, bool continueSelection);
    void updateScroll(bool vertical, int delta, bool continueSelection);
    sf::Vector2<size_t> findCaretPosition(size_t caretOffset) const;
    size_t findCaretOffset(const sf::Vector2<size_t>& caretPosition) const;
    size_t findClosestOffsetToMouse(const sf::Vector2f& mouseLocal) const;
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    std::shared_ptr<MultilineTextBoxStyle> style_;
    bool styleCopied_;

    sf::Vector2<size_t> sizeCharacters_;
    size_t maxCharacters_, maxLines_;
    bool readOnly_;
    TabPolicy tabPolicy_;
    sf::String regexPattern_;
    sf::Vector2f size_;
    std::vector<sf::String> boxStrings_, defaultStrings_;
    sf::String visibleString_;
    sf::Vector2<size_t> scroll_;
    sf::Vector2<size_t> caretPosition_;
    sf::Vector2f caretDrawPosition_;
    mutable std::vector<sf::RectangleShape> selectionLines_;
    std::pair<sf::Vector2<size_t>, bool> selectionStart_;
    sf::Vector2<size_t> selectionEnd_;
};

} // namespace gui
