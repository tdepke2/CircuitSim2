#include <gui/Gui.h>
#include <gui/Theme.h>
#include <gui/widgets/Label.h>

namespace gui {

LabelStyle::LabelStyle(const Gui& gui) :
    gui_(gui) {
}

// sf::Text interface.
void LabelStyle::setFont(const sf::Font& font) {
    text_.setFont(font);
    gui_.requestRedraw();
}
void LabelStyle::setCharacterSize(unsigned int size) {
    text_.setCharacterSize(size);
    gui_.requestRedraw();
}
void LabelStyle::setLineSpacing(float spacingFactor) {
    text_.setLineSpacing(spacingFactor);
    gui_.requestRedraw();
}
void LabelStyle::setLetterSpacing(float spacingFactor) {
    text_.setLetterSpacing(spacingFactor);
    gui_.requestRedraw();
}
void LabelStyle::setTextStyle(uint32_t style) {
    text_.setStyle(style);
    gui_.requestRedraw();
}
void LabelStyle::setTextFillColor(const sf::Color& color) {
    text_.setFillColor(color);
    gui_.requestRedraw();
}
const sf::Font* LabelStyle::getFont() const {
    return text_.getFont();
}
unsigned int LabelStyle::getCharacterSize() const {
    return text_.getCharacterSize();
}
float LabelStyle::getLineSpacing() const {
    return text_.getLineSpacing();
}
float LabelStyle::getLetterSpacing() const {
    return text_.getLetterSpacing();
}
uint32_t LabelStyle::getTextStyle() const {
    return text_.getStyle();
}
const sf::Color& LabelStyle::getTextFillColor() const {
    return text_.getFillColor();
}

void LabelStyle::setTextPadding(const sf::Vector3f& padding) {
    textPadding_ = padding;
    gui_.requestRedraw();
}
const sf::Vector3f& LabelStyle::getTextPadding() const {
    return textPadding_;
}

std::shared_ptr<LabelStyle> LabelStyle::clone() const {
    return std::make_shared<LabelStyle>(*this);
}



std::shared_ptr<Label> Label::create(const Theme& theme) {
    return std::shared_ptr<Label>(new Label(theme.getLabelStyle()));
}
std::shared_ptr<Label> Label::create(std::shared_ptr<LabelStyle> style) {
    return std::shared_ptr<Label>(new Label(style));
}

void Label::setLabel(const sf::String& label) {
    label_ = label;
    computeResize();
    requestRedraw();
}
const sf::Vector2f& Label::getSize() const {
    return size_;
}
const sf::String& Label::getLabel() const {
    return label_;
}

void Label::setStyle(std::shared_ptr<LabelStyle> style) {
    style_ = style;
    styleCopied_ = false;
    requestRedraw();
}
std::shared_ptr<LabelStyle> Label::getStyle() {
    if (!styleCopied_) {
        style_ = style_->clone();
        styleCopied_ = true;
    }
    return style_;
}

sf::FloatRect Label::getLocalBounds() const {
    return {-getOrigin(), size_};
}

Label::Label(std::shared_ptr<LabelStyle> style) :
    style_(style),
    styleCopied_(false) {
}

void Label::computeResize() const {
    style_->text_.setString(label_);
    const auto bounds = style_->text_.getLocalBounds();
    size_ = sf::Vector2f(
        2.0f * style_->textPadding_.x + bounds.left + bounds.width,
        2.0f * style_->textPadding_.y + style_->textPadding_.z * style_->getCharacterSize()
    );
}

void Label::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (!isVisible()) {
        return;
    }
    states.transform *= getTransform();

    computeResize();
    style_->text_.setPosition(style_->textPadding_.x, style_->textPadding_.y);
    target.draw(style_->text_, states);
}

}
