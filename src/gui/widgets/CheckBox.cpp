#include <gui/Gui.h>
#include <gui/Theme.h>
#include <gui/widgets/CheckBox.h>

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

CheckBoxStyle::CheckBoxStyle(const Gui& gui) :
    Style(gui) {
}

// sf::Shape interface.
void CheckBoxStyle::setTexture(const sf::Texture* texture, bool resetRect) {
    rect_.setTexture(texture, resetRect);
    gui_.requestRedraw();
}
void CheckBoxStyle::setTextureRect(const sf::IntRect& rect) {
    rect_.setTextureRect(rect);
    gui_.requestRedraw();
}
void CheckBoxStyle::setFillColor(const sf::Color& color) {
    colorUnchecked_ = color;
    gui_.requestRedraw();
}
void CheckBoxStyle::setOutlineColor(const sf::Color& color) {
    rect_.setOutlineColor(color);
    gui_.requestRedraw();
}
void CheckBoxStyle::setOutlineThickness(float thickness) {
    rect_.setOutlineThickness(thickness);
    gui_.requestRedraw();
}
const sf::Texture* CheckBoxStyle::getTexture() const {
    return rect_.getTexture();
}
const sf::IntRect& CheckBoxStyle::getTextureRect() const {
    return rect_.getTextureRect();
}
const sf::Color& CheckBoxStyle::getFillColor() const {
    return colorUnchecked_;
}
const sf::Color& CheckBoxStyle::getOutlineColor() const {
    return rect_.getOutlineColor();
}
float CheckBoxStyle::getOutlineThickness() const {
    return rect_.getOutlineThickness();
}

// sf::Text interface.
void CheckBoxStyle::setFont(const sf::Font& font) {
    text_.setFont(font);
    gui_.requestRedraw();
}
void CheckBoxStyle::setCharacterSize(unsigned int size) {
    text_.setCharacterSize(size);
    gui_.requestRedraw();
}
void CheckBoxStyle::setLineSpacing(float spacingFactor) {
    text_.setLineSpacing(spacingFactor);
    gui_.requestRedraw();
}
void CheckBoxStyle::setLetterSpacing(float spacingFactor) {
    text_.setLetterSpacing(spacingFactor);
    gui_.requestRedraw();
}
void CheckBoxStyle::setTextStyle(uint32_t style) {
    text_.setStyle(style);
    gui_.requestRedraw();
}
void CheckBoxStyle::setTextFillColor(const sf::Color& color) {
    text_.setFillColor(color);
    gui_.requestRedraw();
}
const sf::Font* CheckBoxStyle::getFont() const {
    return text_.getFont();
}
unsigned int CheckBoxStyle::getCharacterSize() const {
    return text_.getCharacterSize();
}
float CheckBoxStyle::getLineSpacing() const {
    return text_.getLineSpacing();
}
float CheckBoxStyle::getLetterSpacing() const {
    return text_.getLetterSpacing();
}
uint32_t CheckBoxStyle::getTextStyle() const {
    return text_.getStyle();
}
const sf::Color& CheckBoxStyle::getTextFillColor() const {
    return text_.getFillColor();
}

void CheckBoxStyle::setFillColorHover(const sf::Color& color) {
    colorHover_ = color;
    gui_.requestRedraw();
}
void CheckBoxStyle::setFillColorChecked(const sf::Color& color) {
    colorChecked_ = color;
    gui_.requestRedraw();
}
void CheckBoxStyle::setTextPadding(const sf::Vector3f& padding) {
    textPadding_ = padding;
    gui_.requestRedraw();
}
const sf::Color& CheckBoxStyle::getFillColorHover() const {
    return colorHover_;
}
const sf::Color& CheckBoxStyle::getFillColorChecked() const {
    return colorChecked_;
}
const sf::Vector3f& CheckBoxStyle::getTextPadding() const {
    return textPadding_;
}

std::shared_ptr<CheckBoxStyle> CheckBoxStyle::clone() const {
    return std::make_shared<CheckBoxStyle>(*this);
}



std::shared_ptr<CheckBox> CheckBox::create(const Theme& theme, const sf::String& name) {
    return std::shared_ptr<CheckBox>(new CheckBox(theme.getStyle<CheckBoxStyle>("CheckBox"), name));
}
std::shared_ptr<CheckBox> CheckBox::create(std::shared_ptr<CheckBoxStyle> style, const sf::String& name) {
    return std::shared_ptr<CheckBox>(new CheckBox(style, name));
}

void CheckBox::setChecked(bool checked) {
    if (isChecked_ != checked) {
        isChecked_ = checked;
        requestRedraw();
    }
}

bool CheckBox::isChecked() const {
    return isChecked_;
}

void CheckBox::setStyle(std::shared_ptr<CheckBoxStyle> style) {
    style_ = style;
    styleCopied_ = false;
    requestRedraw();
}
std::shared_ptr<CheckBoxStyle> CheckBox::getStyle() {
    if (!styleCopied_) {
        style_ = style_->clone();
        styleCopied_ = true;
    }
    return style_;
}

void CheckBox::handleMouseRelease(sf::Mouse::Button button, const sf::Vector2f& mouseParent) {
    if (button <= sf::Mouse::Middle && Button::isPressed()) {
        setChecked(!isChecked_);
    }
    Button::handleMouseRelease(button, mouseParent);
}

CheckBox::CheckBox(std::shared_ptr<CheckBoxStyle> style, const sf::String& name) :
    Button(nullptr, name),
    style_(style),
    styleCopied_(false),
    isChecked_(false) {
}

void CheckBox::computeResize() const {
    style_->text_.setString(getLabel());
    const auto bounds = style_->text_.getLocalBounds();
    size_ = sf::Vector2f(
        2.0f * style_->textPadding_.x + bounds.left + bounds.width,
        2.0f * style_->textPadding_.y + style_->textPadding_.z * style_->getCharacterSize()
    );
    size_.x += size_.y;
}

void CheckBox::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (!isVisible()) {
        return;
    }
    states.transform *= getTransform();

    if (getAutoResize()) {
        computeResize();
    } else {
        style_->text_.setString(getLabel());
    }
    style_->rect_.setSize({size_.y, size_.y});
    sf::Color fillColor = (isChecked_ ? style_->colorChecked_ : style_->colorUnchecked_);
    if (isMouseHovering()) {
        fillColor = blendColors(style_->colorHover_, fillColor);
    }
    style_->rect_.setFillColor(fillColor);
    target.draw(style_->rect_, states);
    style_->text_.setPosition(style_->textPadding_.x + static_cast<int>(size_.y), style_->textPadding_.y);
    target.draw(style_->text_, states);
}

}
