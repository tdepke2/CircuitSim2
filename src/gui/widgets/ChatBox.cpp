#include <gui/Debug.h>
#include <gui/Gui.h>
#include <gui/Theme.h>
#include <gui/Timer.h>
#include <gui/widgets/ChatBox.h>

#include <algorithm>
#include <cmath>
#include <limits>

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

void ChatBoxStyle::setInvertedTextFillColor(const sf::Color& color) {
    invertedTextColor_ = color;
    gui_.requestRedraw();
}
void ChatBoxStyle::setHighlightFillColor(const sf::Color& color) {
    highlightColor_ = color;
    gui_.requestRedraw();
}
void ChatBoxStyle::setTextPadding(const sf::Vector3f& padding) {
    textPadding_ = padding;
    gui_.requestRedraw();
}
const sf::Color& ChatBoxStyle::getInvertedTextFillColor() const {
    return invertedTextColor_;
}
const sf::Color& ChatBoxStyle::getHighlightFillColor() const {
    return highlightColor_;
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
    if (sizeCharacters_ == sizeCharacters) {
        return;
    }
    sizeCharacters_ = sizeCharacters;

    std::string textBounds(sizeCharacters.x, 'A');
    style_->text_.setString(textBounds);

    const auto bounds = style_->text_.getLocalBounds();
    size_ = sf::Vector2f(
        2.0f * style_->textPadding_.x + bounds.left + bounds.width,
        2.0f * style_->textPadding_.y + style_->textPadding_.z * style_->getCharacterSize() * sizeCharacters.y
    );

    updateVisibleLines();
}
void ChatBox::setSizeWithinBounds(const sf::Vector2f& size) {
    style_->text_.setString("A");
    const auto bounds1 = style_->text_.getLocalBounds();
    style_->text_.setString("AA");
    const auto bounds2 = style_->text_.getLocalBounds();

    // The first character may have some extra spacing, so figure out the actual width of each one.
    const auto charWidthFirst = bounds1.left + bounds1.width;
    const auto charWidth = bounds2.left + bounds2.width - charWidthFirst;

    sf::Vector2<size_t> sizeCharacters;
    if (size.x == size_.x) {
        sizeCharacters.x = sizeCharacters_.x;
    } else {
        sizeCharacters.x = static_cast<size_t>(std::max((size.x - 2.0f * style_->textPadding_.x - charWidthFirst + charWidth) / charWidth, 0.0f));
    }
    if (size.y == size_.y) {
        sizeCharacters.y = sizeCharacters_.y;
    } else {
        sizeCharacters.y = static_cast<size_t>(std::max((size.y - 2.0f * style_->textPadding_.y) / (style_->textPadding_.z * style_->getCharacterSize()), 0.0f));
    }
    setSizeCharacters(sizeCharacters);
    size_ = size;
}
void ChatBox::setMaxLines(size_t maxLines) {
    maxLines_ = maxLines;
    if (maxLines > 0 && lines_.size() > maxLines) {
        lines_.resize(maxLines + 1);
        removeLine(maxLines);
    }
}
void ChatBox::setAutoHide(bool autoHide) {
    if (autoHide_ != autoHide) {
        autoHide_ = autoHide;

        // We set up the callback here, as `shared_from_this()` will not work in
        // the ctor before a shared_ptr can take ownership of the object.
        if (!hideCallback_) {
            GUI_DEBUG << "First time setup for hideCallback_\n";
            auto weakThis = std::weak_ptr<ChatBox>(std::dynamic_pointer_cast<ChatBox>(shared_from_this()));
            hideCallback_ = [weakThis]() {
                auto sharedThis = weakThis.lock();
                if (sharedThis != nullptr && sharedThis->hideCounter_ > 0) {
                    --sharedThis->hideCounter_;
                    sharedThis->requestRedraw();
                }
            };
        }

        verticalScroll_ = 0;
        selectionStart_.second = false;
        if (autoHide) {
            setFocused(false);
        }
        updateVisibleLines();
    }
}
void ChatBox::setHideTime(std::chrono::milliseconds duration) {
    hideTime_ = duration;
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
bool ChatBox::getAutoHide() const {
    return autoHide_;
}
std::chrono::milliseconds ChatBox::getHideTime() const {
    return hideTime_;
}
void ChatBox::addLines(const sf::String& str) {
    addLines(str, style_->textColor_, style_->textStyle_);
}
void ChatBox::addLines(const sf::String& str, const sf::Color& color) {
    addLines(str, color, style_->textStyle_);
}
void ChatBox::addLines(const sf::String& str, uint32_t style) {
    addLines(str, style_->textColor_, style);
}
void ChatBox::addLines(const sf::String& str, const sf::Color& color, uint32_t style) {
    if (maxLines_ > 0 && lines_.size() == maxLines_) {
        lines_.pop_back();
    }
    size_t foundNewline = str.find("\n");
    if (foundNewline == sf::String::InvalidPos) {
        lines_.emplace_front(str, color, style);
    } else {
        lines_.emplace_front(str.substring(0, foundNewline), color, style);
    }

    if (verticalScroll_ > 0 && verticalScroll_ < lines_.size() - 1) {
        ++verticalScroll_;
    }
    if (selectionStart_.second && lines_.size() > 0) {
        selectionStart_.first = std::min(selectionStart_.first + 1, lines_.size() - 1);
        selectionEnd_ = std::min(selectionEnd_ + 1, lines_.size() - 1);
    }
    if (autoHide_) {
        ++hideCounter_;
        Timer::create(hideCallback_, hideTime_);
    }

    if (str.isEmpty() || foundNewline >= str.getSize() - 1) {
        updateVisibleLines();
    } else {
        addLines(str.substring(foundNewline + 1), color, style);
    }
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
        if (selectionStart_.second && lines_.size() > 0) {
            selectionStart_.first = std::min(selectionStart_.first, lines_.size() - 1);
            selectionEnd_ = std::min(selectionEnd_, lines_.size() - 1);
        }
        updateVisibleLines();
        return true;
    }
    return false;
}
void ChatBox::removeAllLines() {
    lines_.clear();
    verticalScroll_ = 0;
    selectionStart_.second = false;
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
bool ChatBox::isMouseIntersecting(const sf::Vector2f& mouseParent) const {
    return (autoHide_ ? false : baseClass::isMouseIntersecting(mouseParent));
}

bool ChatBox::handleMouseMove(const sf::Vector2f& mouseParent) {
    if (baseClass::handleMouseMove(mouseParent)) {
        return true;
    } else if (!isFocused() || autoHide_) {
        return false;
    }
    const auto mouseLocal = toLocalOriginSpace(mouseParent);
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
        updateSelection(findClosestLineToMouse(mouseLocal), true);
    }
    return isMouseIntersecting(mouseParent);
}
bool ChatBox::handleMouseWheelScroll(sf::Mouse::Wheel wheel, float delta, const sf::Vector2f& mouseParent) {
    if (baseClass::handleMouseWheelScroll(wheel, delta, mouseParent)) {
        return true;
    } else if (autoHide_) {
        return false;
    }
    updateScroll(static_cast<int>(std::round(delta)) * 3);
    return true;
}
void ChatBox::handleMousePress(sf::Mouse::Button button, const sf::Vector2f& mouseParent) {
    baseClass::handleMousePress(button, mouseParent);
    if (autoHide_) {
        return;
    }
    const auto mouseLocal = toLocalOriginSpace(mouseParent);
    if (button == sf::Mouse::Left) {
        updateSelection(findClosestLineToMouse(mouseLocal), false);
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
bool ChatBox::handleKeyPressed(const sf::Event::KeyEvent& key) {
    bool eventConsumed = baseClass::handleKeyPressed(key);
    if (autoHide_) {
        return eventConsumed;
    }

    if (key.control) {
        if (key.code == sf::Keyboard::A) {
            if (lines_.size() > 0) {
                updateSelection(0, false);
                updateSelection(lines_.size() - 1, true);
            }
        } else if (key.code == sf::Keyboard::X || key.code == sf::Keyboard::C) {
            if (selectionStart_.second) {
                sf::String selectedText;
                size_t selectionMin = std::min(selectionStart_.first, selectionEnd_);
                size_t selectionMax = std::max(selectionStart_.first, selectionEnd_);
                for (size_t i = selectionMin; i <= selectionMax; ++i) {
                    selectedText = lines_[i].str + "\n" + selectedText;
                }
                sf::Clipboard::setString(selectedText);
            }
        } else {
            return eventConsumed;
        }
        return true;
    }

    if (key.code == sf::Keyboard::PageUp) {
        updateScroll(static_cast<int>(sizeCharacters_.y) - 1);
    } else if (key.code == sf::Keyboard::PageDown) {
        updateScroll(-static_cast<int>(sizeCharacters_.y) + 1);
    } else if (key.code == sf::Keyboard::Home) {
        verticalScroll_ = 0;
        updateVisibleLines();
    } else if (key.code == sf::Keyboard::End) {
        verticalScroll_ = (lines_.size() > 0 ? lines_.size() - 1 : 0);
        updateVisibleLines();
    } else if (key.code == sf::Keyboard::Up) {
        updateScroll(1);
    } else if (key.code == sf::Keyboard::Down) {
        updateScroll(-1);
    } else {
        return eventConsumed;
    }
    return true;
}
void ChatBox::handleFocusChange(bool focused) {
    baseClass::handleFocusChange(focused);
    requestRedraw();
}

ChatBox::ChatBox(std::shared_ptr<ChatBoxStyle> style, const sf::String& name) :
    baseClass(name),
    style_(style),
    styleCopied_(false),
    sizeCharacters_(0, 0),
    maxLines_(0),
    autoHide_(false),
    size_(),
    lines_(),
    visibleLines_(),
    visibleLinesChanged_(false),
    verticalScroll_(0),
    selectionStart_(0, false),
    selectionEnd_(0),
    hideCallback_(),
    hideCounter_(0),
    hideTime_(2000) {
}

void ChatBox::updateVisibleLines(bool applyUpdate) const {
    if (!applyUpdate) {
        visibleLinesChanged_ = true;
        requestRedraw();
        return;
    } else if (!visibleLinesChanged_) {
        return;
    }

    const float textHeight = style_->textPadding_.z * style_->getCharacterSize();
    visibleLines_.clear();
    size_t currentLine = verticalScroll_;
    while (visibleLines_.size() < sizeCharacters_.y) {
        if (currentLine >= lines_.size()) {
            break;
        }
        size_t lastTrimPos = lines_[currentLine].str.getSize();
        do {
            sf::String trimmedString;
            if (lastTrimPos > sizeCharacters_.x) {
                lastTrimPos = (lastTrimPos - 1) / sizeCharacters_.x * sizeCharacters_.x;
                trimmedString = lines_[currentLine].str.substring(lastTrimPos, sizeCharacters_.x);
            } else {
                trimmedString = lines_[currentLine].str.substring(0, sizeCharacters_.x);
                lastTrimPos = 0;
            }

            ChatBoxLine line(
                std::move(trimmedString),
                lines_[currentLine].color,
                lines_[currentLine].style,
                currentLine
            );
            style_->text_.setPosition(
                style_->textPadding_.x,
                style_->textPadding_.y + textHeight * (sizeCharacters_.y - visibleLines_.size() - 1)
            );
            style_->text_.setString(line.str);
            sf::Vector2f pos1 = style_->text_.findCharacterPos(0);
            sf::Vector2f pos2 = style_->text_.findCharacterPos(style_->text_.getString().getSize());
            sf::RectangleShape rect({pos2.x - pos1.x + style_->textPadding_.x, textHeight});
            rect.setPosition(pos1 - sf::Vector2f(style_->textPadding_.x / 2.0f, 0.0f));

            visibleLines_.emplace_back(std::move(line), std::move(rect));
            GUI_DEBUG << "line [" << visibleLines_.back().first.str.toAnsiString() << "] has style " << static_cast<int>(visibleLines_.back().first.style) << "\n";
        } while (visibleLines_.size() < sizeCharacters_.y && lastTrimPos != 0);
        ++currentLine;
    }
    visibleLinesChanged_ = false;
}
void ChatBox::updateSelection(size_t pos, bool continueSelection) {
    const auto lastSelectionStart = selectionStart_;
    const auto lastSelectionEnd = selectionEnd_;

    if (selectionStart_.second && continueSelection) {
        if (pos != std::numeric_limits<size_t>::max()) {
            selectionEnd_ = pos;
        }
    } else if (pos == std::numeric_limits<size_t>::max()) {
        selectionStart_ = {0, false};
        selectionEnd_ = 0;
    } else {
        selectionStart_ = {pos, true};
        selectionEnd_ = pos;
    }

    if (selectionStart_ != lastSelectionStart || selectionEnd_ != lastSelectionEnd) {
        GUI_DEBUG << "Selection now " << selectionStart_.first << " to " << selectionEnd_ << (selectionStart_.second ? "" : " (no selection)") << "\n";
        requestRedraw();
    }
}
void ChatBox::updateScroll(int delta) {
    size_t newScroll = std::min(std::max(static_cast<int>(verticalScroll_) + delta, 0), static_cast<int>(lines_.size() > 0 ? lines_.size() - 1 : 0));
    if (verticalScroll_ != newScroll) {
        verticalScroll_ = newScroll;
        updateVisibleLines();
    }
}
size_t ChatBox::findClosestLineToMouse(const sf::Vector2f& mouseLocal) const {
    GUI_DEBUG << "chat clicked at: " << mouseLocal.x << ", " << mouseLocal.y << "\n";
    const float textHeight = style_->textPadding_.z * style_->getCharacterSize();
    size_t visibleLineClicked = std::max(static_cast<int>(sizeCharacters_.y) - 1 - static_cast<int>((mouseLocal.y + getOrigin().y - style_->textPadding_.y) / textHeight), 0);
    GUI_DEBUG << "visibleLineClicked = " << visibleLineClicked << "\n";
    if (visibleLineClicked < visibleLines_.size()) {
        return visibleLines_[visibleLineClicked].first.id;
    } else {
        return std::numeric_limits<size_t>::max();
    }
}

void ChatBox::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (!isVisible()) {
        return;
    }
    states.transform *= getTransform();

    updateVisibleLines(true);

    if (!autoHide_) {
        style_->rect_.setSize(size_);
        target.draw(style_->rect_, states);
    }

    const float textHeight = style_->textPadding_.z * style_->getCharacterSize();
    sf::Vector2f lastPosition = {
        style_->textPadding_.x,
        style_->textPadding_.y + textHeight * (sizeCharacters_.y - 1)
    };
    size_t lineCount = 0;
    for (size_t i = 0; i < visibleLines_.size(); ++i) {
        if (autoHide_ && lineCount == hideCounter_) {
            break;
        }
        style_->text_.setPosition(roundVector2f(lastPosition));
        style_->text_.setString(visibleLines_[i].first.str);
        if (!autoHide_) {
            style_->text_.setFillColor(visibleLines_[i].first.color);
        } else {
            style_->text_.setFillColor(style_->invertedTextColor_);
            visibleLines_[i].second.setFillColor(visibleLines_[i].first.color);
            target.draw(visibleLines_[i].second, states);
        }
        style_->text_.setStyle(visibleLines_[i].first.style);
        target.draw(style_->text_, states);
        lastPosition.y -= textHeight;
        if (i + 1 < visibleLines_.size() && visibleLines_[i].first.id != visibleLines_[i + 1].first.id) {
            ++lineCount;
        }
    }

    if (!autoHide_ && isFocused()) {
        size_t selectionMin = std::numeric_limits<size_t>::max();
        size_t selectionMax = std::numeric_limits<size_t>::max();
        if (selectionStart_.second) {
            selectionMin = std::min(selectionStart_.first, selectionEnd_);
            selectionMax = std::max(selectionStart_.first, selectionEnd_);
        }

        for (size_t i = 0; i < visibleLines_.size(); ++i) {
            if (visibleLines_[i].first.id >= selectionMin && visibleLines_[i].first.id <= selectionMax) {
                visibleLines_[i].second.setFillColor(style_->highlightColor_);
                target.draw(visibleLines_[i].second, states);
            }
        }
    }
}

}
