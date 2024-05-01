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
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>



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

inline std::array<float, 4> convertRgbToHsv(const std::array<float, 4>& rgb) {
    sf::Vector3f hsv = convertRgbToHsv(sf::Vector3f(rgb[0], rgb[1], rgb[2]));
    return {hsv.x, hsv.y, hsv.z, rgb[3]};
}

/*inline HsvColor convertRgbToHsv(const sf::Color& color) {
    sf::Vector3f hsv = convertRgbToHsv(sf::Vector3f(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f));
    return {
        static_cast<uint8_t>(std::lround(hsv.x * 255.0f)),
        static_cast<uint8_t>(std::lround(hsv.y * 255.0f)),
        static_cast<uint8_t>(std::lround(hsv.z * 255.0f)),
        color.a
    };
}*/

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

inline std::array<float, 4> convertHsvToRgb(const std::array<float, 4>& hsv) {
    sf::Vector3f rgb = convertHsvToRgb(sf::Vector3f(hsv[0], hsv[1], hsv[2]));
    return {rgb.x, rgb.y, rgb.z, hsv[3]};
}

/*inline sf::Color convertHsvToRgb(const HsvColor& color) {
    sf::Vector3f rgb = convertHsvToRgb(sf::Vector3f(color.h / 255.0f, color.s / 255.0f, color.v / 255.0f));
    return {
        static_cast<uint8_t>(std::lround(rgb.x * 255.0f)),
        static_cast<uint8_t>(std::lround(rgb.y * 255.0f)),
        static_cast<uint8_t>(std::lround(rgb.z * 255.0f)),
        color.a
    };
}*/

inline std::array<float, 4> normalizeRgb(const sf::Color& color) {
    return {color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f};
}

inline std::array<float, 4> normalizeHsv(const HsvColor& color) {
    return {color.h / 255.0f, color.s / 255.0f, color.v / 255.0f, color.a / 255.0f};
}

inline sf::Color denormalizeRgb(const std::array<float, 4>& arr) {
    return {
        static_cast<uint8_t>(std::lround(arr[0] * 255.0f)),
        static_cast<uint8_t>(std::lround(arr[1] * 255.0f)),
        static_cast<uint8_t>(std::lround(arr[2] * 255.0f)),
        static_cast<uint8_t>(std::lround(arr[3] * 255.0f))
    };
}

inline HsvColor denormalizeHsv(const std::array<float, 4>& arr) {
    return {
        static_cast<uint8_t>(std::lround(arr[0] * 255.0f)),
        static_cast<uint8_t>(std::lround(arr[1] * 255.0f)),
        static_cast<uint8_t>(std::lround(arr[2] * 255.0f)),
        static_cast<uint8_t>(std::lround(arr[3] * 255.0f))
    };
}

}

