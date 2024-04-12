#include <gui/Gui.h>
#include <gui/Theme.h>
#include <gui/widgets/MultilineTextBox.h>
#include <gui/widgets/TextBox.h>

#include <cmath>
#include <limits>







#include <iostream>



namespace {

std::vector<sf::String> splitString(const sf::String& str) {
    std::vector<sf::String> stringArray;
    size_t i = 0;
    size_t length = str.find("\n");
    while (length != sf::String::InvalidPos) {
        stringArray.emplace_back(str.substring(i, length - i));
        i = length + 1;
        length = str.find("\n", i);
    }
    stringArray.emplace_back(str.substring(i, length));
    return stringArray;
}

sf::String combineStrings(const std::vector<sf::String>& stringArray) {
    sf::String str;
    for (size_t i = 0; i < stringArray.size(); ++i) {
        str += (i == 0 ? "" : "\n") + stringArray[i];
    }
    return str;
}

size_t findStringsLength(const std::vector<sf::String>& stringArray) {
    size_t length = 0;
    for (const auto& x : stringArray) {
        length += x.getSize() + 1;
    }
    return (length > 0 ? length - 1 : length);
}

size_t findLongestLength(const std::vector<sf::String>& stringArray) {
    size_t longest = 0;
    for (const auto& x : stringArray) {
        longest = std::max(longest, x.getSize());
    }
    return longest;
}

template<typename T>
std::pair<sf::Vector2<T>, sf::Vector2<T>> sortByYFirst(const sf::Vector2<T>& a, const sf::Vector2<T>& b) {
    if (a.y < b.y || (a.y == b.y && a.x < b.x)) {
        return {a, b};
    } else {
        return {b, a};
    }
}

}

