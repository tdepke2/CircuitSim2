#include <gui/Gui.h>
#include <gui/Theme.h>
#include <gui/widgets/RadioButton.h>

namespace gui {

RadioButtonStyle::RadioButtonStyle(const Gui& gui) :
    Style(gui),
    circle_(0.0f, 4) {
}

// sf::Shape interface.
void RadioButtonStyle::setTexture(const sf::Texture* texture, bool resetRect) {
    circle_.setTexture(texture, resetRect);
    gui_.requestRedraw();
}
void RadioButtonStyle::setTextureRect(const sf::IntRect& rect) {
    circle_.setTextureRect(rect);
    gui_.requestRedraw();
}
void RadioButtonStyle::setFillColor(const sf::Color& color) {
    colorUnchecked_ = color;
    gui_.requestRedraw();
}
void RadioButtonStyle::setOutlineColor(const sf::Color& color) {
    circle_.setOutlineColor(color);
    gui_.requestRedraw();
}
void RadioButtonStyle::setOutlineThickness(float thickness) {
    circle_.setOutlineThickness(thickness);
    gui_.requestRedraw();
}
const sf::Texture* RadioButtonStyle::getTexture() const {
    return circle_.getTexture();
}
const sf::IntRect& RadioButtonStyle::getTextureRect() const {
    return circle_.getTextureRect();
}
const sf::Color& RadioButtonStyle::getFillColor() const {
    return colorUnchecked_;
}
const sf::Color& RadioButtonStyle::getOutlineColor() const {
    return circle_.getOutlineColor();
}
float RadioButtonStyle::getOutlineThickness() const {
    return circle_.getOutlineThickness();
}

// sf::Text interface.
void RadioButtonStyle::setFont(const sf::Font& font) {
    text_.setFont(font);
    gui_.requestRedraw();
}
void RadioButtonStyle::setCharacterSize(unsigned int size) {
    text_.setCharacterSize(size);
    gui_.requestRedraw();
}
void RadioButtonStyle::setLineSpacing(float spacingFactor) {
    text_.setLineSpacing(spacingFactor);
    gui_.requestRedraw();
}
void RadioButtonStyle::setLetterSpacing(float spacingFactor) {
    text_.setLetterSpacing(spacingFactor);
    gui_.requestRedraw();
}
void RadioButtonStyle::setTextStyle(uint32_t style) {
    text_.setStyle(style);
    gui_.requestRedraw();
}
void RadioButtonStyle::setTextFillColor(const sf::Color& color) {
    text_.setFillColor(color);
    gui_.requestRedraw();
}
const sf::Font* RadioButtonStyle::getFont() const {
    return text_.getFont();
}
unsigned int RadioButtonStyle::getCharacterSize() const {
    return text_.getCharacterSize();
}
float RadioButtonStyle::getLineSpacing() const {
    return text_.getLineSpacing();
}
float RadioButtonStyle::getLetterSpacing() const {
    return text_.getLetterSpacing();
}
uint32_t RadioButtonStyle::getTextStyle() const {
    return text_.getStyle();
}
const sf::Color& RadioButtonStyle::getTextFillColor() const {
    return text_.getFillColor();
}

void RadioButtonStyle::setFillColorChecked(const sf::Color& color) {
    colorChecked_ = color;
    gui_.requestRedraw();
}
void RadioButtonStyle::setTextPadding(const sf::Vector3f& padding) {
    textPadding_ = padding;
    gui_.requestRedraw();
}
const sf::Color& RadioButtonStyle::getFillColorChecked() const {
    return colorChecked_;
}
const sf::Vector3f& RadioButtonStyle::getTextPadding() const {
    return textPadding_;
}

std::shared_ptr<RadioButtonStyle> RadioButtonStyle::clone() const {
    return std::make_shared<RadioButtonStyle>(*this);
}



std::shared_ptr<RadioButton> RadioButton::create(const Theme& theme, const sf::String& name) {
    return std::shared_ptr<RadioButton>(new RadioButton(theme.getStyle<RadioButtonStyle>("RadioButton"), name));
}
std::shared_ptr<RadioButton> RadioButton::create(std::shared_ptr<RadioButtonStyle> style, const sf::String& name) {
    return std::shared_ptr<RadioButton>(new RadioButton(style, name));
}

void RadioButton::setChecked(bool checked) {
    Button::setPressed(checked);
    if (!checked || getParent() == nullptr) {
        return;
    }
    for (const auto& child : getParent()->getChildren()) {
        auto childButton = dynamic_cast<RadioButton*>(child.get());
        if (childButton != nullptr && childButton != this) {
            childButton->Button::setPressed(false);
        }
    }
}

bool RadioButton::isChecked() const {
    return Button::isPressed();
}

void RadioButton::uncheckRadioButtons() {
    if (getParent() == nullptr) {
        return;
    }
    for (const auto& child : getParent()->getChildren()) {
        auto childButton = dynamic_cast<RadioButton*>(child.get());
        if (childButton != nullptr) {
            childButton->Button::setPressed(false);
        }
    }
}

void RadioButton::setStyle(std::shared_ptr<RadioButtonStyle> style) {
    style_ = style;
    styleCopied_ = false;
    requestRedraw();
}
std::shared_ptr<RadioButtonStyle> RadioButton::getStyle() {
    if (!styleCopied_) {
        style_ = style_->clone();
        styleCopied_ = true;
    }
    return style_;
}

void RadioButton::handleMousePress(sf::Mouse::Button button, const sf::Vector2f& mouseParent) {
    Widget::handleMousePress(button, mouseParent);
    const auto mouseLocal = toLocalOriginSpace(mouseParent);
    if (button <= sf::Mouse::Middle) {
        setChecked(true);
        onClick.emit(this, mouseLocal);
    }
    onMousePress.emit(this, button, mouseLocal);
}
void RadioButton::handleMouseRelease(sf::Mouse::Button button, const sf::Vector2f& mouseParent) {
    onMouseRelease.emit(this, button, toLocalOriginSpace(mouseParent));
    Widget::handleMouseRelease(button, mouseParent);
}

void RadioButton::handleMouseEntered() {
    Widget::handleMouseEntered();
}

void RadioButton::handleMouseLeft() {
    Widget::handleMouseLeft();
}

RadioButton::RadioButton(std::shared_ptr<RadioButtonStyle> style, const sf::String& name) :
    Button(nullptr, name),
    style_(style),
    styleCopied_(false) {
}

void RadioButton::computeResize() const {
    style_->text_.setString(getLabel());
    const auto bounds = style_->text_.getLocalBounds();
    size_ = sf::Vector2f(
        2.0f * style_->textPadding_.x + bounds.left + bounds.width,
        2.0f * style_->textPadding_.y + style_->textPadding_.z * style_->getCharacterSize()
    );
    size_.x += size_.y;
}

void RadioButton::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (!isVisible()) {
        return;
    }
    states.transform *= getTransform();

    if (getAutoResize()) {
        computeResize();
    } else {
        style_->text_.setString(getLabel());
    }
    style_->circle_.setRadius(size_.y / 2.0f);
    if (Button::isPressed()) {
        style_->circle_.setFillColor(style_->colorChecked_);
    } else {
        style_->circle_.setFillColor(style_->colorUnchecked_);
    }
    target.draw(style_->circle_, states);
    style_->text_.setPosition(style_->textPadding_.x + static_cast<int>(size_.y), style_->textPadding_.y);
    target.draw(style_->text_, states);
}

}
