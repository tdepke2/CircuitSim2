#pragma once

#include <gui/Style.h>
#include <gui/widgets/Button.h>

#include <memory>
#include <SFML/Graphics.hpp>

namespace gui {
    class Gui;
    class Theme;
}

namespace gui {

/**
 * Visual styling for `CheckBox`.
 * 
 * One instance is shared between objects that use the same style, private
 * members in this class operate as flyweights.
 */
class CheckBoxStyle : public Style {
public:
    CheckBoxStyle(const Gui& gui);
    virtual ~CheckBoxStyle() = default;

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
    void setFillColorChecked(const sf::Color& color);
    void setTextPadding(const sf::Vector3f& padding);
    const sf::Color& getFillColorHover() const;
    const sf::Color& getFillColorChecked() const;
    const sf::Vector3f& getTextPadding() const;

    std::shared_ptr<CheckBoxStyle> clone() const;

private:
    sf::RectangleShape rect_;
    sf::Text text_;
    sf::Color colorUnchecked_, colorHover_, colorChecked_;
    sf::Vector3f textPadding_;

    friend class CheckBox;
    friend class RadioButton;
};


/**
 * A toggleable check box with a text label.
 */
class CheckBox : public Button {
    using baseClass = Button;

public:
    static std::shared_ptr<CheckBox> create(const Theme& theme, const sf::String& name = "");
    static std::shared_ptr<CheckBox> create(std::shared_ptr<CheckBoxStyle> style, const sf::String& name = "");
    virtual ~CheckBox() = default;

    void setPressed(bool pressed) = delete;
    bool isPressed() const = delete;
    void setChecked(bool checked);
    bool isChecked() const;

    void setStyle(std::shared_ptr<CheckBoxStyle> style);
    // Getting the style makes a local copy. Changes to this style will therefore not effect the theme.
    // To get the global style, get it from the theme.
    std::shared_ptr<CheckBoxStyle> getStyle();

    virtual void handleMouseRelease(sf::Mouse::Button button, const sf::Vector2f& mouseParent) override;

protected:
    CheckBox(std::shared_ptr<CheckBoxStyle> style, const sf::String& name);

private:
    virtual void computeResize() const override;
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    std::shared_ptr<CheckBoxStyle> style_;
    bool styleCopied_;
    bool isChecked_;
};

} // namespace gui
