#pragma once

#include <SFML/Graphics.hpp>

namespace gui {
    class Gui;
}

namespace gui {

class Style {
public:
    /**
     * Blends two colors, just like the OpenGL blend mode:
     * `glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);`
     * 
     * For this function, the destination alpha is preserved.
     */
    static sf::Color blendColors(const sf::Color& src, const sf::Color& dest);

    Style(const Gui& gui);
    virtual ~Style() = default;

protected:
    const Gui& gui_;
};

} // namespace gui