namespace gui {

ColorPickerStyle::ColorPickerStyle(const Gui& gui) :
    Style(gui) {
}

// sf::Shape interface.
void ColorPickerStyle::setTexture(const sf::Texture* texture, bool resetRect) {
    dot_.setTexture(texture, resetRect);
    gui_.requestRedraw();
}
void ColorPickerStyle::setTextureRect(const sf::IntRect& rect) {
    dot_.setTextureRect(rect);
    gui_.requestRedraw();
}
//void ColorPickerStyle::setFillColor(const sf::Color& color) {
//    dot_.setFillColor(color);
//    gui_.requestRedraw();
//}
void ColorPickerStyle::setOutlineColor(const sf::Color& color) {
    dot_.setOutlineColor(color);
    gui_.requestRedraw();
}
void ColorPickerStyle::setOutlineThickness(float thickness) {
    dot_.setOutlineThickness(thickness);
    gui_.requestRedraw();
}
const sf::Texture* ColorPickerStyle::getTexture() const {
    return dot_.getTexture();
}
const sf::IntRect& ColorPickerStyle::getTextureRect() const {
    return dot_.getTextureRect();
}
//const sf::Color& ColorPickerStyle::getFillColor() const {
//    return dot_.getFillColor();
//}
const sf::Color& ColorPickerStyle::getOutlineColor() const {
    return dot_.getOutlineColor();
}
float ColorPickerStyle::getOutlineThickness() const {
    return dot_.getOutlineThickness();
}

void ColorPickerStyle::setDotRadius(float radius) {
    dot_.setRadius(radius);
    dot_.setOrigin(radius, radius);
    gui_.requestRedraw();
}
float ColorPickerStyle::getDotRadius() const {
    return dot_.getRadius();
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
    updateLayout();
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

bool ColorPicker::handleMouseMove(const sf::Vector2f& mouseParent) {
    if (baseClass::handleMouseMove(mouseParent)) {
        return true;
    }
    const auto mouseLocal = toLocalOriginSpace(mouseParent);

    if (isDragging_) {
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            std::cout << "ColorPicker::handleMouseMove() with mouse down.\n";
            const auto shadingRectangleSize = getShadingRectangleSize();
            updateShadingRectangle(mouseLocal.x / shadingRectangleSize.x, 1.0f - mouseLocal.y / shadingRectangleSize.y);
        } else {
            isDragging_ = false;
        }
    }

    return isDragging_;
}
void ColorPicker::handleMousePress(sf::Mouse::Button button, const sf::Vector2f& mouseParent) {
    baseClass::handleMousePress(button, mouseParent);
    const auto mouseLocal = toLocalOriginSpace(mouseParent);
    const auto shadingRectangleSize = getShadingRectangleSize();
    if (button == sf::Mouse::Left && !isDragging_ && mouseLocal.x < shadingRectangleSize.x && mouseLocal.y < shadingRectangleSize.y) {
        std::cout << "ColorPicker::handleMousePress().\n";
        updateShadingRectangle(mouseLocal.x / shadingRectangleSize.x, 1.0f - mouseLocal.y / shadingRectangleSize.y);
        isDragging_ = true;
    }
}
void ColorPicker::handleMouseRelease(sf::Mouse::Button button, const sf::Vector2f& mouseParent) {
    if (button == sf::Mouse::Left && isDragging_) {
        isDragging_ = false;
    }
    baseClass::handleMouseRelease(button, mouseParent);
}

ColorPicker::ColorPicker(const Theme& theme, const sf::String& name) :
    baseClass(name),
    style_(theme.getStyle<ColorPickerStyle>("ColorPicker")),
    styleCopied_(false),
    size_(0.0f, 0.0f),
    alphaTexture_(),
    shadingRectangle_(sf::TriangleStrip),
    hueBar_(sf::TriangleStrip, 14),
    alphaBar_(sf::TriangleStrip, 14),
    currentColorHsva_{0.0f, 1.0f, 1.0f, 1.0f},
    isDragging_(false) {

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
    hueSlider_->setRange({0.0f, 1.0f});
    hueSlider_->setStep(0.0f);
    hueSlider_->setRotation(90.0f);
    addChild(hueSlider_);

    hueSlider_->onValueChange.connect([this](Widget* /*w*/, float value) {
        std::cout << "hue changed to " << value << "\n";
        currentColorHsva_[0] = value;
        updateCurrentColor(ColorSource::inputHsva, true, currentColorHsva_);
    });

    alphaSlider_ = Slider::create(theme);
    alphaSlider_->setRange({0.0f, 1.0f});
    alphaSlider_->setStep(0.0f);
    alphaSlider_->setRotation(90.0f);
    addChild(alphaSlider_);

    alphaSlider_->onValueChange.connect([this](Widget* /*w*/, float value) {
        std::cout << "alpha changed to " << value << "\n";
        currentColorHsva_[3] = 1.0f - value;
        updateCurrentColor(ColorSource::inputHsva, true, currentColorHsva_);
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
        box->setRegexPattern("[0-9]*");
        box->onTextChange.connect([this]() {
            updateCurrentColor(ColorSource::rgba, false);
        });
        box->onFocusLost.connect([this]() {
            updateCurrentColor(ColorSource::rgba, true);
        });
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
        box->setRegexPattern("[0-9]*");
        box->onTextChange.connect([this]() {
            updateCurrentColor(ColorSource::hsva, false);
        });
        box->onFocusLost.connect([this]() {
            updateCurrentColor(ColorSource::hsva, true);
        });
        addChild(box);
    }

    rgbaHexLabel_ = Label::create(theme);
    rgbaHexLabel_->setLabel("Hex#: ");
    addChild(rgbaHexLabel_);

    rgbaHexTextBox_ = MultilineTextBox::create(theme);
    rgbaHexTextBox_->setSizeCharacters({8, 1});
    rgbaHexTextBox_->setMaxCharacters(8);
    rgbaHexTextBox_->setMaxLines(1);
    rgbaHexTextBox_->setTabPolicy(MultilineTextBox::TabPolicy::ignoreTab);
    rgbaHexTextBox_->setRegexPattern("[0-9a-fA-F]*");
    rgbaHexTextBox_->onTextChange.connect([this]() {
        updateCurrentColor(ColorSource::rgbaHex, false);
    });
    rgbaHexTextBox_->onFocusLost.connect([this]() {
        updateCurrentColor(ColorSource::rgbaHex, true);
    });
    addChild(rgbaHexTextBox_);

    updateCurrentColor(ColorSource::inputHsva, true, currentColorHsva_);
}

sf::Vector2f ColorPicker::getShadingRectangleSize() const {
    // FIXME
    const float styleBarWidth = 20.0f;
    const float styleBarSpacing = 5.0f;
    const float styleLabelSpacing = 5.0f;

    return {
        size_.x - (styleBarWidth + styleBarSpacing) * 2.0f,
        size_.y - (rgbaLabel_->getSize().y + styleLabelSpacing) * 3.0f
    };
}

void ColorPicker::updateShadingRectangle(float saturation, float value) {
    currentColorHsva_[1] = std::min(std::max(saturation, 0.0f), 1.0f);
    currentColorHsva_[2] = std::min(std::max(value, 0.0f), 1.0f);
    updateCurrentColor(ColorSource::inputHsva, true, currentColorHsva_);
}

void ColorPicker::updateCurrentColor(ColorSource source, bool validateSource, const std::array<float, 4>& inputHsva) {
    auto getBoxValue = [](const std::shared_ptr<MultilineTextBox>& box) {
        int value = std::stoi(box->getText().toWideString());
        return static_cast<uint8_t>(std::min(std::max(value, 0), 255));
    };

    if (source != ColorSource::none) {
        // Set current color from source.
        if (source == ColorSource::rgba) {
            try {
                sf::Color rgba = {
                    getBoxValue(rgbaTextBoxes_[0]),
                    getBoxValue(rgbaTextBoxes_[1]),
                    getBoxValue(rgbaTextBoxes_[2]),
                    getBoxValue(rgbaTextBoxes_[3])
                };
                currentColorHsva_ = convertRgbToHsv(normalizeRgb(rgba));
            } catch (const std::invalid_argument& ex) {
                std::cout << "ColorSource::rgba failed: " << ex.what() << "\n";
            }
        } else if (source == ColorSource::hsva) {
            try {
                HsvColor hsva = {
                    getBoxValue(hsvaTextBoxes_[0]),
                    getBoxValue(hsvaTextBoxes_[1]),
                    getBoxValue(hsvaTextBoxes_[2]),
                    getBoxValue(hsvaTextBoxes_[3])
                };
                currentColorHsva_ = normalizeHsv(hsva);
            } catch (const std::invalid_argument& ex) {
                std::cout << "ColorSource::hsva failed: " << ex.what() << "\n";
            }
        } else if (source == ColorSource::rgbaHex) {
            try {
                sf::Color rgba(static_cast<uint32_t>(std::stoll(rgbaHexTextBox_->getText().toWideString(), nullptr, 16)));
                currentColorHsva_ = convertRgbToHsv(normalizeRgb(rgba));
            } catch (const std::invalid_argument& ex) {
                std::cout << "ColorSource::rgbaHex failed: " << ex.what() << "\n";
            }
        } else if (source == ColorSource::inputHsva) {
            currentColorHsva_ = inputHsva;
            validateSource = true;
        }

        // Fill in text boxes from current color.
        const sf::Color roundedRgba = denormalizeRgb(convertHsvToRgb(currentColorHsva_));
        const HsvColor roundedHsva = denormalizeHsv(currentColorHsva_);
        if (source != ColorSource::rgba || validateSource) {
            for (auto& box : rgbaTextBoxes_) {
                box->onTextChange.setEnabled(false);
            }
            rgbaTextBoxes_[0]->setText(std::to_string(static_cast<int>(roundedRgba.r)));
            rgbaTextBoxes_[1]->setText(std::to_string(static_cast<int>(roundedRgba.g)));
            rgbaTextBoxes_[2]->setText(std::to_string(static_cast<int>(roundedRgba.b)));
            rgbaTextBoxes_[3]->setText(std::to_string(static_cast<int>(roundedRgba.a)));
            for (auto& box : rgbaTextBoxes_) {
                box->onTextChange.setEnabled(true);
            }
        }
        if (source != ColorSource::hsva || validateSource) {
            for (auto& box : hsvaTextBoxes_) {
                box->onTextChange.setEnabled(false);
            }
            hsvaTextBoxes_[0]->setText(std::to_string(static_cast<int>(roundedHsva.h)));
            hsvaTextBoxes_[1]->setText(std::to_string(static_cast<int>(roundedHsva.s)));
            hsvaTextBoxes_[2]->setText(std::to_string(static_cast<int>(roundedHsva.v)));
            hsvaTextBoxes_[3]->setText(std::to_string(static_cast<int>(roundedHsva.a)));
            for (auto& box : hsvaTextBoxes_) {
                box->onTextChange.setEnabled(true);
            }
        }
        if (source != ColorSource::rgbaHex || validateSource) {
            std::stringstream ss;
            ss << std::setfill('0') << std::setw(8) << std::hex << roundedRgba.toInteger();
            rgbaHexTextBox_->onTextChange.setEnabled(false);
            rgbaHexTextBox_->setText(ss.str());
            rgbaHexTextBox_->onTextChange.setEnabled(true);
        }

        hueSlider_->onValueChange.setEnabled(false);
        alphaSlider_->onValueChange.setEnabled(false);
        hueSlider_->setValue(currentColorHsva_[0]);
        alphaSlider_->setValue(1.0f - currentColorHsva_[3]);
        hueSlider_->onValueChange.setEnabled(true);
        alphaSlider_->onValueChange.setEnabled(true);
    }

    /**
     * To display the `shadingRectangle_`, we build a grid of triangles to
     * approximate a linearly interpolated color area. This does not work with
     * just two triangles as the interpolation of vertex coords in OpenGL uses
     * barycentric coordinates.
     * 
     * Example showing vertex indices forming the TriangleStrip, here the
     * `gridSideCount` is set to 3:
     * 
     *  0----2----4----6
     *  |  / |  / |  / |
     *  | /  | /  | /  |
     *  1----3----5----7
     * 14---12---10----8
     *  | \  | \  | \  |
     *  |  \ |  \ |  \ |
     * 15---13---11----9
     * 16---18---20---22
     *  |  / |  / |  / |
     *  | /  | /  | /  |
     * 17---19---21---23
     */

    constexpr int gridSideCount = 16;
    const sf::Vector2f shadingRectangleSize = getShadingRectangleSize();
    const sf::Vector2f gridSize = shadingRectangleSize / static_cast<float>(gridSideCount);

    const float hue = hueSlider_->getValue();
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

    requestRedraw();
}

void ColorPicker::updateLayout() {
    // FIXME: these should go in style settings.
    //const sf::Vector2f stylePadding = {0.0f, 0.0f};   // I don't think we need any global padding.
    const float styleBarWidth = 20.0f;
    const float styleBarSpacing = 5.0f;
    const float styleLabelSpacing = 5.0f;
    const float styleBoxSpacing = 5.0f;    // should combine into vec with above.

    float barHeight = getShadingRectangleSize().y;
    float barX1 = size_.x - 2.0f * styleBarWidth - styleBarSpacing;
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
    barX1 = size_.x - styleBarWidth;
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

    // Put the rgbaHexTextBox_ in position first, then line up the rest of the boxes with it.
    const float textRow3 = size_.y - rgbaLabel_->getSize().y;
    rgbaHexLabel_->setPosition(0.0f, textRow3);
    rgbaHexTextBox_->setPosition(rgbaHexLabel_->getPosition() + sf::Vector2f(rgbaHexLabel_->getSize().x + styleBoxSpacing, 0.0f));
    rgbaHexTextBox_->setSizeWithinBounds({size_.x - rgbaHexTextBox_->getPosition().x, rgbaHexTextBox_->getSize().y});

    const float boxWidth = (rgbaHexTextBox_->getSize().x - 3.0f * styleBoxSpacing) / 4.0f;
    rgbaTextBoxes_[0]->setSizeWithinBounds({boxWidth, rgbaTextBoxes_[0]->getSize().y});
    const auto boxSizeCharacters = rgbaTextBoxes_[0]->getSizeCharacters();
    const float adjustedBoxSpacing = (rgbaHexTextBox_->getSize().x - rgbaTextBoxes_[0]->getSize().x * 4.0f) / 3.0f;
    sf::Vector2f nextPosition;

    const float textRow1 = size_.y - 3.0f * rgbaLabel_->getSize().y - 2.0f * styleLabelSpacing;
    rgbaLabel_->setPosition(0.0f, textRow1);
    nextPosition = rgbaLabel_->getPosition() + sf::Vector2f(rgbaLabel_->getSize().x + styleBoxSpacing, 0.0f);
    for (const auto& box : rgbaTextBoxes_) {
        box->setSizeCharacters(boxSizeCharacters);
        box->setPosition(nextPosition);
        nextPosition = box->getPosition() + sf::Vector2f(box->getSize().x + adjustedBoxSpacing, 0.0f);
    }

    const float textRow2 = size_.y - 2.0f * rgbaLabel_->getSize().y - styleLabelSpacing;
    hsvaLabel_->setPosition(0.0f, textRow2);
    nextPosition = hsvaLabel_->getPosition() + sf::Vector2f(hsvaLabel_->getSize().x + styleBoxSpacing, 0.0f);
    for (const auto& box : hsvaTextBoxes_) {
        box->setSizeCharacters(boxSizeCharacters);
        box->setPosition(nextPosition);
        nextPosition = box->getPosition() + sf::Vector2f(box->getSize().x + adjustedBoxSpacing, 0.0f);
    }

    updateCurrentColor(ColorSource::none, false);
}

void ColorPicker::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (!isVisible()) {
        return;
    }
    states.transform *= getTransform();

    target.draw(shadingRectangle_, states);
    const sf::Color roundedRgba = denormalizeRgb(convertHsvToRgb(currentColorHsva_));
    const sf::Vector2f shadingRectangleSize = getShadingRectangleSize();
    style_->dot_.setPosition(currentColorHsva_[1] * shadingRectangleSize.x, (1.0f - currentColorHsva_[2]) * shadingRectangleSize.y);
    style_->dot_.setFillColor({roundedRgba.r, roundedRgba.g, roundedRgba.b, 255});
    target.draw(style_->dot_, states);

    target.draw(hueBar_, states);
    states.texture = &alphaTexture_;
    target.draw(alphaBar_, states);
    states.texture = nullptr;

    for (const auto& child : getChildren()) {
        target.draw(*child, states);
    }
}

}
