#include <gui/Gui.h>
#include <gui/Theme.h>
#include <gui/widgets/Label.h>
#include <gui/widgets/Slider.h>

#include <algorithm>
#include <cassert>
#include <cmath>

namespace {

/**
 * Blends two colors, just like the OpenGL blend mode:
 * `glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);`
 * 
 * For this function, the destination alpha is preserved though.
 */
sf::Color blendColors(const sf::Color& src, const sf::Color& dest) {
    return {
        static_cast<uint8_t>((static_cast<int>(src.r) * src.a + static_cast<int>(dest.r) * (255 - src.a)) / 255),
        static_cast<uint8_t>((static_cast<int>(src.g) * src.a + static_cast<int>(dest.g) * (255 - src.a)) / 255),
        static_cast<uint8_t>((static_cast<int>(src.b) * src.a + static_cast<int>(dest.b) * (255 - src.a)) / 255),
        dest.a
    };
}

}

namespace gui {

SliderStyle::SliderStyle(const Gui& gui) :
    Style(gui) {
}

// sf::Shape interface.
void SliderStyle::setTexture(const sf::Texture* texture, bool resetRect) {
    rect_.setTexture(texture, resetRect);
    gui_.requestRedraw();
}
void SliderStyle::setTextureRect(const sf::IntRect& rect) {
    rect_.setTextureRect(rect);
    gui_.requestRedraw();
}
void SliderStyle::setFillColor(const sf::Color& color) {
    colorRect_ = color;
    gui_.requestRedraw();
}
void SliderStyle::setOutlineColor(const sf::Color& color) {
    rect_.setOutlineColor(color);
    gui_.requestRedraw();
}
void SliderStyle::setOutlineThickness(float thickness) {
    rect_.setOutlineThickness(thickness);
    gui_.requestRedraw();
}
const sf::Texture* SliderStyle::getTexture() const {
    return rect_.getTexture();
}
const sf::IntRect& SliderStyle::getTextureRect() const {
    return rect_.getTextureRect();
}
const sf::Color& SliderStyle::getFillColor() const {
    return colorRect_;
}
const sf::Color& SliderStyle::getOutlineColor() const {
    return rect_.getOutlineColor();
}
float SliderStyle::getOutlineThickness() const {
    return rect_.getOutlineThickness();
}

// sf::Shape interface.
void SliderStyle::setThumbTexture(const sf::Texture* texture, bool resetRect) {
    thumb_.setTexture(texture, resetRect);
    gui_.requestRedraw();
}
void SliderStyle::setThumbTextureRect(const sf::IntRect& rect) {
    thumb_.setTextureRect(rect);
    gui_.requestRedraw();
}
void SliderStyle::setThumbFillColor(const sf::Color& color) {
    colorThumb_ = color;
    gui_.requestRedraw();
}
void SliderStyle::setThumbOutlineColor(const sf::Color& color) {
    thumb_.setOutlineColor(color);
    gui_.requestRedraw();
}
void SliderStyle::setThumbOutlineThickness(float thickness) {
    thumb_.setOutlineThickness(thickness);
    gui_.requestRedraw();
}
const sf::Texture* SliderStyle::getThumbTexture() const {
    return thumb_.getTexture();
}
const sf::IntRect& SliderStyle::getThumbTextureRect() const {
    return thumb_.getTextureRect();
}
const sf::Color& SliderStyle::getThumbFillColor() const {
    return colorThumb_;
}
const sf::Color& SliderStyle::getThumbOutlineColor() const {
    return thumb_.getOutlineColor();
}
float SliderStyle::getThumbOutlineThickness() const {
    return thumb_.getOutlineThickness();
}

void SliderStyle::setFillColorHover(const sf::Color& color) {
    colorRectHover_ = color;
    gui_.requestRedraw();
}
void SliderStyle::setFillColorDown(const sf::Color& color) {
    colorThumbDown_ = color;
    gui_.requestRedraw();
}
void SliderStyle::setThumbMinWidth(float thumbMinWidth) {
    thumbMinWidth_ = thumbMinWidth;
    gui_.requestRedraw();
}
const sf::Color& SliderStyle::getFillColorHover() const {
    return colorRectHover_;
}
const sf::Color& SliderStyle::getFillColorDown() const {
    return colorThumbDown_;
}
float SliderStyle::getThumbMinWidth() const {
    return thumbMinWidth_;
}

std::shared_ptr<SliderStyle> SliderStyle::clone() const {
    return std::make_shared<SliderStyle>(*this);
}



std::shared_ptr<Slider> Slider::create(const Theme& theme, const sf::String& name) {
    return std::shared_ptr<Slider>(new Slider(theme.getStyle<SliderStyle>("Slider"), name));
}
std::shared_ptr<Slider> Slider::create(std::shared_ptr<SliderStyle> style, const sf::String& name) {
    return std::shared_ptr<Slider>(new Slider(style, name));
}

void Slider::setSize(const sf::Vector2f& size) {
    size_ = size;
    requestRedraw();
}
void Slider::setLabel(std::shared_ptr<Label> label) {
    label_ = label;
    if (label != nullptr) {
        onValueChange.emit(this, value_);
    }
    requestRedraw();
}
void Slider::setRange(const std::pair<float, float>& range) {
    range_.first = std::min(range.first, range.second);
    range_.second = std::max(range.first, range.second);
    setValue(value_);
}
void Slider::setValue(float value) {
    float newValue = std::min(std::max(value, range_.first), range_.second);
    if (newValue != value_) {
        value_ = newValue;
        onValueChange.emit(this, newValue);
        requestRedraw();
    }
}
void Slider::setStep(float step) {
    assert(step >= 0.0f);
    step_ = step;
}
const sf::Vector2f& Slider::getSize() const {
    return size_;
}
std::shared_ptr<Label> Slider::getLabel() const {
    return label_;
}
const std::pair<float, float>& Slider::getRange() const {
    return range_;
}
float Slider::getValue() const {
    return value_;
}
float Slider::getStep() const {
    return step_;
}

void Slider::setStyle(std::shared_ptr<SliderStyle> style) {
    style_ = style;
    styleCopied_ = false;
    requestRedraw();
}
std::shared_ptr<SliderStyle> Slider::getStyle() {
    if (!styleCopied_) {
        style_ = style_->clone();
        styleCopied_ = true;
    }
    return style_;
}

sf::FloatRect Slider::getLocalBounds() const {
    return {-getOrigin(), size_};
}

bool Slider::handleMouseMove(const sf::Vector2f& mouseParent) {
    if (Widget::handleMouseMove(mouseParent)) {
        return true;
    }
    const auto mouseLocal = toLocalOriginSpace(mouseParent);

    if (isDragging_) {
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            updateThumb(mouseLocal.x + getOrigin().x);
        } else {
            isDragging_ = false;
            requestRedraw();
        }
    }

