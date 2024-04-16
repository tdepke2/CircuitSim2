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
 * Visual styling for `RadioButton`.
 * 
 * One instance is shared between objects that use the same style, private
 * members in this class operate as flyweights.
 */
class RadioButtonStyle : public Style {
public:
    RadioButtonStyle(const Gui& gui);

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

    void setFillColorChecked(const sf::Color& color);
    void setTextPadding(const sf::Vector3f& padding);
    const sf::Color& getFillColorChecked() const;
    const sf::Vector3f& getTextPadding() const;

    std::shared_ptr<RadioButtonStyle> clone() const;

private:
    sf::CircleShape circle_;
    sf::Text text_;
    sf::Color colorUnchecked_, colorChecked_;
    sf::Vector3f textPadding_;

    friend class RadioButton;
};


/**
 * A button that is mutually exclusive. Setting the button checked will uncheck
 * all other `RadioButton` widgets contained in the parent. A `Group` widget can
 * be used to separate radio buttons from other unrelated ones.
 */
class RadioButton : public Button {
public:
    static std::shared_ptr<RadioButton> create(const Theme& theme, const sf::String& name = "");
    static std::shared_ptr<RadioButton> create(std::shared_ptr<RadioButtonStyle> style, const sf::String& name = "");
    virtual ~RadioButton() noexcept = default;

    void setPressed(bool pressed) = delete;
    bool isPressed() const = delete;
    void setChecked(bool checked);
    bool isChecked() const;
    void uncheckRadioButtons();

    void setStyle(std::shared_ptr<RadioButtonStyle> style);
    // Getting the style makes a local copy. Changes to this style will therefore not effect the theme.
    // To get the global style, get it from the theme.
    std::shared_ptr<RadioButtonStyle> getStyle();

    virtual void handleMousePress(sf::Mouse::Button button, const sf::Vector2f& mouseParent) override;
    virtual void handleMouseRelease(sf::Mouse::Button button, const sf::Vector2f& mouseParent) override;

    virtual void handleMouseEntered() override;
    virtual void handleMouseLeft() override;

protected:
    RadioButton(std::shared_ptr<RadioButtonStyle> style, const sf::String& name);

private:
    virtual void computeResize() const override;
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    std::shared_ptr<RadioButtonStyle> style_;
    bool styleCopied_;
};

} // namespace gui
