#pragma once

#include <gui/Style.h>
#include <gui/widgets/Group.h>

#include <memory>
#include <SFML/Graphics.hpp>
#include <utility>
#include <vector>

namespace gui {
    class Button;
    class Gui;
    class Label;
    class Theme;
}

namespace gui {

/**
 * Visual styling for `DialogBox`.
 * 
 * One instance is shared between objects that use the same style, private
 * members in this class operate as flyweights.
 */
class DialogBoxStyle : public Style {
public:
    DialogBoxStyle(const Gui& gui);
    virtual ~DialogBoxStyle() = default;

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

    void setTitleBarTexture(const sf::Texture* texture, bool resetRect = false);
    void setTitleBarTextureRect(const sf::IntRect& rect);
    void setTitleBarFillColor(const sf::Color& color);
    void setTitleBarOutlineColor(const sf::Color& color);
    void setTitleBarOutlineThickness(float thickness);
    const sf::Texture* getTitleBarTexture() const;
    const sf::IntRect& getTitleBarTextureRect() const;
    const sf::Color& getTitleBarFillColor() const;
    const sf::Color& getTitleBarOutlineColor() const;
    float getTitleBarOutlineThickness() const;

    void setTitleBarHeight(float height);
    void setTitlePadding(const sf::Vector2f& titlePadding);
    void setButtonPadding(const sf::Vector2f& buttonPadding);
    float getTitleBarHeight() const;
    const sf::Vector2f& getTitlePadding() const;
    const sf::Vector2f& getButtonPadding() const;

    std::shared_ptr<DialogBoxStyle> clone() const;

private:
    sf::RectangleShape rect_;
    sf::RectangleShape titleBar_;
    float titleBarHeight_;
    sf::Vector2f titlePadding_;
    sf::Vector2f buttonPadding_;

    friend class DialogBox;
};


/**
 * A draggable window with a title bar and some option buttons. The dialog box
 * responds to the Enter and Escape keys to trigger the submit/cancel buttons
 * respectively. Pressing Tab will cycle through all `TextBox` widgets in the
 * dialog.
 */
class DialogBox : public Group {
    using baseClass = Group;

public:
    enum class Alignment {
        left, center, right
    };

    static std::shared_ptr<DialogBox> create(const Theme& theme, const sf::String& name = "");
    static std::shared_ptr<DialogBox> create(std::shared_ptr<DialogBoxStyle> style, const sf::String& name = "");
    virtual ~DialogBox() = default;

    void setSize(const sf::Vector2f& size);
    void setDraggable(bool draggable);
    void setTitle(std::shared_ptr<Label> title);
    void setSubmitButton(size_t index, std::shared_ptr<Button> button);
    void setCancelButton(size_t index, std::shared_ptr<Button> button);
    void setOptionButton(size_t index, std::shared_ptr<Button> button);
    void setTitleAlignment(Alignment titleAlignment);
    void setButtonAlignment(Alignment buttonAlignment);
    const sf::Vector2f& getSize() const;
    bool isDraggable() const;
    std::shared_ptr<Label> getTitle() const;
    std::shared_ptr<Button> getSubmitButton() const;
    std::shared_ptr<Button> getCancelButton() const;
    std::shared_ptr<Button> getOptionButton(size_t index) const;
    Alignment getTitleAlignment() const;
    Alignment getButtonAlignment() const;

    virtual void setVisible(bool visible) override;

    void setStyle(std::shared_ptr<DialogBoxStyle> style);
    // Getting the style makes a local copy. Changes to this style will therefore not effect the theme.
    // To get the global style, get it from the theme.
    std::shared_ptr<DialogBoxStyle> getStyle();

    virtual sf::FloatRect getLocalBounds() const override;
    virtual bool isMouseIntersecting(const sf::Vector2f& mouseParent) const override;

    virtual bool handleMouseMove(const sf::Vector2f& mouseParent) override;
    virtual void handleMousePress(sf::Mouse::Button button, const sf::Vector2f& mouseParent) override;
    virtual void handleMouseRelease(sf::Mouse::Button button, const sf::Vector2f& mouseParent) override;
    virtual bool handleKeyPressed(const sf::Event::KeyEvent& key) override;

protected:
    DialogBox(std::shared_ptr<DialogBoxStyle> style, const sf::String& name);

private:
    void updateTitle();
    void updateButtons();
    void focusNextTextBox();
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    std::shared_ptr<DialogBoxStyle> style_;
    bool styleCopied_;
    sf::Vector2f size_;
    bool draggable_;
    std::pair<sf::Vector2f, bool> dragPoint_;
    sf::Vector2f initialPosition_;
    std::shared_ptr<Label> title_;
    std::vector<std::shared_ptr<Button>> optionButtons_;
    size_t submitIndex_, cancelIndex_;
    Alignment titleAlignment_, buttonAlignment_;
};

} // namespace gui
