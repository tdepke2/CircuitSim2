#include <gui/Gui.h>
#include <gui/Theme.h>
#include <gui/widgets/ChatBox.h>

#include <algorithm>
#include <cmath>






#include <iostream>



namespace {

sf::Vector2f roundVector2f(sf::Vector2f v) {
    return {std::round(v.x), std::round(v.y)};
}

}

namespace gui {

ChatBoxStyle::ChatBoxStyle(const Gui& gui) :
    Style(gui),
    textStyle_(sf::Text::Regular) {
}

// sf::Shape interface.
void ChatBoxStyle::setTexture(const sf::Texture* texture, bool resetRect) {
    rect_.setTexture(texture, resetRect);
    gui_.requestRedraw();
}
void ChatBoxStyle::setTextureRect(const sf::IntRect& rect) {
    rect_.setTextureRect(rect);
    gui_.requestRedraw();
}
void ChatBoxStyle::setFillColor(const sf::Color& color) {
    rect_.setFillColor(color);
    gui_.requestRedraw();
}
void ChatBoxStyle::setOutlineColor(const sf::Color& color) {
    rect_.setOutlineColor(color);
    gui_.requestRedraw();
}
void ChatBoxStyle::setOutlineThickness(float thickness) {
    rect_.setOutlineThickness(thickness);
    gui_.requestRedraw();
}
const sf::Texture* ChatBoxStyle::getTexture() const {
    return rect_.getTexture();
}
const sf::IntRect& ChatBoxStyle::getTextureRect() const {
    return rect_.getTextureRect();
}
const sf::Color& ChatBoxStyle::getFillColor() const {
    return rect_.getFillColor();
}
const sf::Color& ChatBoxStyle::getOutlineColor() const {
    return rect_.getOutlineColor();
}
float ChatBoxStyle::getOutlineThickness() const {
    return rect_.getOutlineThickness();
}

// sf::Text interface.
void ChatBoxStyle::setFont(const sf::Font& font) {
    text_.setFont(font);
    gui_.requestRedraw();
}
void ChatBoxStyle::setCharacterSize(unsigned int size) {
    text_.setCharacterSize(size);
    gui_.requestRedraw();
}
void ChatBoxStyle::setLineSpacing(float spacingFactor) {
    text_.setLineSpacing(spacingFactor);
    gui_.requestRedraw();
}
void ChatBoxStyle::setLetterSpacing(float spacingFactor) {
    text_.setLetterSpacing(spacingFactor);
    gui_.requestRedraw();
}
void ChatBoxStyle::setTextStyle(uint32_t style) {
    textStyle_ = style;
    gui_.requestRedraw();
}
void ChatBoxStyle::setTextFillColor(const sf::Color& color) {
    textColor_ = color;
    gui_.requestRedraw();
}
const sf::Font* ChatBoxStyle::getFont() const {
    return text_.getFont();
}
unsigned int ChatBoxStyle::getCharacterSize() const {
    return text_.getCharacterSize();
}
float ChatBoxStyle::getLineSpacing() const {
    return text_.getLineSpacing();
}
float ChatBoxStyle::getLetterSpacing() const {
    return text_.getLetterSpacing();
}
uint32_t ChatBoxStyle::getTextStyle() const {
    return textStyle_;
}
const sf::Color& ChatBoxStyle::getTextFillColor() const {
    return textColor_;
}

void ChatBoxStyle::setTextPadding(const sf::Vector3f& padding) {
    textPadding_ = padding;
    gui_.requestRedraw();
}
const sf::Vector3f& ChatBoxStyle::getTextPadding() const {
    return textPadding_;
}

std::shared_ptr<ChatBoxStyle> ChatBoxStyle::clone() const {
    return std::make_shared<ChatBoxStyle>(*this);
}



std::shared_ptr<ChatBox> ChatBox::create(const Theme& theme, const sf::String& name) {
    return std::shared_ptr<ChatBox>(new ChatBox(theme.getStyle<ChatBoxStyle>("ChatBox"), name));
}
std::shared_ptr<ChatBox> ChatBox::create(std::shared_ptr<ChatBoxStyle> style, const sf::String& name) {
    return std::shared_ptr<ChatBox>(new ChatBox(style, name));
}

void ChatBox::setSizeCharacters(const sf::Vector2<size_t>& sizeCharacters) {
    sizeCharacters_ = sizeCharacters;

    std::string textBounds(sizeCharacters.x, 'A');
    style_->text_.setString(textBounds);

    const auto bounds = style_->text_.getLocalBounds();
    size_ = sf::Vector2f(
        2.0f * style_->textPadding_.x + bounds.left + bounds.width,
        2.0f * style_->textPadding_.y + style_->textPadding_.z * style_->getCharacterSize() * sizeCharacters.y
    );

    // FIXME update and redraw?
    updateVisibleLines();
}
void ChatBox::setMaxLines(size_t maxLines) {
    maxLines_ = maxLines;
    // FIXME trim lines to fit the new max.
}
const sf::Vector2f& ChatBox::getSize() const {
    return size_;
}
const sf::Vector2<size_t>& ChatBox::getSizeCharacters() const {
    return sizeCharacters_;
}
size_t ChatBox::getMaxLines() const {
    return maxLines_;
}
void ChatBox::addLine(const sf::String& str) {
    addLine(str, style_->textColor_, style_->textStyle_);
}
void ChatBox::addLine(const sf::String& str, const sf::Color& color) {
    addLine(str, color, style_->textStyle_);
}
void ChatBox::addLine(const sf::String& str, uint32_t style) {
    addLine(str, style_->textColor_, style);
}
void ChatBox::addLine(const sf::String& str, const sf::Color& color, uint32_t style) {
    lines_.emplace_front(str, color, style);
    if (verticalScroll_ > 0) {
        ++verticalScroll_;
    }
    updateVisibleLines();
}
const ChatBoxLine& ChatBox::getLine(size_t index) const {
    return lines_.at(index);
}
size_t ChatBox::getNumLines() const {
    return lines_.size();
}
bool ChatBox::removeLine(size_t index) {
    if (index < lines_.size()) {
        lines_.erase(lines_.begin() + index);
        verticalScroll_ = std::min(verticalScroll_, (lines_.size() > 0 ? lines_.size() - 1 : 0));
        updateVisibleLines();
        return true;
    }
    return false;
}
void ChatBox::removeAllLines() {
    lines_.clear();
    verticalScroll_ = 0;
    updateVisibleLines();
}

void ChatBox::setStyle(std::shared_ptr<ChatBoxStyle> style) {
    style_ = style;
    styleCopied_ = false;
    requestRedraw();
}
std::shared_ptr<ChatBoxStyle> ChatBox::getStyle() {
    if (!styleCopied_) {
        style_ = style_->clone();
        styleCopied_ = true;
    }
    return style_;
}

sf::FloatRect ChatBox::getLocalBounds() const {
    return {-getOrigin(), size_};
}
bool ChatBox::handleMouseWheelScroll(sf::Mouse::Wheel wheel, float delta, const sf::Vector2f& mouseParent) {
    if (baseClass::handleMouseWheelScroll(wheel, delta, mouseParent)) {
        return true;
    }
    updateScroll(static_cast<int>(std::round(delta)) * 3);
    return true;
}
void ChatBox::handleMousePress(sf::Mouse::Button button, const sf::Vector2f& mouseParent) {
    baseClass::handleMousePress(button, mouseParent);
    const auto mouseLocal = toLocalOriginSpace(mouseParent);
    if (button == sf::Mouse::Left) {
        std::cout << "chat clicked at: " << mouseLocal.x << ", " << mouseLocal.y << "\n";
        const float textHeight = style_->textPadding_.z * style_->getCharacterSize();
        size_t visibleLineClicked = std::max(static_cast<int>(sizeCharacters_.y) - 1 - static_cast<int>((mouseLocal.y + getOrigin().y - style_->textPadding_.y) / textHeight), 0);
        std::cout << "visibleLineClicked = " << visibleLineClicked << "\n";
        if (visibleLineClicked < visibleLines_.size()) {
            updateSelection(visibleLines_[visibleLineClicked].id, false);
        }
    }
    if (button <= sf::Mouse::Middle) {
        onClick.emit(this, mouseLocal);
    }
    onMousePress.emit(this, button, mouseLocal);
}
void ChatBox::handleMouseRelease(sf::Mouse::Button button, const sf::Vector2f& mouseParent) {
    onMouseRelease.emit(this, button, toLocalOriginSpace(mouseParent));
    baseClass::handleMouseRelease(button, mouseParent);
}
/*bool ChatBox::handleKeyPressed(const sf::Event::KeyEvent& key) {
    bool eventConsumed = baseClass::handleKeyPressed(key);
    size_t caretOffset = findCaretOffset(caretPosition_);

    if (key.control) {
        if (key.code == sf::Keyboard::Up) {
            updateScroll(true, 1, key.shift);
        } else if (key.code == sf::Keyboard::Down) {
            updateScroll(true, -1, key.shift);
        } else if (key.code == sf::Keyboard::A) {
            updateCaretPosition(0, false);
            updateCaretPosition(findStringsLength(boxStrings_), true);
        } else if (key.code == sf::Keyboard::X) {
            if (selectionStart_.second) {
                sf::Clipboard::setString(getSelectedText());
                insertCharacter('\u0008');
            }
        } else if (key.code == sf::Keyboard::C) {
            if (selectionStart_.second) {
                sf::Clipboard::setString(getSelectedText());
            }
        } else {
            return eventConsumed;
        }
        return true;
    }

    if (key.code == sf::Keyboard::PageUp) {
        updateCaretPosition(0, key.shift);
    } else if (key.code == sf::Keyboard::PageDown) {
        updateCaretPosition(findStringsLength(boxStrings_), key.shift);
    } else if (key.code == sf::Keyboard::Home) {
        updateCaretPosition({0, caretPosition_.y}, key.shift);
    } else if (key.code == sf::Keyboard::End) {
        updateCaretPosition({boxStrings_[caretPosition_.y].getSize(), caretPosition_.y}, key.shift);
    } else if (key.code == sf::Keyboard::Up) {
        if (caretPosition_.y > 0) {
            updateCaretPosition(findCaretOffset({caretPosition_.x, caretPosition_.y - 1}), key.shift);
        } else {
            updateCaretPosition(0, key.shift);
        }
    } else if (key.code == sf::Keyboard::Down) {
        if (caretPosition_.y < boxStrings_.size() - 1) {
            updateCaretPosition(findCaretOffset({caretPosition_.x, caretPosition_.y + 1}), key.shift);
        } else {
            updateCaretPosition(findStringsLength(boxStrings_), key.shift);
        }
    } else {
        return eventConsumed;
    }
    return true;
}
void ChatBox::handleFocusChange(bool focused) {
    baseClass::handleFocusChange(focused);
    requestRedraw();
}*/

ChatBox::ChatBox(std::shared_ptr<ChatBoxStyle> style, const sf::String& name) :
    baseClass(name),
    style_(style),
    styleCopied_(false),
    sizeCharacters_(0, 0),
    maxLines_(0),
    size_(),
    lines_(),
    visibleLines_(),
    verticalScroll_(0),
    selectionStart_(0, false),
    selectionEnd_(0) {
}

void ChatBox::updateVisibleLines() {
    visibleLines_.clear();
    size_t currentLine = verticalScroll_;
    while (visibleLines_.size() < sizeCharacters_.y) {
        if (currentLine >= lines_.size()) {
            break;
        }
        size_t lastTrimPos = lines_[currentLine].str.getSize();
        do {
            if (lastTrimPos > sizeCharacters_.x) {
                lastTrimPos = (lastTrimPos - 1) / sizeCharacters_.x * sizeCharacters_.x;
                visibleLines_.emplace_back(lines_[currentLine].str.substring(lastTrimPos, sizeCharacters_.x), lines_[currentLine].color, lines_[currentLine].style, currentLine);
                std::cout << "line [" << visibleLines_.back().str.toAnsiString() << "] has style " << static_cast<int>(visibleLines_.back().style) << "\n";
            } else {
                visibleLines_.emplace_back(lines_[currentLine].str.substring(0, sizeCharacters_.x), lines_[currentLine].color, lines_[currentLine].style, currentLine);
                std::cout << "line [" << visibleLines_.back().str.toAnsiString() << "] has style " << static_cast<int>(visibleLines_.back().style) << "\n";
                break;
            }
        } while (visibleLines_.size() < sizeCharacters_.y);
        ++currentLine;
    }
    requestRedraw();
}
void ChatBox::updateSelection(size_t pos, bool continueSelection) {
    /*if (!selectionStart_.second && continueSelection) {
        selectionStart_.first = pos;
        selectionStart_.second = true;
    }

    if (selectionStart_.second) {

    }*/

    std::cout << "need to select line " << pos << "\n";
}
void ChatBox::updateScroll(int delta) {
    size_t newScroll = std::min(std::max(static_cast<int>(verticalScroll_) + delta, 0), static_cast<int>(lines_.size() > 0 ? lines_.size() - 1 : 0));
    if (verticalScroll_ != newScroll) {
        verticalScroll_ = newScroll;
        updateVisibleLines();
    }
}

void ChatBox::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (!isVisible()) {
        return;
    }
    states.transform *= getTransform();

    style_->rect_.setSize(size_);
    target.draw(style_->rect_, states);

    const float textHeight = style_->textPadding_.z * style_->getCharacterSize();
    sf::Vector2f lastPosition = {
        style_->textPadding_.x,
        style_->textPadding_.y + textHeight * (sizeCharacters_.y - 1)
    };
    for (const auto& line : visibleLines_) {
        style_->text_.setPosition(roundVector2f(lastPosition));
        style_->text_.setString(line.str);
        style_->text_.setFillColor(line.color);
        style_->text_.setStyle(line.style);
        target.draw(style_->text_, states);
        lastPosition.y -= textHeight;
    }
}

}
