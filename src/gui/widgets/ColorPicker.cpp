#include <gui/Gui.h>
#include <gui/Theme.h>
#include <gui/widgets/ColorPicker.h>

#include <algorithm>
#include <array>

namespace {

struct HsvColor {
    HsvColor() :
        h(0), s(0), v(0), a(255) {
    }
    HsvColor(uint8_t hue, uint8_t saturation, uint8_t value, uint8_t alpha = 255) :
        h(hue), s(saturation), v(value), a(alpha) {
    }

    uint8_t h;
    uint8_t s;
    uint8_t v;
    uint8_t a;
};

// Based on information found on the following page:
// https://en.wikipedia.org/wiki/HSL_and_HSV
HsvColor convertRgbToHsv(const sf::Color& color) {
    uint8_t value = std::max(std::max(color.r, color.g), color.b);
    uint8_t chroma = value - std::min(std::min(color.r, color.g), color.b);
    float hue;
    constexpr float hue60Degrees = 255.0f / 6.0f;
    if (chroma == 0) {
        hue = 0.0f;
    } else if (value == color.r) {
        hue = hue60Degrees * std::fmod(static_cast<float>(color.g - color.b) / chroma, 6.0f);
    } else if (value == color.g) {
        hue = hue60Degrees * (static_cast<float>(color.b - color.r) / chroma + 2.0f);
    } else {    // value == color.b
        hue = hue60Degrees * (static_cast<float>(color.r - color.g) / chroma + 4.0f);
    }
    uint8_t saturation = (value == 0 ? 0 : chroma / value);

    return {
        static_cast<uint8_t>(hue), saturation, value, color.a
    };
}

}

namespace gui {

ColorPickerStyle::ColorPickerStyle(const Gui& gui) :
    Style(gui) {
}

std::shared_ptr<ColorPickerStyle> ColorPickerStyle::clone() const {
    return std::make_shared<ColorPickerStyle>(*this);
}



std::shared_ptr<ColorPicker> ColorPicker::create(const Theme& theme, const sf::String& name) {
    return std::shared_ptr<ColorPicker>(new ColorPicker(theme.getStyle<ColorPickerStyle>("ColorPicker"), name));
}
std::shared_ptr<ColorPicker> ColorPicker::create(std::shared_ptr<ColorPickerStyle> style, const sf::String& name) {
    return std::shared_ptr<ColorPicker>(new ColorPicker(style, name));
}

void ColorPicker::setSize(const sf::Vector2f& size) {
    size_ = size;

    const float squareSize = size.y;
    saturationArea_[0] = {{0.0f, 0.0f}, sf::Color::White};    // FIXME: this isn't right, need to use the HSV color space, will need to approximate with large grid of triangles?
    saturationArea_[1] = {{squareSize, 0.0f}, sf::Color::Red};
    saturationArea_[2] = {{0.0f, squareSize}, sf::Color::Black};
    saturationArea_[3] = {{squareSize, squareSize}, sf::Color::Black};

    const float barX1 = size.x + 5.0f;
    const float barX2 = size.x + 25.0f;
    hueBar_[ 0] = {{barX1, size.y * (0.0f / 6.0f)}, {255,   0,   0}};
    hueBar_[ 1] = {{barX2, size.y * (0.0f / 6.0f)}, {255,   0,   0}};
    hueBar_[ 2] = {{barX1, size.y * (1.0f / 6.0f)}, {255, 255,   0}};
    hueBar_[ 3] = {{barX2, size.y * (1.0f / 6.0f)}, {255, 255,   0}};
    hueBar_[ 4] = {{barX1, size.y * (2.0f / 6.0f)}, {  0, 255,   0}};
    hueBar_[ 5] = {{barX2, size.y * (2.0f / 6.0f)}, {  0, 255,   0}};
    hueBar_[ 6] = {{barX1, size.y * (3.0f / 6.0f)}, {  0, 255, 255}};
    hueBar_[ 7] = {{barX2, size.y * (3.0f / 6.0f)}, {  0, 255, 255}};
    hueBar_[ 8] = {{barX1, size.y * (4.0f / 6.0f)}, {  0,   0, 255}};
    hueBar_[ 9] = {{barX2, size.y * (4.0f / 6.0f)}, {  0,   0, 255}};
    hueBar_[10] = {{barX1, size.y * (5.0f / 6.0f)}, {255,   0, 255}};
    hueBar_[11] = {{barX2, size.y * (5.0f / 6.0f)}, {255,   0, 255}};
    hueBar_[12] = {{barX1, size.y * (6.0f / 6.0f)}, {255,   0,   0}};
    hueBar_[13] = {{barX2, size.y * (6.0f / 6.0f)}, {255,   0,   0}};

    requestRedraw();
}
const sf::Vector2f& ColorPicker::getSize() const {
    return size_;
}

void ColorPicker::setStyle(std::shared_ptr<ColorPickerStyle> style) {
    style_ = style;
    styleCopied_ = false;
    requestRedraw();
}
std::shared_ptr<ColorPickerStyle> ColorPicker::getStyle() {
    if (!styleCopied_) {
        style_ = style_->clone();
        styleCopied_ = true;
    }
    return style_;
}

sf::FloatRect ColorPicker::getLocalBounds() const {
    return {-getOrigin(), size_};
}
bool ColorPicker::isMouseIntersecting(const sf::Vector2f& mouseParent) const {
    return Widget::isMouseIntersecting(mouseParent);
}

ColorPicker::ColorPicker(std::shared_ptr<ColorPickerStyle> style, const sf::String& name) :
    baseClass(name),
    style_(style),
    styleCopied_(false),
    size_(0.0f, 0.0f),
    saturationArea_(sf::TriangleStrip, 4),
    hueBar_(sf::TriangleStrip, 14) {
}

void ColorPicker::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (!isVisible()) {
        return;
    }
    states.transform *= getTransform();

    target.draw(saturationArea_, states);
    target.draw(hueBar_, states);

    for (const auto& child : getChildren()) {
        target.draw(*child, states);
    }
}

}
