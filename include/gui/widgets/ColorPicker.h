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
    void setBarWidth(float barWidth);
    void setBarSpacing(float barSpacing);
    void setBoxSpacing(const sf::Vector2f& boxSpacing);
    float getDotRadius() const;
    float getBarWidth() const;
    float getBarSpacing() const;
    const sf::Vector2f& getBoxSpacing() const;

    std::shared_ptr<ColorPickerStyle> clone() const;

private:
    sf::CircleShape dot_;
    float barWidth_;
    float barSpacing_;
    sf::Vector2f boxSpacing_;

    friend class ColorPicker;
};


/**
 * Displays a shading area, hue/alpha bar, and RGBA/HSVA text boxes for color
 * selection. Note that color change events are only sent when the RGBA color
 * changes, and conversions between RGBA and HSVA are not one to one.
 */
class ColorPicker : public Group {
    using baseClass = Group;

public:
    static std::shared_ptr<ColorPicker> create(const Theme& theme, const sf::String& name = "");
    //static std::shared_ptr<ColorPicker> create(std::shared_ptr<ColorPickerStyle> style, const sf::String& name = "");
    virtual ~ColorPicker() = default;

    void setSize(const sf::Vector2f& size);
    void setColor(const sf::Color& color);
    const sf::Vector2f& getSize() const;
    const sf::Color& getColor() const;

    void setStyle(std::shared_ptr<ColorPickerStyle> style);
    // Getting the style makes a local copy. Changes to this style will therefore not effect the theme.
    // To get the global style, get it from the theme.
    std::shared_ptr<ColorPickerStyle> getStyle();

    virtual sf::FloatRect getLocalBounds() const override;
    virtual bool isMouseIntersecting(const sf::Vector2f& mouseParent) const override;

    virtual bool handleMouseMove(const sf::Vector2f& mouseParent) override;
    virtual void handleMousePress(sf::Mouse::Button button, const sf::Vector2f& mouseParent) override;
    virtual void handleMouseRelease(sf::Mouse::Button button, const sf::Vector2f& mouseParent) override;

    Signal<Widget*, const sf::Color&> onColorChange;

protected:
    ColorPicker(const Theme& theme, const sf::String& name);

private:
    enum class ColorSource {
        none, rgba, hsva, rgbaHex, inputHsva
    };

    sf::Vector2f getShadingRectangleSize() const;
    void updateShadingRectangle(float saturation, float value);
    void updateCurrentColor(ColorSource source, bool validateSource, const std::array<float, 4>& inputHsva = {0.0f, 0.0f, 0.0f, 1.0f});
    void updateLayout();
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    std::shared_ptr<ColorPickerStyle> style_;
    bool styleCopied_;
    sf::Vector2f size_;
    sf::Texture alphaTexture_;
    sf::VertexArray shadingRectangle_, hueBar_, alphaBar_;
    std::array<float, 4> currentColorHsva_;
    sf::Color roundedColorRgba_;
    std::shared_ptr<Slider> hueSlider_;
    std::shared_ptr<Slider> alphaSlider_;
    std::shared_ptr<Label> rgbaLabel_, hsvaLabel_, rgbaHexLabel_;
    std::array<std::shared_ptr<MultilineTextBox>, 4> rgbaTextBoxes_;
    std::array<std::shared_ptr<MultilineTextBox>, 4> hsvaTextBoxes_;
    std::shared_ptr<MultilineTextBox> rgbaHexTextBox_;
    bool isDragging_;
};

} // namespace gui
