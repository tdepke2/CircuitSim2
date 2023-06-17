#pragma once

#include <memory>
#include <SFML/Graphics.hpp>

namespace gui {
    class ButtonStyle;
    class Gui;
    class PanelStyle;
    class TextBoxStyle;
}

namespace gui {

class Theme {
public:
    Theme(const Gui& gui);
    virtual ~Theme() noexcept = default;

    virtual std::shared_ptr<ButtonStyle> getButtonStyle() const = 0;
    virtual std::shared_ptr<PanelStyle> getPanelStyle() const = 0;
    virtual std::shared_ptr<TextBoxStyle> getTextBoxStyle() const = 0;

protected:
    // Creates a string with some ASCII characters and measures the bounds, this
    // gives an estimate of the maximum character size for the given font.
    // Dividing this by the default font size gives a ratio that can be
    // multiplied by the current font size during rendering.
    float computeTextMaxHeightRatio(const sf::Font& font) const;

    const Gui& gui_;
    mutable std::shared_ptr<ButtonStyle> buttonStyle_;
    mutable std::shared_ptr<PanelStyle> panelStyle_;
    mutable std::shared_ptr<TextBoxStyle> textBoxStyle_;
};

}
