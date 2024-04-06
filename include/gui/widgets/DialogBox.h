#pragma once

#include <gui/widgets/Group.h>

#include <memory>
#include <SFML/Graphics.hpp>

namespace gui {
    class Button;
    class Gui;
    class Label;
    class TextBox;
    class Theme;
}

namespace gui {

/**
 * Visual styling for `DialogBox`.
 * 
 * One instance is shared between objects that use the same style, private
 * members in this class operate as flyweights.
 */
class DialogBoxStyle {
public:
    DialogBoxStyle(const Gui& gui);

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

    std::shared_ptr<DialogBoxStyle> clone() const;

protected:
    const Gui& gui_;

private:
    sf::RectangleShape rect_;

    friend class DialogBox;
};


/**
 * FIXME
 */
class DialogBox : public Group {
public:
    static std::shared_ptr<DialogBox> create(const Theme& theme, const sf::String& name = "");
    static std::shared_ptr<DialogBox> create(std::shared_ptr<DialogBoxStyle> style, const sf::String& name = "");
    virtual ~DialogBox() noexcept = default;

    void setSize(const sf::Vector2f& size);
    void setTitle(std::shared_ptr<Label> title);
    void setSubmitButton(std::shared_ptr<Button> button);
    void setCancelButton(std::shared_ptr<Button> button);
    const sf::Vector2f& getSize() const;
    std::shared_ptr<Label> getTitle() const;
    std::shared_ptr<Button> getSubmitButton() const;
    std::shared_ptr<Button> getCancelButton() const;

    virtual void setVisible(bool visible) override;

    void setStyle(std::shared_ptr<DialogBoxStyle> style);
    // Getting the style makes a local copy. Changes to this style will therefore not effect the theme.
    // To get the global style, get it from the theme.
    std::shared_ptr<DialogBoxStyle> getStyle();

    virtual sf::FloatRect getLocalBounds() const override;
    virtual bool isMouseIntersecting(const sf::Vector2f& mouseParent) const override;
    virtual bool handleKeyPressed(const sf::Event::KeyEvent& key) override;

protected:
    DialogBox(std::shared_ptr<DialogBoxStyle> style, const sf::String& name);

private:
    void focusNextTextBox();
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    std::shared_ptr<DialogBoxStyle> style_;
    bool styleCopied_;
    sf::Vector2f size_;
    std::shared_ptr<Label> title_;
    std::shared_ptr<Button> submitButton_;
    std::shared_ptr<Button> cancelButton_;
};

} // namespace gui
