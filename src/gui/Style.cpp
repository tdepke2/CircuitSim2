#include <gui/Style.h>

namespace gui {

sf::Color Style::blendColors(const sf::Color& src, const sf::Color& dest) {
    return {
        static_cast<uint8_t>((static_cast<int>(src.r) * src.a + static_cast<int>(dest.r) * (255 - src.a)) / 255),
        static_cast<uint8_t>((static_cast<int>(src.g) * src.a + static_cast<int>(dest.g) * (255 - src.a)) / 255),
        static_cast<uint8_t>((static_cast<int>(src.b) * src.a + static_cast<int>(dest.b) * (255 - src.a)) / 255),
        dest.a
    };
}

Style::Style(const Gui& gui) :
    gui_(gui) {
}

}
