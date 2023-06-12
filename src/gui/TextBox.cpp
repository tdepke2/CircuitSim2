#include <gui/TextBox.h>
#include <gui/Theme.h>

#include <cmath>
#include <limits>







#include <iostream>




namespace gui {

// sf::Shape interface.
void TextBoxStyle::setTexture(const sf::Texture* texture, bool resetRect) {
    box_.setTexture(texture, resetRect);
}
void TextBoxStyle::setTextureRect(const sf::IntRect& rect) {
    box_.setTextureRect(rect);
}
void TextBoxStyle::setFillColor(const sf::Color& color) {
    box_.setFillColor(color);
}
void TextBoxStyle::setOutlineColor(const sf::Color& color) {
    box_.setOutlineColor(color);
}
void TextBoxStyle::setOutlineThickness(float thickness) {
    box_.setOutlineThickness(thickness);
}
const sf::Texture* TextBoxStyle::getTexture() const {
    return box_.getTexture();
}
const sf::IntRect& TextBoxStyle::getTextureRect() const {
    return box_.getTextureRect();
}
const sf::Color& TextBoxStyle::getFillColor() const {
    return box_.getFillColor();
}
const sf::Color& TextBoxStyle::getOutlineColor() const {
    return box_.getOutlineColor();
}
float TextBoxStyle::getOutlineThickness() const {
    return box_.getOutlineThickness();
}

// sf::Text interface.
void TextBoxStyle::setFont(const sf::Font& font) {
    text_.setFont(font);
}
void TextBoxStyle::setCharacterSize(unsigned int size) {
    text_.setCharacterSize(size);
}
void TextBoxStyle::setLineSpacing(float spacingFactor) {
    text_.setLineSpacing(spacingFactor);
}
void TextBoxStyle::setLetterSpacing(float spacingFactor) {
    text_.setLetterSpacing(spacingFactor);
}
void TextBoxStyle::setTextStyle(uint32_t style) {
    text_.setStyle(style);
}
void TextBoxStyle::setTextFillColor(const sf::Color& color) {
    text_.setFillColor(color);
}
const sf::Font* TextBoxStyle::getFont() const {
    return text_.getFont();
}
unsigned int TextBoxStyle::getCharacterSize() const {
    return text_.getCharacterSize();
}
float TextBoxStyle::getLineSpacing() const {
    return text_.getLineSpacing();
}
float TextBoxStyle::getLetterSpacing() const {
    return text_.getLetterSpacing();
}
uint32_t TextBoxStyle::getTextStyle() const {
    return text_.getStyle();
}
const sf::Color& TextBoxStyle::getTextFillColor() const {
    return text_.getFillColor();
}

void TextBoxStyle::setCaretSize(const sf::Vector2f& size) {
    caret_.setSize(size);
}
void TextBoxStyle::setCaretFillColor(const sf::Color& color) {
    caret_.setFillColor(color);
}
void TextBoxStyle::setTextPadding(const sf::Vector2f& padding) {
    textPadding_ = padding;
}
const sf::Vector2f& TextBoxStyle::getCaretSize() const {
    return caret_.getSize();
}
const sf::Color& TextBoxStyle::getCaretFillColor() const {
    return caret_.getFillColor();
}
const sf::Vector2f& TextBoxStyle::getTextPadding() const {
    return textPadding_;
}

std::shared_ptr<TextBoxStyle> TextBoxStyle::clone() const {
    return std::make_shared<TextBoxStyle>(*this);
}



std::shared_ptr<TextBox> TextBox::create(const Theme& theme) {
    return std::shared_ptr<TextBox>(new TextBox(theme.getTextBoxStyle()));
}
std::shared_ptr<TextBox> TextBox::create(std::shared_ptr<TextBoxStyle> style) {
    return std::shared_ptr<TextBox>(new TextBox(style));
}

void TextBox::setWidthCharacters(size_t widthCharacters) {
    widthCharacters_ = widthCharacters;

    std::string textBounds(widthCharacters, 'A');
    style_->text_.setString(textBounds);

    const auto bounds = style_->text_.getLocalBounds();
    size_ = sf::Vector2f(
        2.0f * (bounds.left + style_->textPadding_.x) + bounds.width,
        2.0f * (bounds.top + style_->textPadding_.y) + bounds.height
    );

    updateCaretPosition(0);
}
void TextBox::setMaxCharacters(size_t maxCharacters) {
    maxCharacters_ = maxCharacters;
}
void TextBox::setReadOnly(bool readOnly) {
    readOnly_ = readOnly;
}
void TextBox::setText(const sf::String& text) {
    boxString_ = text;
    updateCaretPosition(0);
}
const sf::Vector2f& TextBox::getSize() const {
    return size_;
}
size_t TextBox::getWidthCharacters() const {
    return widthCharacters_;
}
size_t TextBox::getMaxCharacters() const {
    return maxCharacters_;
}
bool TextBox::getReadOnly() const {
    return readOnly_;
}
const sf::String& TextBox::getText() const {
    return boxString_;
}

void TextBox::setStyle(std::shared_ptr<TextBoxStyle> style) {
    style_ = style;
    styleCopied_ = false;
}
std::shared_ptr<TextBoxStyle> TextBox::getStyle() {
    if (!styleCopied_) {
        style_ = style_->clone();
        styleCopied_ = true;
    }
    return style_;
}

sf::FloatRect TextBox::getLocalBounds() const {
    return {-getOrigin(), size_};
}
void TextBox::handleMousePress(sf::Mouse::Button button, const sf::Vector2f& mouseLocal) {
    if (!isEnabled()) {
        return;
    }
    const auto mouseWidgetLocal = toLocalSpace(mouseLocal);
    if (button <= sf::Mouse::Button::Middle) {
        onClick.emit(this, mouseWidgetLocal);
    }
    onMousePress.emit(this, button, mouseWidgetLocal);

    style_->text_.setString(visibleString_);
    style_->text_.setPosition(style_->textPadding_);
    size_t closestIndex = 0;
    float closestDistance = std::numeric_limits<float>::max();
    for (size_t i = 0; i <= visibleString_.getSize(); ++i) {
        float distance = fabs(mouseWidgetLocal.x + getOrigin().x - style_->text_.findCharacterPos(i).x);
        if (distance < closestDistance) {
            closestIndex = i;
            closestDistance = distance;
        }
    }
    std::cout << "mouseWidgetLocal.x = " << mouseWidgetLocal.x << ", closest = " << closestIndex << ", " << closestDistance << "\n";
    updateCaretPosition(closestIndex + horizontalScroll_);

    Widget::handleMousePress(button, mouseLocal);
}
void TextBox::handleMouseRelease(sf::Mouse::Button button, const sf::Vector2f& mouseLocal) {
    if (!isEnabled()) {
        return;
    }
    onMouseRelease.emit(this, button, toLocalSpace(mouseLocal));
    Widget::handleMouseRelease(button, mouseLocal);
}
void TextBox::handleTextEntered(uint32_t unicode) {
    if (readOnly_) {
        return;
    }
    if (unicode == '\u0008') {    // Backspace.
        if (caretPosition_ > 0) {
            if (horizontalScroll_ > 0 && horizontalScroll_ + widthCharacters_ >= boxString_.getSize()) {
                --horizontalScroll_;
            }
            boxString_.erase(caretPosition_ - 1, 1);
            updateCaretPosition(caretPosition_ - 1);
        }
    } else if (unicode >= '\u0020' && unicode <= '\u007e') {    // Printable character.
        if (maxCharacters_ == 0 || boxString_.getSize() < maxCharacters_) {
            boxString_.insert(caretPosition_, sf::String(unicode));
            updateCaretPosition(caretPosition_ + 1);
        }
    } else if (unicode == '\u007f') {    // Delete.
        if (caretPosition_ < boxString_.getSize() && boxString_.getSize() > 0) {
            if (horizontalScroll_ > 0 && horizontalScroll_ + widthCharacters_ >= boxString_.getSize()) {
                --horizontalScroll_;
            }
            boxString_.erase(caretPosition_, 1);
            updateCaretPosition(caretPosition_);
        }
    }
}
void TextBox::handleKeyPressed(sf::Keyboard::Key key) {
    if (key == sf::Keyboard::Enter) {
        onEnterPressed.emit(this, boxString_);
    } else if (key == sf::Keyboard::End && caretPosition_ != boxString_.getSize()) {
        updateCaretPosition(boxString_.getSize());
    } else if (key == sf::Keyboard::Home && caretPosition_ != 0) {
        updateCaretPosition(0);
    } else if (key == sf::Keyboard::Left) {
        if (readOnly_ && horizontalScroll_ > 0) {
            updateCaretPosition(horizontalScroll_ - 1);
        } else if (!readOnly_ && caretPosition_ > 0) {
            updateCaretPosition(caretPosition_ - 1);
        }
    } else if (key == sf::Keyboard::Right) {
        if (readOnly_ && horizontalScroll_ + widthCharacters_ < boxString_.getSize()) {
            updateCaretPosition(horizontalScroll_ + widthCharacters_ + 1);
        } else if (!readOnly_ && caretPosition_ < boxString_.getSize()) {
            updateCaretPosition(caretPosition_ + 1);
        }
    }
}

TextBox::TextBox(std::shared_ptr<TextBoxStyle> style) :
    style_(style),
    styleCopied_(false),
    widthCharacters_(0),
    maxCharacters_(0),
    readOnly_(false),
    horizontalScroll_(0) {

    updateCaretPosition(0);
}

void TextBox::updateCaretPosition(size_t caretPosition) {
    caretPosition_ = caretPosition;
    if (caretPosition < horizontalScroll_) {
        horizontalScroll_ = caretPosition;
    } else if (caretPosition > horizontalScroll_ + widthCharacters_) {
        horizontalScroll_ = caretPosition - widthCharacters_;
    }
    visibleString_ = boxString_.substring(horizontalScroll_, widthCharacters_);

    style_->text_.setString(visibleString_);
    style_->text_.setPosition(style_->textPadding_);
    caretDrawPosition_ = style_->text_.findCharacterPos(caretPosition - horizontalScroll_);

    std::cout << "TextBox::updateCaretPosition(), caretPosition = " << caretPosition << ", horizontalScroll_ = " << horizontalScroll_ << "\n";
}

void TextBox::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (!isVisible()) {
        return;
    }
    states.transform *= getTransform();

    style_->box_.setSize(size_);
    target.draw(style_->box_, states);
    style_->text_.setString(visibleString_);
    style_->text_.setPosition(style_->textPadding_);
    target.draw(style_->text_, states);
    if (isFocused() && !readOnly_) {
        style_->caret_.setPosition(caretDrawPosition_);
        target.draw(style_->caret_, states);
    }
}

}
