#include "SFML/System/Vector2.hpp"
#include <gui/Button.h>
#include <gui/Theme.h>

namespace gui {


ButtonStyle::ButtonStyle() {
    // FIXME: set default style here?
}

// sf::Shape interface.
void ButtonStyle::setTexture(const sf::Texture* texture, bool resetRect) {
    rect_.setTexture(texture, resetRect);
}
void ButtonStyle::setTextureRect(const sf::IntRect& rect) {
    rect_.setTextureRect(rect);
}
void ButtonStyle::setFillColor(const sf::Color& color) {
    colorUp_ = color;
}
void ButtonStyle::setOutlineColor(const sf::Color& color) {
    rect_.setOutlineColor(color);
}
void ButtonStyle::setOutlineThickness(float thickness) {
    rect_.setOutlineThickness(thickness);
}
const sf::Texture* ButtonStyle::getTexture() const {
    return rect_.getTexture();
}
const sf::IntRect& ButtonStyle::getTextureRect() const {
    return rect_.getTextureRect();
}
const sf::Color& ButtonStyle::getFillColor() const {
    return colorUp_;
}
const sf::Color& ButtonStyle::getOutlineColor() const {
    return rect_.getOutlineColor();
}
float ButtonStyle::getOutlineThickness() const {
    return rect_.getOutlineThickness();
}

// sf::Text interface.
void ButtonStyle::setFont(const sf::Font& font) {
    text_.setFont(font);
}
void ButtonStyle::setCharacterSize(unsigned int size) {
    text_.setCharacterSize(size);
}
void ButtonStyle::setLineSpacing(float spacingFactor) {
    text_.setLineSpacing(spacingFactor);
}
void ButtonStyle::setLetterSpacing(float spacingFactor) {
    text_.setLetterSpacing(spacingFactor);
}
void ButtonStyle::setTextStyle(uint32_t style) {
    text_.setStyle(style);
}
void ButtonStyle::setTextFillColor(const sf::Color& color) {
    text_.setFillColor(color);
}
const sf::Font* ButtonStyle::getFont() const {
    return text_.getFont();
}
unsigned int ButtonStyle::getCharacterSize() const {
    return text_.getCharacterSize();
}
float ButtonStyle::getLineSpacing() const {
    return text_.getLineSpacing();
}
float ButtonStyle::getLetterSpacing() const {
    return text_.getLetterSpacing();
}
uint32_t ButtonStyle::getTextStyle() const {
    return text_.getStyle();
}
const sf::Color& ButtonStyle::getTextFillColor() const {
    return text_.getFillColor();
}

void ButtonStyle::setFillColorDown(const sf::Color& color) {
    colorDown_ = color;
}
void ButtonStyle::setTextPadding(float padding) {
    textPadding_ = padding;
}
const sf::Color& ButtonStyle::getFillColorDown() const {
    return colorDown_;
}
float ButtonStyle::getTextPadding() const {
    return textPadding_;
}

std::shared_ptr<ButtonStyle> ButtonStyle::clone() const {
    auto style = std::make_shared<ButtonStyle>();
    style->rect_ = rect_;
    style->text_ = text_;
    style->colorUp_ = colorUp_;
    style->colorDown_ = colorDown_;
    style->textPadding_ = textPadding_;

    // FIXME better to just pass through ctor instead?

    return style;
}



std::shared_ptr<Button> Button::create(std::shared_ptr<Theme> theme) {
    return std::shared_ptr<Button>(new Button(theme));
}
std::shared_ptr<Button> Button::create(std::shared_ptr<ButtonStyle> style) {
    return std::shared_ptr<Button>(new Button(style));
}

void Button::setSize(const sf::Vector2f& size) {
    size_ = size;
}
void Button::setLabel(const sf::String& label) {
    label_ = label;
}
const sf::Vector2f& Button::getSize() const {
    return size_;
}
const sf::String& Button::getLabel() const {
    return label_;
}

void Button::setStyle(std::shared_ptr<ButtonStyle> style) {
    style_ = style;
    styleCopied_ = false;
}
std::shared_ptr<ButtonStyle> Button::getStyle() {
    if (!styleCopied_) {
        style_ = style_->clone();
        styleCopied_ = true;
    }
    return style_;
}

Button::Button(std::shared_ptr<Theme> theme) {
    style_ = theme->getButtonStyle();
}
Button::Button(std::shared_ptr<ButtonStyle> style) {
    style_ = style;
}

void Button::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    states.transform *= getTransform();
    
    style_->rect_.setSize(size_);
    target.draw(style_->rect_, states);
    style_->text_.setString(label_);
    target.draw(style_->text_, states);
}

}
