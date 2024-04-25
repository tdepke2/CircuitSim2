#include <gui/Gui.h>
#include <gui/Theme.h>
#include <gui/widgets/ColorPicker.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>



#include <iostream>





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
sf::Vector3f convertRgbToHsv(const sf::Vector3f& rgb) {
    float value = std::max(std::max(rgb.x, rgb.y), rgb.z);
    float chroma = value - std::min(std::min(rgb.x, rgb.y), rgb.z);
    float hue;
    if (chroma == 0.0f) {
        hue = 0.0f;
    } else if (value == rgb.x) {
        hue = (rgb.y - rgb.z) / chroma + 6.0f;
    } else if (value == rgb.y) {
        hue = (rgb.z - rgb.x) / chroma + 2.0f;
    } else {    // value == rgb.z
        hue = (rgb.x - rgb.y) / chroma + 4.0f;
    }
    hue = std::fmod(hue, 6.0f) / 6.0f;

    return {
        hue, (value == 0.0f ? 0.0f : chroma / value), value
    };
}

HsvColor convertRgbToHsv(const sf::Color& color) {
    sf::Vector3f hsv = convertRgbToHsv(sf::Vector3f(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f));
    return {
        static_cast<uint8_t>(std::lround(hsv.x * 255.0f)),
        static_cast<uint8_t>(std::lround(hsv.y * 255.0f)),
        static_cast<uint8_t>(std::lround(hsv.z * 255.0f)),
        color.a
    };
}

sf::Vector3f convertHsvToRgb(const sf::Vector3f& hsv) {
    float chroma = hsv.y * hsv.z;
    float m = hsv.z - chroma;
    float x = chroma * (1.0f - std::fabs(std::fmod(hsv.x * 6.0f, 2.0f) - 1.0f));

    int hueSection = static_cast<int>(hsv.x * 6.0f);
    switch(hueSection) {
    case 0:
        return {hsv.z, x + m, m};
    case 1:
        return {x + m, hsv.z, m};
    case 2:
        return {m, hsv.z, x + m};
    case 3:
        return {m, x + m, hsv.z};
    case 4:
        return {x + m, m, hsv.z};
    default:
        return {hsv.z, m, x + m};
    }
}

sf::Color convertHsvToRgb(const HsvColor& color) {
    sf::Vector3f rgb = convertHsvToRgb(sf::Vector3f(color.h / 255.0f, color.s / 255.0f, color.v / 255.0f));
    return {
        static_cast<uint8_t>(std::lround(rgb.x * 255.0f)),
        static_cast<uint8_t>(std::lround(rgb.y * 255.0f)),
        static_cast<uint8_t>(std::lround(rgb.z * 255.0f)),
        color.a
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

    // FIXME: do this during ctor instead?
    constexpr int gridSideCount = 16;
    const float squareSize = size.y;
    const float gridSize = squareSize / gridSideCount;
    saturationArea_.resize(2 * (gridSideCount + 1) * gridSideCount);

    int i = 0;
    for (int y = 0; y < gridSideCount; ++y) {
        int dx = 1 - (y % 2) * 2;
        int xStart = (y % 2 == 0 ? 0 : gridSideCount);
        int xStop = gridSideCount - xStart + dx;

        for (int x = xStart; x != xStop; x += dx) {
            sf::Vector3f top = convertHsvToRgb(sf::Vector3f(0.0f, static_cast<float>(x) / gridSideCount, 1.0f - static_cast<float>(y) / gridSideCount));
            sf::Vector3f bottom = convertHsvToRgb(sf::Vector3f(0.0f, static_cast<float>(x) / gridSideCount, 1.0f - static_cast<float>(y + 1) / gridSideCount));

            saturationArea_[i++] = {{x * gridSize, y * gridSize}, sf::Color(
                static_cast<uint8_t>(std::lround(top.x * 255.0f)),
                static_cast<uint8_t>(std::lround(top.y * 255.0f)),
                static_cast<uint8_t>(std::lround(top.z * 255.0f))
            )};
            saturationArea_[i++] = {{x * gridSize, (y + 1) * gridSize}, sf::Color(
                static_cast<uint8_t>(std::lround(bottom.x * 255.0f)),
                static_cast<uint8_t>(std::lround(bottom.y * 255.0f)),
                static_cast<uint8_t>(std::lround(bottom.z * 255.0f))
            )};
        }
    }
    assert(i == static_cast<int>(saturationArea_.getVertexCount()));

    /*

Example with gridSideCount = 3

 0----2----4----6
 |  / |  / |  / |
 | /  | /  | /  |
 1----3----5----7
14---12---10----8
 | \  | \  | \  |
 |  \ |  \ |  \ |
15---13---11----9
16---18---20---22
 |  / |  / |  / |
 | /  | /  | /  |
17---19---21---23

    */

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
    saturationArea_(sf::TriangleStrip),
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
