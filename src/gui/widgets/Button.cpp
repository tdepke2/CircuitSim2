#include <gui/Gui.h>
#include <gui/Theme.h>
#include <gui/widgets/Button.h>

namespace gui {

ButtonStyle::ButtonStyle(const Gui& gui) :
    Style(gui) {
}

// sf::Shape interface.
void ButtonStyle::setTexture(const sf::Texture* texture, bool resetRect) {
    rect_.setTexture(texture, resetRect);
    gui_.requestRedraw();
}
void ButtonStyle::setTextureRect(const sf::IntRect& rect) {
    rect_.setTextureRect(rect);
    gui_.requestRedraw();
}
void ButtonStyle::setFillColor(const sf::Color& color) {
    colorUp_ = color;
    gui_.requestRedraw();
}
void ButtonStyle::setOutlineColor(const sf::Color& color) {
    rect_.setOutlineColor(color);
    gui_.requestRedraw();
}
void ButtonStyle::setOutlineThickness(float thickness) {
    rect_.setOutlineThickness(thickness);
    gui_.requestRedraw();
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
    gui_.requestRedraw();
}
void ButtonStyle::setCharacterSize(unsigned int size) {
    text_.setCharacterSize(size);
    gui_.requestRedraw();
}
void ButtonStyle::setLineSpacing(float spacingFactor) {
    text_.setLineSpacing(spacingFactor);
    gui_.requestRedraw();
}
void ButtonStyle::setLetterSpacing(float spacingFactor) {
    text_.setLetterSpacing(spacingFactor);
    gui_.requestRedraw();
}
void ButtonStyle::setTextStyle(uint32_t style) {
    text_.setStyle(style);
    gui_.requestRedraw();
}
void ButtonStyle::setTextFillColor(const sf::Color& color) {
    text_.setFillColor(color);
    gui_.requestRedraw();
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

void ButtonStyle::setFillColorHover(const sf::Color& color) {
    colorHover_ = color;
    gui_.requestRedraw();
}
void ButtonStyle::setFillColorDown(const sf::Color& color) {
    colorDown_ = color;
    gui_.requestRedraw();
}
void ButtonStyle::setTextPadding(const sf::Vector3f& padding) {
    textPadding_ = padding;
    gui_.requestRedraw();
}
const sf::Color& ButtonStyle::getFillColorHover() const {
    return colorHover_;
}
const sf::Color& ButtonStyle::getFillColorDown() const {
    return colorDown_;
}
const sf::Vector3f& ButtonStyle::getTextPadding() const {
    return textPadding_;
}

std::shared_ptr<ButtonStyle> ButtonStyle::clone() const {
    return std::make_shared<ButtonStyle>(*this);
}



std::shared_ptr<Button> Button::create(const Theme& theme, const sf::String& name) {
    return std::shared_ptr<Button>(new Button(theme.getStyle<ButtonStyle>("Button"), name));
}
std::shared_ptr<Button> Button::create(std::shared_ptr<ButtonStyle> style, const sf::String& name) {
    return std::shared_ptr<Button>(new Button(style, name));
}

void Button::setSize(const sf::Vector2f& size) {
    size_ = size;
    autoResize_ = false;
    requestRedraw();
}
void Button::setLabel(const sf::String& label) {
    label_ = label;
    if (autoResize_) {
        computeResize();
    }
    requestRedraw();
}
void Button::setAutoResize(bool autoResize) {
    autoResize_ = autoResize;
}
void Button::setPressed(bool pressed) {
    if (isPressed_ != pressed) {
        isPressed_ = pressed;
        requestRedraw();
    }
}
const sf::Vector2f& Button::getSize() const {
    return size_;
}
const sf::String& Button::getLabel() const {
    return label_;
}
bool Button::getAutoResize() const {
    return autoResize_;
}
bool Button::isPressed() const {
    return isPressed_;
}

void Button::setStyle(std::shared_ptr<ButtonStyle> style) {
    style_ = style;
    styleCopied_ = false;
    requestRedraw();
}
std::shared_ptr<ButtonStyle> Button::getStyle() {
    if (!styleCopied_) {
        style_ = style_->clone();
        styleCopied_ = true;
    }
    return style_;
}

sf::FloatRect Button::getLocalBounds() const {
    return {-getOrigin(), size_};
}
void Button::handleMousePress(sf::Mouse::Button button, const sf::Vector2f& mouseParent) {
    baseClass::handleMousePress(button, mouseParent);
    const auto mouseLocal = toLocalOriginSpace(mouseParent);
    if (button <= sf::Mouse::Middle) {
        setPressed(true);
    }
    onMousePress.emit(this, button, mouseLocal);
}
void Button::handleMouseRelease(sf::Mouse::Button button, const sf::Vector2f& mouseParent) {
    const auto mouseLocal = toLocalOriginSpace(mouseParent);
    if (button <= sf::Mouse::Middle && isPressed_) {
        setPressed(false);
        onClick.emit(this, mouseLocal);
    }
    onMouseRelease.emit(this, button, mouseLocal);
    baseClass::handleMouseRelease(button, mouseParent);
}

void Button::handleMouseEntered() {
    baseClass::handleMouseEntered();
    requestRedraw();
}

void Button::handleMouseLeft() {
    requestRedraw();
    setPressed(false);
    baseClass::handleMouseLeft();
}

Button::Button(std::shared_ptr<ButtonStyle> style, const sf::String& name) :
    baseClass(name),
    style_(style),
    styleCopied_(false),
    autoResize_(true),
    isPressed_(false) {
}

void Button::computeResize() const {
    style_->text_.setString(label_);
    const auto bounds = style_->text_.getLocalBounds();
    size_ = sf::Vector2f(
        2.0f * style_->textPadding_.x + bounds.left + bounds.width,
        2.0f * style_->textPadding_.y + style_->textPadding_.z * style_->getCharacterSize()
    );
}

void Button::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (!isVisible()) {
        return;
    }
    states.transform *= getTransform();

    if (autoResize_) {
        computeResize();
    } else {
        style_->text_.setString(label_);
    }
    style_->rect_.setSize(size_);
    if (isPressed_) {
        style_->rect_.setFillColor(style_->colorDown_);
    } else if (isMouseHovering()) {
        style_->rect_.setFillColor(Style::blendColors(style_->colorHover_, style_->colorUp_));
    } else {
        style_->rect_.setFillColor(style_->colorUp_);
    }
    target.draw(style_->rect_, states);
    style_->text_.setPosition(style_->textPadding_.x, style_->textPadding_.y);
    target.draw(style_->text_, states);
}

}