namespace gui {

MultilineTextBoxStyle::MultilineTextBoxStyle(const Gui& gui) :
    TextBoxStyle(gui) {
}

void MultilineTextBoxStyle::setHighlightFillColor(const sf::Color& color) {
    highlightColor_ = color;
    gui_.requestRedraw();
}
const sf::Color& MultilineTextBoxStyle::getHighlightFillColor() const {
    return highlightColor_;
}

std::shared_ptr<MultilineTextBoxStyle> MultilineTextBoxStyle::clone() const {
    return std::make_shared<MultilineTextBoxStyle>(*this);
}



std::shared_ptr<MultilineTextBox> MultilineTextBox::create(const Theme& theme, const sf::String& name) {
    return std::shared_ptr<MultilineTextBox>(new MultilineTextBox(theme.getMultilineTextBoxStyle(), name));
}
std::shared_ptr<MultilineTextBox> MultilineTextBox::create(std::shared_ptr<MultilineTextBoxStyle> style, const sf::String& name) {
    return std::shared_ptr<MultilineTextBox>(new MultilineTextBox(style, name));
}

void MultilineTextBox::setSizeCharacters(const sf::Vector2<size_t>& sizeCharacters) {
    sizeCharacters_ = sizeCharacters;

    std::string textBounds(sizeCharacters.x, 'A');
    style_->text_.setString(textBounds);

    const auto bounds = style_->text_.getLocalBounds();
    size_ = sf::Vector2f(
        2.0f * style_->textPadding_.x + bounds.left + bounds.width,
        2.0f * style_->textPadding_.y + style_->textPadding_.z * style_->getCharacterSize() * sizeCharacters.y
    );

    updateCaretPosition(0, false);
}
void MultilineTextBox::setMaxCharacters(size_t maxCharacters) {
    maxCharacters_ = maxCharacters;
}
void MultilineTextBox::setMaxLines(size_t maxLines) {
    maxLines_ = maxLines;
}
void MultilineTextBox::setReadOnly(bool readOnly) {
    readOnly_ = readOnly;
    requestRedraw();
}
void MultilineTextBox::setTabPolicy(TabPolicy tabPolicy) {
    tabPolicy_ = tabPolicy;
}
void MultilineTextBox::setText(const sf::String& text) {
    boxStrings_ = splitString(text);
    updateCaretPosition(0, false);
    onTextChange.emit(this, findStringsLength(boxStrings_));
}
void MultilineTextBox::setDefaultText(const sf::String& text) {
    defaultStrings_ = splitString(text);
    updateCaretPosition(0, false);
}
const sf::Vector2f& MultilineTextBox::getSize() const {
    return size_;
}
const sf::Vector2<size_t>& MultilineTextBox::getSizeCharacters() const {
    return sizeCharacters_;
}
size_t MultilineTextBox::getMaxCharacters() const {
    return maxCharacters_;
}
size_t MultilineTextBox::getMaxLines() const {
    return maxLines_;
}
bool MultilineTextBox::getReadOnly() const {
    return readOnly_;
}
MultilineTextBox::TabPolicy MultilineTextBox::getTabPolicy() const {
    return tabPolicy_;
}
sf::String MultilineTextBox::getText() const {
    return combineStrings(boxStrings_);
}
sf::String MultilineTextBox::getDefaultText() const {
    return combineStrings(defaultStrings_);
}
void MultilineTextBox::selectAll() {
    updateCaretPosition(0, false);
    updateCaretPosition(findStringsLength(boxStrings_), true);
}
void MultilineTextBox::deselectAll() {
    updateCaretPosition(0, false);
}

void MultilineTextBox::setStyle(std::shared_ptr<MultilineTextBoxStyle> style) {
    style_ = style;
    styleCopied_ = false;
    requestRedraw();
}
std::shared_ptr<MultilineTextBoxStyle> MultilineTextBox::getStyle() {
    if (!styleCopied_) {
        style_ = style_->clone();
        styleCopied_ = true;
    }
    return style_;
}

sf::FloatRect MultilineTextBox::getLocalBounds() const {
    return {-getOrigin(), size_};
}
bool MultilineTextBox::handleMouseMove(const sf::Vector2f& mouseParent) {
    if (Widget::handleMouseMove(mouseParent)) {
        return true;
    }
    const auto mouseLocal = toLocalOriginSpace(mouseParent);
    if (!isFocused()) {
        return false;
    }
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
        size_t offset = findClosestOffsetToMouse(mouseLocal);
        if (offset != findCaretOffset(caretPosition_)) {
            updateCaretPosition(offset, true);
        }
    }
    return true;
}
bool MultilineTextBox::handleMouseWheelScroll(sf::Mouse::Wheel wheel, float delta, const sf::Vector2f& mouseParent) {
    if (Widget::handleMouseWheelScroll(wheel, delta, mouseParent)) {
        return true;
    }
    const bool shiftKeyPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::RShift);
    updateScroll(!shiftKeyPressed, static_cast<int>(std::round(delta)) * 3, false);
    return true;
}
void MultilineTextBox::handleMousePress(sf::Mouse::Button button, const sf::Vector2f& mouseParent) {
    Widget::handleMousePress(button, mouseParent);
    const auto mouseLocal = toLocalOriginSpace(mouseParent);
    if (button == sf::Mouse::Left) {
        updateCaretPosition(findClosestOffsetToMouse(mouseLocal), false);
    }
    if (button <= sf::Mouse::Middle) {
        onClick.emit(this, mouseLocal);
    }
    onMousePress.emit(this, button, mouseLocal);
}
void MultilineTextBox::handleMouseRelease(sf::Mouse::Button button, const sf::Vector2f& mouseParent) {
    onMouseRelease.emit(this, button, toLocalOriginSpace(mouseParent));
    Widget::handleMouseRelease(button, mouseParent);
}
bool MultilineTextBox::handleTextEntered(uint32_t unicode) {
    bool eventConsumed = Widget::handleTextEntered(unicode);
    if (unicode >= '\u0020' && unicode != '\u007f') {    // Printable character.
        insertCharacter(unicode);
        return true;
    }
    return eventConsumed;
}
bool MultilineTextBox::handleKeyPressed(const sf::Event::KeyEvent& key) {
    bool eventConsumed = Widget::handleKeyPressed(key);
    size_t caretOffset = findCaretOffset(caretPosition_);

    auto getSelectedText = [this]() -> sf::String {
        sf::String selectedText = "";
        auto selection = sortByYFirst(selectionStart_.first, selectionEnd_);
        for (size_t y = selection.first.y; y <= selection.second.y; ++y) {
            if (y == selection.first.y) {
                if (y == selection.second.y) {
                    selectedText += boxStrings_[y].substring(selection.first.x, selection.second.x - selection.first.x);
                } else {
                    selectedText += boxStrings_[y].substring(selection.first.x);
                }
            } else if (y > selection.first.y && y < selection.second.y) {
                selectedText += "\n" + boxStrings_[y];
            } else if (y == selection.second.y) {
                selectedText += "\n" + boxStrings_[y].substring(0, selection.second.x);
            }
        }
        std::cout << "selectedText = [" << selectedText.toAnsiString() << "]\n";
        return selectedText;
    };

    if (key.control) {
        if (key.code == sf::Keyboard::Up) {
            updateScroll(true, 1, key.shift);
        } else if (key.code == sf::Keyboard::Down) {
            updateScroll(true, -1, key.shift);
        } else if (key.code == sf::Keyboard::Left) {
            if (caretPosition_.x > 0) {
                bool foundNonSpace = false;
                const sf::String& line = boxStrings_[caretPosition_.y];
                size_t i = caretPosition_.x - 1;
                while (i > 0) {
                    if (line[i] != ' ') {
                        foundNonSpace = true;
                    } else if (foundNonSpace) {
                        break;
                    }
                    --i;
                }
                updateCaretPosition(findCaretOffset({(line[i] == ' ' && foundNonSpace ? i + 1 : i), caretPosition_.y}), key.shift);
            } else if (caretOffset > 0) {
                updateCaretPosition(caretOffset - 1, key.shift);
            }
        } else if (key.code == sf::Keyboard::Right) {
            if (caretPosition_.x < boxStrings_[caretPosition_.y].getSize()) {
                bool foundSpace = false;
                const sf::String& line = boxStrings_[caretPosition_.y];
                size_t i = caretPosition_.x;
                while (i < line.getSize()) {
                    if (line[i] == ' ') {
                        foundSpace = true;
                    } else if (foundSpace) {
                        break;
                    }
                    ++i;
                }
                updateCaretPosition(findCaretOffset({i, caretPosition_.y}), key.shift);
            } else if (caretOffset < findStringsLength(boxStrings_)) {
                updateCaretPosition(caretOffset + 1, key.shift);
            }
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
        } else if (key.code == sf::Keyboard::V) {
            bool textChanged = false;
            for (auto c : sf::Clipboard::getString()) {
                textChanged = insertCharacter(c, true) || textChanged;
            }
            if (textChanged) {
                onTextChange.emit(this, findStringsLength(boxStrings_));
            }
        } else {
            return eventConsumed;
        }
        return true;
    }

    if (key.code == sf::Keyboard::Enter) {
        if (maxLines_ == 1 || !insertCharacter('\u000a')) {
            return eventConsumed;
        }
    } else if (key.code == sf::Keyboard::Backspace) {
        insertCharacter('\u0008');
    } else if (key.code == sf::Keyboard::Tab) {
        if (tabPolicy_ == TabPolicy::ignoreTab) {
            return eventConsumed;
        }
        insertCharacter('\u0009');
    } else if (key.code == sf::Keyboard::Delete) {
        insertCharacter('\u007f');
    } else if (key.code == sf::Keyboard::PageUp) {
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
    } else if (key.code == sf::Keyboard::Left) {
        if (selectionStart_.second && !key.shift) {
            updateCaretPosition(sortByYFirst(selectionStart_.first, selectionEnd_).first, false);
        } else if (caretOffset > 0) {
            updateCaretPosition(caretOffset - 1, key.shift);
        }
    } else if (key.code == sf::Keyboard::Right) {
        if (selectionStart_.second && !key.shift) {
            updateCaretPosition(sortByYFirst(selectionStart_.first, selectionEnd_).second, false);
        } else if (caretOffset < findStringsLength(boxStrings_)) {
            updateCaretPosition(caretOffset + 1, key.shift);
        }
    } else {
        return eventConsumed;
    }
    return true;
}

