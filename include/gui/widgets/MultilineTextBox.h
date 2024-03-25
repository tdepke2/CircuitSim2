#pragma once

#include <gui/Signal.h>
#include <gui/Widget.h>

#include <memory>
#include <SFML/Graphics.hpp>
#include <utility>
#include <vector>

namespace gui {
    class Gui;
    class TextBoxStyle;
    class Theme;
}

namespace gui {

class MultilineTextBox : public Widget {
public:
    static std::shared_ptr<MultilineTextBox> create(const Theme& theme);
    static std::shared_ptr<MultilineTextBox> create(std::shared_ptr<TextBoxStyle> style);
    virtual ~MultilineTextBox() noexcept = default;

    void setSizeCharacters(const sf::Vector2<size_t>& sizeCharacters);
    void setMaxCharacters(size_t maxCharacters);
    void setReadOnly(bool readOnly);
    void setText(const sf::String& text);
    void setDefaultText(const sf::String& text);
    const sf::Vector2f& getSize() const;
    const sf::Vector2<size_t>& getSizeCharacters() const;
    size_t getMaxCharacters() const;
    bool getReadOnly() const;
    sf::String getText() const;
    sf::String getDefaultText() const;

    void setStyle(std::shared_ptr<TextBoxStyle> style);
    // Getting the style makes a local copy. Changes to this style will therefore not effect the theme.
    // To get the global style, get it from the theme.
    std::shared_ptr<TextBoxStyle> getStyle();

    virtual sf::FloatRect getLocalBounds() const override;
    virtual void handleMouseMove(const sf::Vector2f& mouseParent) override;
    virtual void handleMousePress(sf::Mouse::Button button, const sf::Vector2f& mouseParent) override;
    virtual void handleMouseRelease(sf::Mouse::Button button, const sf::Vector2f& mouseParent) override;
    virtual void handleTextEntered(uint32_t unicode) override;
    virtual void handleKeyPressed(const sf::Event::KeyEvent& key) override;

    Signal<Widget*, sf::Mouse::Button, const sf::Vector2f&> onMousePress;
    Signal<Widget*, sf::Mouse::Button, const sf::Vector2f&> onMouseRelease;
    Signal<Widget*, const sf::Vector2f&> onClick;

protected:
    MultilineTextBox(std::shared_ptr<TextBoxStyle> style);

private:
    void insertCharacter(uint32_t unicode);
    // Internal. Should always be used to set the caret position as this updates the draw position as well.
    void updateCaretPosition(const sf::Vector2<size_t>& caretPosition, bool continueSelection);
    void updateCaretPosition(size_t caretOffset, bool continueSelection);
    sf::Vector2<size_t> findCaretPosition(size_t caretOffset) const;
    size_t findCaretOffset(const sf::Vector2<size_t>& caretPosition) const;
    size_t findClosestOffsetToMouse(const sf::Vector2f& mouseLocal) const;
    size_t findClosestOffsetToMouse2(const sf::Vector2f& mouseLocal) const;
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    std::shared_ptr<TextBoxStyle> style_;
    bool styleCopied_;

    sf::Vector2<size_t> sizeCharacters_;
    size_t maxCharacters_;
    bool readOnly_;
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
