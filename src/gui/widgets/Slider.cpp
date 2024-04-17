#include <gui/Gui.h>
#include <gui/Theme.h>
#include <gui/widgets/Slider.h>

#include <algorithm>
#include <cmath>

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
    rect_.setFillColor(color);
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
    return rect_.getFillColor();
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
    thumb_.setFillColor(color);
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
    return thumb_.getFillColor();
}
const sf::Color& SliderStyle::getThumbOutlineColor() const {
    return thumb_.getOutlineColor();
}
float SliderStyle::getThumbOutlineThickness() const {
    return thumb_.getOutlineThickness();
}

void SliderStyle::setThumbMinWidth(float thumbMinWidth) {
    thumbMinWidth_ = thumbMinWidth;
    gui_.requestRedraw();
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
void Slider::setRange(const std::pair<float, float>& range) {
    range_.first = std::min(range.first, range.second);
    range_.second = std::max(range.first, range.second);
    setValue(value_);
}
void Slider::setValue(float value) {
    value_ = std::min(std::max(value, range_.first), range_.second);
    requestRedraw();
}
void Slider::setStep(float step) {
    step_ = step;
}
const sf::Vector2f& Slider::getSize() const {
    return size_;
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

    if (isDragging_) {
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            // FIXME
        } else {
            isDragging_ = false;
        }
    }

    return isFocused();
}
void Slider::handleMousePress(sf::Mouse::Button button, const sf::Vector2f& mouseParent) {
    Widget::handleMousePress(button, mouseParent);
    if (button == sf::Mouse::Left) {
        isDragging_ = true;
    }
}
void Slider::handleMouseRelease(sf::Mouse::Button button, const sf::Vector2f& mouseParent) {
    if (button == sf::Mouse::Left) {
        isDragging_ = false;
    }
    Widget::handleMouseRelease(button, mouseParent);
}

Slider::Slider(std::shared_ptr<SliderStyle> style, const sf::String& name) :
    Widget(name),
    style_(style),
    styleCopied_(false),
    size_(0.0f, 0.0f),
    range_(0.0f, 1.0f),
    value_(0.0f),
    step_(0.0f),
    isDragging_(false) {
}

void Slider::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (!isVisible()) {
        return;
    }
    states.transform *= getTransform();

    float numThumbPositions;
    if (step_ == 0.0f) {
        numThumbPositions = size_.x;
    } else {
        numThumbPositions = std::round((range_.second - range_.first) / step_) + 1.0f;
    }

    style_->rect_.setSize(size_);
    target.draw(style_->rect_, states);
    style_->thumb_.setSize({std::max(size_.x / numThumbPositions, style_->thumbMinWidth_), size_.y});
    target.draw(style_->thumb_, states);
}

}