MultilineTextBox::MultilineTextBox(std::shared_ptr<MultilineTextBoxStyle> style, const sf::String& name) :
    Widget(name),
    style_(style),
    styleCopied_(false),
    sizeCharacters_(0, 0),
    maxCharacters_(0),
    maxLines_(0),
    readOnly_(false),
    tabPolicy_(TabPolicy::expandTab),
    size_(0.0f, 0.0f),
    boxStrings_{""},
    defaultStrings_{""},
    visibleString_(""),
    scroll_(0, 0),
    caretPosition_(),
    caretDrawPosition_(),
    selectionLines_(),
    selectionStart_({0, 0}, false),
    selectionEnd_(0, 0) {

    updateCaretPosition(0, false);
}

bool MultilineTextBox::insertCharacter(uint32_t unicode, bool suppressSignals) {
    if (readOnly_) {
        return false;
    }
    bool textChanged = false, clearedSelection = false;
    if (selectionStart_.second) {
        auto selection = sortByYFirst(selectionStart_.first, selectionEnd_);
        for (size_t y = selection.first.y; y <= selection.second.y; ++y) {
            if (y == selection.first.y) {
                if (y == selection.second.y) {
                    boxStrings_[y] = boxStrings_[y].substring(0, selection.first.x) + boxStrings_[y].substring(selection.second.x);
                } else {
                    boxStrings_[y] = boxStrings_[y].substring(0, selection.first.x);
                }
            } else if (y > selection.first.y && y < selection.second.y) {
                boxStrings_.erase(boxStrings_.begin() + selection.first.y + 1);
            } else if (y == selection.second.y) {
                boxStrings_[selection.first.y] += boxStrings_[selection.first.y + 1].substring(selection.second.x);
                boxStrings_.erase(boxStrings_.begin() + selection.first.y + 1);
            }
        }
        if (selectionStart_.first != selectionEnd_) {
            textChanged = true;
            clearedSelection = true;
        }
        updateCaretPosition(selection.first, false);
    }
    size_t caretOffset = findCaretOffset(caretPosition_);

    if (unicode == '\u000a') {    // Enter key.
        if ((maxCharacters_ == 0 || findStringsLength(boxStrings_) < maxCharacters_) && (maxLines_ == 0 || boxStrings_.size() < maxLines_)) {
            boxStrings_.emplace(boxStrings_.begin() + caretPosition_.y + 1, boxStrings_[caretPosition_.y].substring(caretPosition_.x));
            boxStrings_[caretPosition_.y] = boxStrings_[caretPosition_.y].substring(0, caretPosition_.x);
            updateCaretPosition(caretOffset + 1, false);
            textChanged = true;
        }
    } else if (unicode == '\u0008') {    // Backspace key.
        if (!clearedSelection && caretOffset > 0) {
            if (caretPosition_.x == 0) {
                boxStrings_[caretPosition_.y - 1] += boxStrings_[caretPosition_.y];
                boxStrings_.erase(boxStrings_.begin() + caretPosition_.y);
            } else {
                boxStrings_[caretPosition_.y].erase(caretPosition_.x - 1, 1);
            }
            if (scroll_.x > 0 && scroll_.x + sizeCharacters_.x > findLongestLength(boxStrings_)) {
                --scroll_.x;
            }
            updateCaretPosition(caretOffset - 1, false);
            textChanged = true;
        }
    } else if (unicode == '\u0009') {    // Tab key.
        size_t count = 4 - caretPosition_.x % 4;
        for (size_t i = 0; i < count; ++i) {
            textChanged = insertCharacter(' ', true) || textChanged;
        }
    } else if (unicode == '\u007f') {    // Delete key.
        if (!clearedSelection && caretOffset < findStringsLength(boxStrings_)) {
            if (caretPosition_.x == boxStrings_[caretPosition_.y].getSize()) {
                boxStrings_[caretPosition_.y] += boxStrings_[caretPosition_.y + 1];
                boxStrings_.erase(boxStrings_.begin() + caretPosition_.y + 1);
            } else {
                boxStrings_[caretPosition_.y].erase(caretPosition_.x, 1);
            }
            if (scroll_.x > 0 && scroll_.x + sizeCharacters_.x > findLongestLength(boxStrings_)) {
                --scroll_.x;
            }
            updateCaretPosition(caretPosition_, false);
            textChanged = true;
        }
    } else {    // Printable character.
        if (maxCharacters_ == 0 || findStringsLength(boxStrings_) < maxCharacters_) {
            boxStrings_[caretPosition_.y].insert(caretPosition_.x, sf::String(unicode));
            updateCaretPosition(caretOffset + 1, false);
            textChanged = true;
        }
    }
    if (textChanged && !suppressSignals) {
        onTextChange.emit(this, findStringsLength(boxStrings_));
    }
    return textChanged;
}

