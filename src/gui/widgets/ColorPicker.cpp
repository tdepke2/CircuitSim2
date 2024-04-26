#include <gui/Gui.h>
#include <gui/Theme.h>
#include <gui/widgets/ColorPicker.h>
#include <gui/widgets/Label.h>
#include <gui/widgets/MultilineTextBox.h>
#include <gui/widgets/Slider.h>

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
    return std::shared_ptr<ColorPicker>(new ColorPicker(theme, name));
}
//std::shared_ptr<ColorPicker> ColorPicker::create(std::shared_ptr<ColorPickerStyle> style, const sf::String& name) {
//    return std::shared_ptr<ColorPicker>(new ColorPicker(style, name));
//}

void ColorPicker::setSize(const sf::Vector2f& size) {
    size_ = size;

    // FIXME: these should go in style settings.
    //const sf::Vector2f stylePadding = {0.0f, 0.0f};   // I don't think we need any global padding.
    const float styleBarWidth = 20.0f;
    const float styleBarSpacing = 5.0f;
    const float styleLabelSpacing = 5.0f;
    const float styleBoxSpacing = 5.0f;    // should combine into vec with above.

    // FIXME: do this during ctor instead? maybe not, need to recompute colors each time hue changes.
    constexpr int gridSideCount = 16;
    const sf::Vector2f shadingRectangleSize = {
        size.x - (styleBarWidth + styleBarSpacing) * 2.0f,
        size.y - (rgbaLabel_->getSize().y + styleLabelSpacing) * 3
    };
    const sf::Vector2f gridSize = shadingRectangleSize / static_cast<float>(gridSideCount);

    const float hue = hueSlider_->getValue() / 255.0f;
    shadingRectangle_.resize(2 * (gridSideCount + 1) * gridSideCount);

    int i = 0;
    for (int y = 0; y < gridSideCount; ++y) {
        int dx = 1 - (y % 2) * 2;
        int xStart = (y % 2 == 0 ? 0 : gridSideCount);
        int xStop = gridSideCount - xStart + dx;

        for (int x = xStart; x != xStop; x += dx) {
            sf::Color topColor, bottomColor;
            if (y == 0) {
                sf::Vector3f top = convertHsvToRgb(sf::Vector3f(hue, static_cast<float>(x) / gridSideCount, 1.0f - static_cast<float>(y) / gridSideCount));
                topColor = {
                    static_cast<uint8_t>(std::lround(top.x * 255.0f)),
                    static_cast<uint8_t>(std::lround(top.y * 255.0f)),
                    static_cast<uint8_t>(std::lround(top.z * 255.0f))
                };
            } else {
                topColor = shadingRectangle_[i - std::abs(x - xStart) * 4 - 1].color;
            }
            sf::Vector3f bottom = convertHsvToRgb(sf::Vector3f(hue, static_cast<float>(x) / gridSideCount, 1.0f - static_cast<float>(y + 1) / gridSideCount));
            bottomColor = {
                static_cast<uint8_t>(std::lround(bottom.x * 255.0f)),
                static_cast<uint8_t>(std::lround(bottom.y * 255.0f)),
                static_cast<uint8_t>(std::lround(bottom.z * 255.0f))
            };

            shadingRectangle_[i++] = {{x * gridSize.x, y * gridSize.y}, topColor};
            shadingRectangle_[i++] = {{x * gridSize.x, (y + 1) * gridSize.y}, bottomColor};
        }
    }
    assert(i == static_cast<int>(shadingRectangle_.getVertexCount()));

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

    float barHeight = shadingRectangleSize.y;
    float barX1 = size.x - 2.0f * styleBarWidth - styleBarSpacing;
    float barX2 = barX1 + styleBarWidth;
    hueBar_[ 0] = {{barX1, barHeight * (0.0f / 6.0f)}, {255,   0,   0}};
    hueBar_[ 1] = {{barX2, barHeight * (0.0f / 6.0f)}, {255,   0,   0}};
    hueBar_[ 2] = {{barX1, barHeight * (1.0f / 6.0f)}, {255, 255,   0}};
    hueBar_[ 3] = {{barX2, barHeight * (1.0f / 6.0f)}, {255, 255,   0}};
    hueBar_[ 4] = {{barX1, barHeight * (2.0f / 6.0f)}, {  0, 255,   0}};
    hueBar_[ 5] = {{barX2, barHeight * (2.0f / 6.0f)}, {  0, 255,   0}};
    hueBar_[ 6] = {{barX1, barHeight * (3.0f / 6.0f)}, {  0, 255, 255}};
    hueBar_[ 7] = {{barX2, barHeight * (3.0f / 6.0f)}, {  0, 255, 255}};
    hueBar_[ 8] = {{barX1, barHeight * (4.0f / 6.0f)}, {  0,   0, 255}};
    hueBar_[ 9] = {{barX2, barHeight * (4.0f / 6.0f)}, {  0,   0, 255}};
    hueBar_[10] = {{barX1, barHeight * (5.0f / 6.0f)}, {255,   0, 255}};
    hueBar_[11] = {{barX2, barHeight * (5.0f / 6.0f)}, {255,   0, 255}};
    hueBar_[12] = {{barX1, barHeight * (6.0f / 6.0f)}, {255,   0,   0}};
    hueBar_[13] = {{barX2, barHeight * (6.0f / 6.0f)}, {255,   0,   0}};

    hueSlider_->setSize({barHeight, styleBarWidth});
    hueSlider_->setPosition(barX1 + styleBarWidth, 0.0f);

    const float alphaWidth = static_cast<float>(alphaTexture_.getSize().x);
    const float alphaHeight = barHeight / (styleBarWidth * 6.0f) * alphaWidth;
    barX1 = size.x - styleBarWidth;
    barX2 = barX1 + styleBarWidth;
    alphaBar_[ 0] = {{barX1, barHeight * (0.0f / 6.0f)}, {  0,   0,   0}, {     0.0f,  alphaHeight * 0.0f}};
    alphaBar_[ 1] = {{barX2, barHeight * (0.0f / 6.0f)}, {  0,   0,   0}, {alphaWidth, alphaHeight * 0.0f}};
    alphaBar_[ 2] = {{barX1, barHeight * (1.0f / 6.0f)}, { 43,  43,  43}, {     0.0f,  alphaHeight * 1.0f}};
    alphaBar_[ 3] = {{barX2, barHeight * (1.0f / 6.0f)}, { 43,  43,  43}, {alphaWidth, alphaHeight * 1.0f}};
    alphaBar_[ 4] = {{barX1, barHeight * (2.0f / 6.0f)}, { 85,  85,  85}, {     0.0f,  alphaHeight * 2.0f}};
    alphaBar_[ 5] = {{barX2, barHeight * (2.0f / 6.0f)}, { 85,  85,  85}, {alphaWidth, alphaHeight * 2.0f}};
    alphaBar_[ 6] = {{barX1, barHeight * (3.0f / 6.0f)}, {128, 128, 128}, {     0.0f,  alphaHeight * 3.0f}};
    alphaBar_[ 7] = {{barX2, barHeight * (3.0f / 6.0f)}, {128, 128, 128}, {alphaWidth, alphaHeight * 3.0f}};
    alphaBar_[ 8] = {{barX1, barHeight * (4.0f / 6.0f)}, {170, 170, 170}, {     0.0f,  alphaHeight * 4.0f}};
    alphaBar_[ 9] = {{barX2, barHeight * (4.0f / 6.0f)}, {170, 170, 170}, {alphaWidth, alphaHeight * 4.0f}};
    alphaBar_[10] = {{barX1, barHeight * (5.0f / 6.0f)}, {213, 213, 213}, {     0.0f,  alphaHeight * 5.0f}};
    alphaBar_[11] = {{barX2, barHeight * (5.0f / 6.0f)}, {213, 213, 213}, {alphaWidth, alphaHeight * 5.0f}};
    alphaBar_[12] = {{barX1, barHeight * (6.0f / 6.0f)}, {255, 255, 255}, {     0.0f,  alphaHeight * 6.0f}};
    alphaBar_[13] = {{barX2, barHeight * (6.0f / 6.0f)}, {255, 255, 255}, {alphaWidth, alphaHeight * 6.0f}};

    alphaSlider_->setSize({barHeight, styleBarWidth});
    alphaSlider_->setPosition(barX1 + styleBarWidth, 0.0f);

    // FIXME would be nice if we could stretch the boxes to fit the area.
    // maybe stretch the hex one first, then line up the others with the width of that one?

    sf::Vector2f nextPosition;

    const float textRow1 = size.y - 3.0f * rgbaLabel_->getSize().y - 2.0f * styleLabelSpacing;
    rgbaLabel_->setPosition(0.0f, textRow1);
    nextPosition = rgbaLabel_->getPosition() + sf::Vector2f(rgbaLabel_->getSize().x + styleBoxSpacing, 0.0f);
    for (const auto& box : rgbaTextBoxes_) {
        box->setPosition(nextPosition);
        nextPosition = box->getPosition() + sf::Vector2f(box->getSize().x + styleBoxSpacing, 0.0f);
    }

    const float textRow2 = size.y - 2.0f * rgbaLabel_->getSize().y - styleLabelSpacing;
    hsvaLabel_->setPosition(0.0f, textRow2);
    nextPosition = hsvaLabel_->getPosition() + sf::Vector2f(hsvaLabel_->getSize().x + styleBoxSpacing, 0.0f);
    for (const auto& box : hsvaTextBoxes_) {
        box->setPosition(nextPosition);
        nextPosition = box->getPosition() + sf::Vector2f(box->getSize().x + styleBoxSpacing, 0.0f);
    }

    const float textRow3 = size.y - rgbaLabel_->getSize().y;
    rgbaHexLabel_->setPosition(0.0f, textRow3);
    rgbaHexTextBox_->setPosition(rgbaHexLabel_->getPosition() + sf::Vector2f(rgbaHexLabel_->getSize().x + styleBoxSpacing, 0.0f));

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

ColorPicker::ColorPicker(const Theme& theme, const sf::String& name) :
    baseClass(name),
    style_(theme.getStyle<ColorPickerStyle>("ColorPicker")),
    styleCopied_(false),
    size_(0.0f, 0.0f),
    alphaTexture_(),
    shadingRectangle_(sf::TriangleStrip),
    hueBar_(sf::TriangleStrip, 14),
    alphaBar_(sf::TriangleStrip, 14) {

    constexpr unsigned int alphaSize = 16;
    sf::Image alphaImage;
    alphaImage.create(alphaSize, alphaSize, {200, 200, 200});
    for (unsigned int y = 0; y < alphaSize; ++y) {
        for (unsigned int x = 0; x < alphaSize; ++x) {
            if ((y >= alphaSize / 2) != (x >= alphaSize / 2)) {
                alphaImage.setPixel(x, y, {127, 127, 127});
            }
        }
    }
    alphaTexture_.loadFromImage(alphaImage);
    alphaTexture_.setRepeated(true);

    hueSlider_ = Slider::create(theme);
    hueSlider_->setRange({0.0f, 255.0f});
    hueSlider_->setStep(1.0f);
    hueSlider_->setRotation(90.0f);
    addChild(hueSlider_);

    hueSlider_->onValueChange.connect([this](Widget* /*w*/, float value) {
        std::cout << "hue changed to " << value << "\n";
        setSize(size_);
    });

    alphaSlider_ = Slider::create(theme);
    alphaSlider_->setRange({0.0f, 255.0f});
    alphaSlider_->setStep(1.0f);
    alphaSlider_->setRotation(90.0f);
    addChild(alphaSlider_);

    alphaSlider_->onValueChange.connect([this](Widget* /*w*/, float value) {
        std::cout << "alpha changed to " << value << "\n";
        setSize(size_);
    });

    auto barSliderStyle = hueSlider_->getStyle();
    barSliderStyle->setFillColor({0, 0, 0, 0});
    barSliderStyle->setThumbOutlineColor(sf::Color::White);
    barSliderStyle->setThumbOutlineThickness(-1.0f);
    barSliderStyle->setThumbFillColor({0, 0, 0, 0});
    sf::Color thumbColorDown = barSliderStyle->getFillColorDown();
    thumbColorDown.a = 100;
    barSliderStyle->setFillColorDown(thumbColorDown);
    barSliderStyle->setThumbMinWidth(5.0f);
    alphaSlider_->setStyle(barSliderStyle);

    rgbaLabel_ = Label::create(theme);
    rgbaLabel_->setLabel("RGBA: ");
    addChild(rgbaLabel_);

    for (auto& box : rgbaTextBoxes_) {
        box = MultilineTextBox::create(theme);
        box->setSizeCharacters({3, 1});
        box->setMaxCharacters(3);
        box->setMaxLines(1);
        box->setTabPolicy(MultilineTextBox::TabPolicy::ignoreTab);
        addChild(box);
    }

    hsvaLabel_ = Label::create(theme);
    hsvaLabel_->setLabel("HSVA: ");
    addChild(hsvaLabel_);

    for (auto& box : hsvaTextBoxes_) {
        box = MultilineTextBox::create(theme);
        box->setSizeCharacters({3, 1});
        box->setMaxCharacters(3);
        box->setMaxLines(1);
        box->setTabPolicy(MultilineTextBox::TabPolicy::ignoreTab);
        addChild(box);
    }

    rgbaHexLabel_ = Label::create(theme);
    rgbaHexLabel_->setLabel("Hex:  ");
    addChild(rgbaHexLabel_);

    rgbaHexTextBox_ = MultilineTextBox::create(theme);
    rgbaHexTextBox_->setSizeCharacters({10, 1});
    rgbaHexTextBox_->setMaxCharacters(10);
    rgbaHexTextBox_->setMaxLines(1);
    rgbaHexTextBox_->setTabPolicy(MultilineTextBox::TabPolicy::ignoreTab);
    addChild(rgbaHexTextBox_);
}

void ColorPicker::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (!isVisible()) {
        return;
    }
    states.transform *= getTransform();

    target.draw(shadingRectangle_, states);
    target.draw(hueBar_, states);
    states.texture = &alphaTexture_;
    target.draw(alphaBar_, states);
    states.texture = nullptr;

    sf::Sprite test(alphaTexture_);
    target.draw(test, states);

    for (const auto& child : getChildren()) {
        target.draw(*child, states);
    }
}

}
