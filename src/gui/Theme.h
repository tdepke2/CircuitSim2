#pragma once

#include <memory>
#include <SFML/Graphics.hpp>

namespace gui {
    class ButtonStyle;
    class Gui;
    class LabelStyle;
    class MenuBarStyle;
    class PanelStyle;
    class TextBoxStyle;
}

namespace gui {

/**
 * Abstract class providing factory methods for various widget styles. Each
 * widget style defines colors, padding, alignment, textures, etc. to use when
 * drawing the widget. The `Theme` should allocate styles as they are needed,
 * instead of during construction.
 * 
 * Note that a theme may need to store objects like `sf::Font` and `sf::Texture`
 * for widgets to use. Therefore, the lifetime of the `Theme` must be bound to
 * the `Gui`.
 */
class Theme {
public:
    Theme(const Gui& gui);
    virtual ~Theme() noexcept = default;

    virtual std::shared_ptr<ButtonStyle> getButtonStyle() const = 0;
    virtual std::shared_ptr<LabelStyle> getLabelStyle() const = 0;
    virtual std::shared_ptr<PanelStyle> getPanelStyle() const = 0;
    virtual std::shared_ptr<TextBoxStyle> getTextBoxStyle() const = 0;
    virtual std::shared_ptr<MenuBarStyle> getMenuBarStyle() const = 0;

protected:
    // Creates a string with some ASCII characters and measures the bounds, this
    // gives an estimate of the maximum character size for the given font.
    // Dividing this by the default font size gives a ratio that can be
    // multiplied by the current font size during rendering.
    float computeTextMaxHeightRatio(const sf::Font& font) const;

    const Gui& gui_;
    mutable std::shared_ptr<ButtonStyle> buttonStyle_;
    mutable std::shared_ptr<LabelStyle> labelStyle_;
    mutable std::shared_ptr<PanelStyle> panelStyle_;
    mutable std::shared_ptr<TextBoxStyle> textBoxStyle_;
    mutable std::shared_ptr<MenuBarStyle> menuBarStyle_;
};

} // namespace gui