void MultilineTextBox::updateCaretPosition(const sf::Vector2<size_t>& caretPosition, bool continueSelection) {
    if (!selectionStart_.second && continueSelection) {
        selectionStart_.first = caretPosition_;
        selectionStart_.second = true;
    }

    caretPosition_ = caretPosition;
    if (caretPosition_.x < scroll_.x) {
        scroll_.x = caretPosition_.x;
    } else if (caretPosition_.x > scroll_.x + sizeCharacters_.x) {
        scroll_.x = caretPosition_.x - sizeCharacters_.x;
    }
    if (caretPosition_.y < scroll_.y) {
        scroll_.y = caretPosition_.y;
    } else if (caretPosition_.y >= scroll_.y + sizeCharacters_.y) {
        scroll_.y = caretPosition_.y - sizeCharacters_.y + 1;
    }
    visibleString_ = "";
    size_t visibleCaretOffset = 0;
    for (size_t y = scroll_.y; y < std::min(boxStrings_.size(), sizeCharacters_.y + scroll_.y); ++y) {
        if (y > scroll_.y) {
            visibleString_ += "\n";
        }
        if (y == caretPosition_.y) {
            visibleCaretOffset = visibleString_.getSize() + caretPosition_.x - scroll_.x;
        }
        visibleString_ += boxStrings_[y].substring(std::min(boxStrings_[y].getSize(), scroll_.x), sizeCharacters_.x);
    }

    style_->text_.setString(visibleString_);
    style_->text_.setPosition(style_->textPadding_.x, style_->textPadding_.y);
    caretDrawPosition_ = style_->text_.findCharacterPos(visibleCaretOffset);

    if (selectionStart_.second) {
        if (continueSelection) {
            selectionEnd_ = caretPosition_;
            selectionLines_.clear();
            auto addSelectionLine = [this](size_t a, size_t b) {
                sf::Vector2f pos1 = style_->text_.findCharacterPos(a);
                sf::Vector2f pos2 = style_->text_.findCharacterPos(b);
                selectionLines_.emplace_back(sf::Vector2f(pos2.x - pos1.x, style_->textPadding_.z * style_->getCharacterSize()));
                selectionLines_.back().setPosition(pos1);
            };
            auto selection = sortByYFirst(selectionStart_.first, selectionEnd_);
            if (selection.first.y < scroll_.y) {
                selection.first = scroll_;
            } else {
                selection.first.x = std::min(std::max(selection.first.x, scroll_.x), sizeCharacters_.x + scroll_.x);
            }
            if (selection.second.y > sizeCharacters_.y + scroll_.y) {
                selection.second = sizeCharacters_ + scroll_;
            } else {
                selection.second.x = std::min(std::max(selection.second.x, scroll_.x), sizeCharacters_.x + scroll_.x);
            }

            size_t stringLength = 0;
            for (size_t y = scroll_.y; y < std::min(boxStrings_.size(), sizeCharacters_.y + scroll_.y); ++y) {
                if (y > scroll_.y) {
                    stringLength += 1;
                }
                size_t deltaLength = boxStrings_[y].substring(std::min(boxStrings_[y].getSize(), scroll_.x), sizeCharacters_.x).getSize();
                if (y == selection.first.y) {
                    if (y == selection.second.y) {
                        addSelectionLine(stringLength + selection.first.x - scroll_.x, stringLength + selection.second.x - scroll_.x);
                    } else {
                        addSelectionLine(stringLength + selection.first.x - scroll_.x, stringLength + deltaLength);
                    }
                } else if (y > selection.first.y && y < selection.second.y) {
                    addSelectionLine(stringLength, stringLength + deltaLength);
                } else if (y == selection.second.y) {
                    addSelectionLine(stringLength, stringLength + selection.second.x - scroll_.x);
                }
                stringLength += deltaLength;
            }
            std::cout << "MultilineTextBox::updateCaretPosition(), selection from (" << selectionStart_.first.x << ", " << selectionStart_.first.y << ") to (" << selectionEnd_.x << ", " << selectionEnd_.y << ") with " << selectionLines_.size() << " lines.\n";
        } else {
            selectionStart_.second = false;
            selectionLines_.clear();
        }
    }

    std::cout << "MultilineTextBox::updateCaretPosition(), caretPosition_ = (" << caretPosition_.x << ", " << caretPosition_.y << "), visibleCaretOffset = " << visibleCaretOffset << ", scroll_ = (" << scroll_.x << ", " << scroll_.y << ")\n";
    /*std::cout << "boxStrings_ =\n";
    for (size_t i = 0; i < boxStrings_.size(); ++i) {
        std::cout << i << ": [" << boxStrings_[i].toAnsiString() << "]\n";
    }*/
    requestRedraw();
}

