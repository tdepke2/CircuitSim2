#pragma once

#include <map>
#include <memory>
#include <SFML/Graphics.hpp>

namespace gui {
    class Gui;
    class Style;
}

namespace gui {

/**
 * Abstract class providing a factory method for various widget styles. Each
 * widget style defines colors, padding, alignment, textures, etc. to use when
 * drawing the widget. The `Theme` allocates styles as they are needed, instead
 * of creating all of them during construction.
 * 
 * For custom widgets that need a style, there are two options to add their
 * styles to the theme. The first is to create a new theme by inheriting from an
 * existing one, then override `loadStyle()` for the new widgets. The second
 * option is to just call `addStyle()` on an existing instance of a theme.
 * 
 * Note that a theme may need to store objects like `sf::Font` and `sf::Texture`
 * for widgets to use. Therefore, the lifetime of the `Theme` must be bound to
 * the `Gui`.
 */
class Theme {
public:
    Theme(const Gui& gui);
    virtual ~Theme() noexcept = default;

    void addStyle(const sf::String& widgetName, std::shared_ptr<Style> style);
    bool removeStyle(const sf::String& widgetName);
    std::shared_ptr<Style> getStyle(const sf::String& widgetName) const;
    template<typename T>
    std::shared_ptr<T> getStyle(const sf::String& widgetName) const {
        return std::dynamic_pointer_cast<T>(getStyle(widgetName));
    }

protected:
    // Creates a string with some ASCII characters and measures the bounds, this
    // gives an estimate of the maximum character size for the given font.
    // Dividing this by the default font size gives a ratio that can be
    // multiplied by the current font size during rendering.
    float computeTextMaxHeightRatio(const sf::Font& font) const;
    virtual std::shared_ptr<Style> loadStyle(const sf::String& widgetName) const = 0;

    const Gui& gui_;
    mutable std::map<sf::String, std::shared_ptr<Style>> styles_;
};

} // namespace gui
