#pragma once

#include <gui/Style.h>
#include <gui/widgets/Group.h>

#include <array>
#include <memory>
#include <SFML/Graphics.hpp>

namespace gui {
    class Gui;
    class Label;
    class MultilineTextBox;
    class Slider;
    class Theme;
}

namespace gui {

/**
 * Visual styling for `ColorPicker`.
 * 
 * One instance is shared between objects that use the same style, private
 * members in this class operate as flyweights.
 */
class ColorPickerStyle : public Style {
public:
    ColorPickerStyle(const Gui& gui);
    virtual ~ColorPickerStyle() = default;

    // sf::Shape interface.
    void setTexture(const sf::Texture* texture, bool resetRect = false);
    void setTextureRect(const sf::IntRect& rect);
    //void setFillColor(const sf::Color& color);
    void setOutlineColor(const sf::Color& color);
    void setOutlineThickness(float thickness);
    const sf::Texture* getTexture() const;
    const sf::IntRect& getTextureRect() const;
    //const sf::Color& getFillColor() const;
    const sf::Color& getOutlineColor() const;
    float getOutlineThickness() const;

    void setDotRadius(float radius);
    float getDotRadius() const;

    std::shared_ptr<ColorPickerStyle> clone() const;

private:
    sf::CircleShape dot_;

    friend class ColorPicker;
};


/**
 * FIXME
 */
class ColorPicker : public Group {
    using baseClass = Group;

public:
    static std::shared_ptr<ColorPicker> create(const Theme& theme, const sf::String& name = "");
    //static std::shared_ptr<ColorPicker> create(std::shared_ptr<ColorPickerStyle> style, const sf::String& name = "");
    virtual ~ColorPicker() = default;

    void setSize(const sf::Vector2f& size);
    const sf::Vector2f& getSize() const;

    void setStyle(std::shared_ptr<ColorPickerStyle> style);
    // Getting the style makes a local copy. Changes to this style will therefore not effect the theme.
    // To get the global style, get it from the theme.
    std::shared_ptr<ColorPickerStyle> getStyle();

    virtual sf::FloatRect getLocalBounds() const override;
    virtual bool isMouseIntersecting(const sf::Vector2f& mouseParent) const override;

protected:
    ColorPicker(const Theme& theme, const sf::String& name);

private:
    sf::Vector2f getShadingRectangleSize() const;
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    std::shared_ptr<ColorPickerStyle> style_;
    bool styleCopied_;
    sf::Vector2f size_;
    sf::Texture alphaTexture_;
    sf::VertexArray shadingRectangle_, hueBar_, alphaBar_;
    sf::Color currentColor_;
    std::shared_ptr<Slider> hueSlider_;
    std::shared_ptr<Slider> alphaSlider_;
    std::shared_ptr<Label> rgbaLabel_, hsvaLabel_, rgbaHexLabel_;
    std::array<std::shared_ptr<MultilineTextBox>, 4> rgbaTextBoxes_;
    std::array<std::shared_ptr<MultilineTextBox>, 4> hsvaTextBoxes_;
    std::shared_ptr<MultilineTextBox> rgbaHexTextBox_;
};

} // namespace gui