void MultilineTextBox::updateCaretPosition(size_t caretOffset, bool continueSelection) {
    updateCaretPosition(findCaretPosition(caretOffset), continueSelection);
}

void MultilineTextBox::updateScroll(bool vertical, int delta, bool continueSelection) {
    if (vertical && delta >= 0) {
        while (delta != 0 && scroll_.y > 0) {
            --scroll_.y;
            --delta;
            if (caretPosition_.y >= scroll_.y + sizeCharacters_.y) {
                updateCaretPosition(findCaretOffset({caretPosition_.x, caretPosition_.y - 1}), continueSelection);
            } else {
                updateCaretPosition(caretPosition_, continueSelection);
            }
        }
    } else if (vertical && delta < 0) {
        while (delta != 0 && scroll_.y + sizeCharacters_.y < boxStrings_.size()) {
            ++scroll_.y;
            ++delta;
            if (caretPosition_.y < scroll_.y) {
                updateCaretPosition(findCaretOffset({caretPosition_.x, caretPosition_.y + 1}), continueSelection);
            } else {
                updateCaretPosition(caretPosition_, continueSelection);
            }
        }
    } else if (delta >= 0) {
        while (delta != 0 && scroll_.x > 0) {
            --scroll_.x;
            --delta;
            if (caretPosition_.x > scroll_.x + sizeCharacters_.x) {
                updateCaretPosition(findCaretOffset({caretPosition_.x - 1, caretPosition_.y}), continueSelection);
            } else {
                updateCaretPosition(caretPosition_, continueSelection);
            }
        }
    } else {
        size_t longestLength = findLongestLength(boxStrings_);
        while (delta != 0 && scroll_.x + sizeCharacters_.x < longestLength) {
            ++scroll_.x;
            ++delta;
            if (caretPosition_.x < scroll_.x) {
                updateCaretPosition(findCaretOffset({caretPosition_.x + 1, caretPosition_.y}), continueSelection);
            } else {
                updateCaretPosition(caretPosition_, continueSelection);
            }
        }
    }
}

