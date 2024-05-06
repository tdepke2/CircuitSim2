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
    // FIXME: insert multiple lines if we need to split it up? nope, not anymore
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
        updateVisibleLines();
        return true;
    }
    return false;
}
void ChatBox::removeAllLines() {
    lines_.clear();
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

ChatBox::ChatBox(std::shared_ptr<ChatBoxStyle> style, const sf::String& name) :
    baseClass(name),
    style_(style),
    styleCopied_(false),
    sizeCharacters_(0, 0),
    maxLines_(0),
    size_(),
    lines_(),
    visibleLines_(),
    verticalScroll_(0) {
}

void ChatBox::updateVisibleLines() {
    visibleLines_.clear();
    size_t currentLine = 0;
    while (visibleLines_.size() < sizeCharacters_.y) {
        if (currentLine >= lines_.size()) {
            break;
        }
        size_t lastTrimPos = lines_[currentLine].str.getSize();
        do {
            if (lastTrimPos > sizeCharacters_.x) {
                lastTrimPos = (lastTrimPos - 1) / sizeCharacters_.x * sizeCharacters_.x;
                visibleLines_.emplace_back(lines_[currentLine].str.substring(lastTrimPos, sizeCharacters_.x), lines_[currentLine].color, lines_[currentLine].style);
                std::cout << "line [" << visibleLines_.back().str.toAnsiString() << "] has style " << static_cast<int>(visibleLines_.back().style) << "\n";
            } else {
                visibleLines_.emplace_back(lines_[currentLine].str.substring(0, sizeCharacters_.x), lines_[currentLine].color, lines_[currentLine].style);
                std::cout << "line [" << visibleLines_.back().str.toAnsiString() << "] has style " << static_cast<int>(visibleLines_.back().style) << "\n";
                break;
            }
        } while (visibleLines_.size() < sizeCharacters_.y);
        ++currentLine;
    }
    requestRedraw();
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