    return isDragging_;
}
void Slider::handleMousePress(sf::Mouse::Button button, const sf::Vector2f& mouseParent) {
    Widget::handleMousePress(button, mouseParent);
    const auto mouseLocal = toLocalOriginSpace(mouseParent);
    if (button == sf::Mouse::Left && !isDragging_) {
        updateThumb(mouseLocal.x + getOrigin().x);
        isDragging_ = true;
        requestRedraw();
    }
}
void Slider::handleMouseRelease(sf::Mouse::Button button, const sf::Vector2f& mouseParent) {
    if (button == sf::Mouse::Left && isDragging_) {
        isDragging_ = false;
        requestRedraw();
    }
    Widget::handleMouseRelease(button, mouseParent);
}

void Slider::handleMouseEntered() {
    Widget::handleMouseEntered();
    requestRedraw();
}

void Slider::handleMouseLeft() {
    requestRedraw();
    Widget::handleMouseLeft();
}

Slider::Slider(std::shared_ptr<SliderStyle> style, const sf::String& name) :
    Widget(name),
    style_(style),
    styleCopied_(false),
    size_(0.0f, 0.0f),
    label_(nullptr),
    range_(0.0f, 1.0f),
    value_(0.0f),
    step_(0.0f),
    isDragging_(false) {
}

float Slider::getThumbWidth() const {
    float numThumbPositions;
    if (step_ == 0.0f) {
        numThumbPositions = size_.x;
    } else {
        numThumbPositions = std::floor((range_.second - range_.first) / step_) + 1.0f;
    }
    return std::max(size_.x / numThumbPositions, style_->thumbMinWidth_);
}

void Slider::updateThumb(float thumbPosition) {
    const float thumbWidth = getThumbWidth();
    const float thumbNormalized = std::min(std::max((thumbPosition - 0.5f * thumbWidth) / (size_.x - thumbWidth), 0.0f), 1.0f);

    if (step_ == 0.0f) {
        setValue((range_.second - range_.first) * thumbNormalized + range_.first);
    } else {
        float numThumbPositions = std::floor((range_.second - range_.first) / step_) + 1.0f;
        setValue(std::round(thumbNormalized * (numThumbPositions - 1.0f)) * step_ + range_.first);
    }
}

void Slider::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (!isVisible()) {
        return;
    }
    states.transform *= getTransform();

    const float thumbWidth = getThumbWidth();

    style_->rect_.setSize(size_);
    if (isMouseHovering()) {
        style_->rect_.setFillColor(blendColors(style_->colorRectHover_, style_->colorRect_));
    } else {
        style_->rect_.setFillColor(style_->colorRect_);
    }
    target.draw(style_->rect_, states);
    style_->thumb_.setSize({thumbWidth, size_.y});
    style_->thumb_.setPosition((value_ - range_.first) / (range_.second - range_.first) * (size_.x - thumbWidth), 0.0f);
    style_->thumb_.setFillColor(isDragging_ ? style_->colorThumbDown_ : style_->colorThumb_);
    target.draw(style_->thumb_, states);

    if (label_ != nullptr) {
        target.draw(*label_, states);
    }
}

}
