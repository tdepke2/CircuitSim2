#include <gui/Gui.h>
#include <gui/Theme.h>
#include <gui/widgets/TextBox.h>

#include <cmath>
#include <limits>







#include <iostream>




namespace gui {

TextBoxStyle::TextBoxStyle(const Gui& gui) :
    gui_(gui) {
}

// sf::Shape interface.
void TextBoxStyle::setTexture(const sf::Texture* texture, bool resetRect) {
    box_.setTexture(texture, resetRect);
    gui_.requestRedraw();
}
void TextBoxStyle::setTextureRect(const sf::IntRect& rect) {
    box_.setTextureRect(rect);
    gui_.requestRedraw();
}
void TextBoxStyle::setFillColor(const sf::Color& color) {
    boxColor_ = color;
    gui_.requestRedraw();
}
void TextBoxStyle::setOutlineColor(const sf::Color& color) {
    box_.setOutlineColor(color);
    gui_.requestRedraw();
}
void TextBoxStyle::setOutlineThickness(float thickness) {
    box_.setOutlineThickness(thickness);
    gui_.requestRedraw();
}
const sf::Texture* TextBoxStyle::getTexture() const {
    return box_.getTexture();
}
const sf::IntRect& TextBoxStyle::getTextureRect() const {
    return box_.getTextureRect();
}
const sf::Color& TextBoxStyle::getFillColor() const {
    return boxColor_;
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
    gui_.requestRedraw();
}
void TextBoxStyle::setCharacterSize(unsigned int size) {
    text_.setCharacterSize(size);
    gui_.requestRedraw();
}
void TextBoxStyle::setLineSpacing(float spacingFactor) {
    text_.setLineSpacing(spacingFactor);
    gui_.requestRedraw();
}
void TextBoxStyle::setLetterSpacing(float spacingFactor) {
    text_.setLetterSpacing(spacingFactor);
    gui_.requestRedraw();
}
void TextBoxStyle::setTextStyle(uint32_t style) {
    text_.setStyle(style);
    gui_.requestRedraw();
}
void TextBoxStyle::setTextFillColor(const sf::Color& color) {
    textColor_ = color;
    gui_.requestRedraw();
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
    return textColor_;
}

void TextBoxStyle::setReadOnlyFillColor(const sf::Color& color) {
    readOnlyBoxColor_ = color;
    gui_.requestRedraw();
}
void TextBoxStyle::setDefaultTextFillColor(const sf::Color& color) {
    defaultTextColor_ = color;
    gui_.requestRedraw();
}
void TextBoxStyle::setCaretSize(const sf::Vector2f& size) {
    caret_.setSize(size);
    gui_.requestRedraw();
}
void TextBoxStyle::setCaretFillColor(const sf::Color& color) {
    caret_.setFillColor(color);
    gui_.requestRedraw();
}
void TextBoxStyle::setTextPadding(const sf::Vector3f& padding) {
    textPadding_ = padding;
    gui_.requestRedraw();
}
const sf::Color& TextBoxStyle::getReadOnlyFillColor() const {
    return readOnlyBoxColor_;
}
const sf::Color& TextBoxStyle::getDefaultTextFillColor() const {
    return defaultTextColor_;
}
const sf::Vector2f& TextBoxStyle::getCaretSize() const {
    return caret_.getSize();
}
const sf::Color& TextBoxStyle::getCaretFillColor() const {
    return caret_.getFillColor();
}
const sf::Vector3f& TextBoxStyle::getTextPadding() const {
    return textPadding_;
}

std::shared_ptr<TextBoxStyle> TextBoxStyle::clone() const {
    return std::make_shared<TextBoxStyle>(*this);
}



std::shared_ptr<TextBox> TextBox::create(const Theme& theme, const sf::String& name) {
    return std::shared_ptr<TextBox>(new TextBox(theme.getTextBoxStyle(), name));
}
std::shared_ptr<TextBox> TextBox::create(std::shared_ptr<TextBoxStyle> style, const sf::String& name) {
    return std::shared_ptr<TextBox>(new TextBox(style, name));
}

void TextBox::setWidthCharacters(size_t widthCharacters) {
    widthCharacters_ = widthCharacters;

    std::string textBounds(widthCharacters, 'A');
    style_->text_.setString(textBounds);

    const auto bounds = style_->text_.getLocalBounds();
    size_ = sf::Vector2f(
        2.0f * style_->textPadding_.x + bounds.left + bounds.width,
        2.0f * style_->textPadding_.y + style_->textPadding_.z * style_->getCharacterSize()
    );

    updateCaretPosition(0);
}
void TextBox::setMaxCharacters(size_t maxCharacters) {
    maxCharacters_ = maxCharacters;
}
void TextBox::setReadOnly(bool readOnly) {
    readOnly_ = readOnly;
    requestRedraw();
}
void TextBox::setText(const sf::String& text) {
    boxString_ = text;
    updateCaretPosition(0);
    onTextChange.emit(this, boxString_);
}
void TextBox::setDefaultText(const sf::String& text) {
    defaultString_ = text;
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
const sf::String& TextBox::getDefaultText() const {
    return defaultString_;
}

void TextBox::setStyle(std::shared_ptr<TextBoxStyle> style) {
    style_ = style;
    styleCopied_ = false;
    requestRedraw();
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
void TextBox::handleMousePress(sf::Mouse::Button button, const sf::Vector2f& mouseParent) {
    Widget::handleMousePress(button, mouseParent);
    const auto mouseLocal = toLocalOriginSpace(mouseParent);
    if (button <= sf::Mouse::Middle) {
        onClick.emit(this, mouseLocal);
    }
    onMousePress.emit(this, button, mouseLocal);

    style_->text_.setString(visibleString_);
    style_->text_.setPosition(style_->textPadding_.x, style_->textPadding_.y);
    size_t closestIndex = 0;
    float closestDistance = std::numeric_limits<float>::max();
    for (size_t i = 0; i <= visibleString_.getSize(); ++i) {
        float distance = fabs(mouseLocal.x + getOrigin().x - style_->text_.findCharacterPos(i).x);
        if (distance < closestDistance) {
            closestIndex = i;
            closestDistance = distance;
        }
    }
    std::cout << "mouseLocal.x = " << mouseLocal.x << ", closest = " << closestIndex << ", " << closestDistance << "\n";
    updateCaretPosition(closestIndex + horizontalScroll_);
}
void TextBox::handleMouseRelease(sf::Mouse::Button button, const sf::Vector2f& mouseParent) {
    onMouseRelease.emit(this, button, toLocalOriginSpace(mouseParent));
    Widget::handleMouseRelease(button, mouseParent);
}
bool TextBox::handleTextEntered(uint32_t unicode) {
    bool eventConsumed = Widget::handleTextEntered(unicode);
    if (!readOnly_ && unicode >= '\u0020' && unicode != '\u007f') {    // Printable character.
        if (maxCharacters_ == 0 || boxString_.getSize() < maxCharacters_) {
            boxString_.insert(caretPosition_, sf::String(unicode));
            updateCaretPosition(caretPosition_ + 1);
            onTextChange.emit(this, boxString_);
        }
        return true;
    }
    return eventConsumed;
}
bool TextBox::handleKeyPressed(const sf::Event::KeyEvent& key) {
    bool eventConsumed = Widget::handleKeyPressed(key);
    if (key.code == sf::Keyboard::Enter) {
        onEnterPressed.emit(this, boxString_);
        return eventConsumed;
    } else if (key.code == sf::Keyboard::Backspace) {
        if (!readOnly_ && caretPosition_ > 0) {
            if (horizontalScroll_ > 0 && horizontalScroll_ + widthCharacters_ >= boxString_.getSize()) {
                --horizontalScroll_;
            }
            boxString_.erase(caretPosition_ - 1, 1);
            updateCaretPosition(caretPosition_ - 1);
            onTextChange.emit(this, boxString_);
        }
    } else if (key.code == sf::Keyboard::Delete) {
        if (!readOnly_ && caretPosition_ < boxString_.getSize() && boxString_.getSize() > 0) {
            if (horizontalScroll_ > 0 && horizontalScroll_ + widthCharacters_ >= boxString_.getSize()) {
                --horizontalScroll_;
            }
            boxString_.erase(caretPosition_, 1);
            updateCaretPosition(caretPosition_);
            onTextChange.emit(this, boxString_);
        }
    } else if (key.code == sf::Keyboard::End && caretPosition_ != boxString_.getSize()) {
        updateCaretPosition(boxString_.getSize());
    } else if (key.code == sf::Keyboard::Home && caretPosition_ != 0) {
        updateCaretPosition(0);
    } else if (key.code == sf::Keyboard::Left) {
        if (readOnly_ && horizontalScroll_ > 0) {
            updateCaretPosition(horizontalScroll_ - 1);
        } else if (!readOnly_ && caretPosition_ > 0) {
            updateCaretPosition(caretPosition_ - 1);
        }
    } else if (key.code == sf::Keyboard::Right) {
        if (readOnly_ && horizontalScroll_ + widthCharacters_ < boxString_.getSize()) {
            updateCaretPosition(horizontalScroll_ + widthCharacters_ + 1);
        } else if (!readOnly_ && caretPosition_ < boxString_.getSize()) {
            updateCaretPosition(caretPosition_ + 1);
        }
    } else {
        return eventConsumed;
    }
    return true;
}

TextBox::TextBox(std::shared_ptr<TextBoxStyle> style, const sf::String& name) :
    Widget(name),
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
    style_->text_.setPosition(style_->textPadding_.x, style_->textPadding_.y);
    caretDrawPosition_ = style_->text_.findCharacterPos(caretPosition - horizontalScroll_);

    std::cout << "TextBox::updateCaretPosition(), caretPosition = " << caretPosition << ", horizontalScroll_ = " << horizontalScroll_ << "\n";
    requestRedraw();
}

void TextBox::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (!isVisible()) {
        return;
    }
    states.transform *= getTransform();

    style_->box_.setSize(size_);
    style_->box_.setFillColor(readOnly_ ? style_->readOnlyBoxColor_ : style_->boxColor_);
    target.draw(style_->box_, states);
    if (boxString_.isEmpty() && !defaultString_.isEmpty() && (!isFocused() || readOnly_)) {
        style_->text_.setString(defaultString_.substring(0, widthCharacters_));
        style_->text_.setFillColor(style_->defaultTextColor_);
    } else {
        style_->text_.setString(visibleString_);
        style_->text_.setFillColor(style_->textColor_);
    }
    if (readOnly_) {
        style_->text_.setFillColor(style_->defaultTextColor_);
    }
    style_->text_.setPosition(style_->textPadding_.x, style_->textPadding_.y);
    target.draw(style_->text_, states);
    if (isFocused() && !readOnly_) {
        style_->caret_.setPosition(caretDrawPosition_);
        target.draw(style_->caret_, states);
    }
}

}
