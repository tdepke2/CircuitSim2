#pragma once

#include <gui/Style.h>
#include <gui/widgets/Group.h>

#include <memory>
#include <SFML/Graphics.hpp>

namespace gui {
    class Gui;
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

    std::shared_ptr<ColorPickerStyle> clone() const;

private:


    friend class ColorPicker;
};


/**
 * FIXME
 */
class ColorPicker : public Group {
    using baseClass = Group;

public:
    static std::shared_ptr<ColorPicker> create(const Theme& theme, const sf::String& name = "");
    static std::shared_ptr<ColorPicker> create(std::shared_ptr<ColorPickerStyle> style, const sf::String& name = "");
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
    ColorPicker(std::shared_ptr<ColorPickerStyle> style, const sf::String& name);

private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    std::shared_ptr<ColorPickerStyle> style_;
    bool styleCopied_;
    sf::Vector2f size_;
    sf::VertexArray saturationArea_, hueBar_;
};

} // namespace gui