sf::Vector2<size_t> MultilineTextBox::findCaretPosition(size_t caretOffset) const {
    size_t y = 0;
    size_t caretRemaining = caretOffset;
    while (caretRemaining > boxStrings_[y].getSize()) {
        caretRemaining -= boxStrings_[y].getSize() + 1;
        ++y;
    }
    return {caretRemaining, y};
}

size_t MultilineTextBox::findCaretOffset(const sf::Vector2<size_t>& caretPosition) const {
    size_t pos = 0;
    size_t y = std::min(caretPosition.y, boxStrings_.size() - 1);
    for (size_t i = 0; i < y; ++i) {
        pos += boxStrings_[i].getSize() + 1;
    }
    return pos + std::min(caretPosition.x, boxStrings_[y].getSize());
}

size_t MultilineTextBox::findClosestOffsetToMouse(const sf::Vector2f& mouseLocal) const {
    style_->text_.setString(visibleString_);
    style_->text_.setPosition(style_->textPadding_.x, style_->textPadding_.y);

    std::vector<size_t> firstIndexInLine = {0};
    for (size_t i = 0; i < visibleString_.getSize(); ++i) {
        if (visibleString_[i] == '\n') {
            firstIndexInLine.push_back(i + 1);
        }
    }
    /*std::cout << "firstIndexInLine: ";
    for (auto x : firstIndexInLine) {
        std::cout << x << " ";
    }
    std::cout << "\n";*/

    sf::Vector2<size_t> closestPos = {0, 0};
    size_t left, count;

    // Binary search for the closest y position.
    left = 0;
    count = firstIndexInLine.size() - 1;
    while (count > 0) {
        sf::Vector2f charPos = style_->text_.findCharacterPos(firstIndexInLine[left + count / 2]);
        float localY = charPos.y - getOrigin().y + style_->textPadding_.z * style_->getCharacterSize();
        if (localY < mouseLocal.y) {
            left = left + count / 2 + 1;
            count -= count / 2 + 1;
        } else {
            count = count / 2;
        }
    }
    closestPos.y = left;

    // Binary search for the closest x position.
    left = firstIndexInLine[closestPos.y];
    size_t nextNewline = visibleString_.find("\n", firstIndexInLine[closestPos.y]);
    count = (nextNewline == sf::String::InvalidPos ? visibleString_.getSize() : nextNewline) - firstIndexInLine[closestPos.y];
    while (count > 0) {
        sf::Vector2f charPos = style_->text_.findCharacterPos(left + count / 2);
        float localX = charPos.x - getOrigin().x;
        //std::cout << "checking pos " << left + count / 2 << ", localX = " << localX << ", mouseLocal.x = " << mouseLocal.x << "\n";
        if (localX < mouseLocal.x) {
            left = left + count / 2 + 1;
            count -= count / 2 + 1;
        } else {
            count = count / 2;
        }
    }
    // Final check against current and previous character to decide closest.
    if (left > firstIndexInLine[closestPos.y]) {
        //std::cout << "do final check\n";
        float localX = style_->text_.findCharacterPos(left).x - getOrigin().x;
        float localXPrevious = style_->text_.findCharacterPos(left - 1).x - getOrigin().x;
        if (mouseLocal.x < localXPrevious + (localX - localXPrevious) / 2.0f) {
            --left;
        }
    }
    closestPos.x = left - firstIndexInLine[closestPos.y];

    std::cout << "mouseLocal = (" << mouseLocal.x << ", " << mouseLocal.y << "), closest = (" << closestPos.x << ", " << closestPos.y << ")\n";
    return findCaretOffset(closestPos + scroll_);
}

void MultilineTextBox::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (!isVisible()) {
        return;
    }
    states.transform *= getTransform();

    style_->box_.setSize(size_);
    style_->box_.setFillColor(readOnly_ ? style_->readOnlyBoxColor_ : style_->boxColor_);
    target.draw(style_->box_, states);
    if (boxStrings_.size() == 1 && boxStrings_[0].isEmpty() && (!isFocused() || readOnly_)) {
        sf::String defaultString = "";
        for (size_t i = 0; i < std::min(defaultStrings_.size(), sizeCharacters_.y); ++i) {
            defaultString += (i > 0 ? "\n" : "") + defaultStrings_[i].substring(0, sizeCharacters_.x);
        }
        style_->text_.setString(defaultString);
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

    if (isFocused()) {
        for (auto& line : selectionLines_) {
            line.setFillColor(style_->highlightColor_);
            target.draw(line, states);
        }
        if (!readOnly_) {
            style_->caret_.setPosition(caretDrawPosition_);
            target.draw(style_->caret_, states);
        }
    }
}

}
