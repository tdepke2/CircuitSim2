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

std::shared_ptr<MultilineTextBox> MultilineTextBox::create(const Theme& theme) {
    return std::shared_ptr<MultilineTextBox>(new MultilineTextBox(theme.getTextBoxStyle()));
}
std::shared_ptr<MultilineTextBox> MultilineTextBox::create(std::shared_ptr<TextBoxStyle> style) {
    return std::shared_ptr<MultilineTextBox>(new MultilineTextBox(style));
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
void MultilineTextBox::setReadOnly(bool readOnly) {
    readOnly_ = readOnly;
    requestRedraw();
}
void MultilineTextBox::setText(const sf::String& text) {
    boxStrings_ = splitString(text);
    updateCaretPosition(0, false);
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
bool MultilineTextBox::getReadOnly() const {
    return readOnly_;
}
sf::String MultilineTextBox::getText() const {
    return combineStrings(boxStrings_);
}
sf::String MultilineTextBox::getDefaultText() const {
    return combineStrings(defaultStrings_);
}

void MultilineTextBox::setStyle(std::shared_ptr<TextBoxStyle> style) {
    style_ = style;
    styleCopied_ = false;
    requestRedraw();
}
std::shared_ptr<TextBoxStyle> MultilineTextBox::getStyle() {
    if (!styleCopied_) {
        style_ = style_->clone();
        styleCopied_ = true;
    }
    return style_;
}

sf::FloatRect MultilineTextBox::getLocalBounds() const {
    return {-getOrigin(), size_};
}
void MultilineTextBox::handleMouseMove(const sf::Vector2f& mouseParent) {
    Widget::handleMouseMove(mouseParent);
    const auto mouseLocal = toLocalOriginSpace(mouseParent);

}
void MultilineTextBox::handleMousePress(sf::Mouse::Button button, const sf::Vector2f& mouseParent) {
    Widget::handleMousePress(button, mouseParent);
    const auto mouseLocal = toLocalOriginSpace(mouseParent);
    if (button <= sf::Mouse::Button::Middle) {
        onClick.emit(this, mouseLocal);
    }
    onMousePress.emit(this, button, mouseLocal);

    style_->text_.setString(visibleString_);
    style_->text_.setPosition(style_->textPadding_.x, style_->textPadding_.y);
    sf::Vector2<size_t> pos = {0, 0};
    sf::Vector2<size_t> closestPos = {0, 0};
    sf::Vector2f closestDistance = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
    for (size_t i = 0; i <= visibleString_.getSize(); ++i) {
        sf::Vector2f charPos = style_->text_.findCharacterPos(i);
        sf::Vector2f distance = {
            static_cast<float>(fabs(mouseLocal.x + getOrigin().x - charPos.x)),
            static_cast<float>(fabs(mouseLocal.y + getOrigin().y - charPos.y - style_->textPadding_.z * style_->getCharacterSize() * 0.5f))
        };
        if (distance.y < closestDistance.y || (distance.y == closestDistance.y && distance.x < closestDistance.x)) {
            closestPos = pos;
            closestDistance = distance;
        }
        if (i < visibleString_.getSize() && visibleString_[i] == '\n') {
            pos.x = 0;
            ++pos.y;
        } else {
            ++pos.x;
        }
    }
    std::cout << "mouseLocal = (" << mouseLocal.x << ", " << mouseLocal.y << "), closest = (" << closestPos.x << ", " << closestPos.y << "), dist = (" << closestDistance.x << ", " << closestDistance.y << ")\n";
    updateCaretPosition(findCaretOffset(closestPos + scroll_), false);
}
void MultilineTextBox::handleMouseRelease(sf::Mouse::Button button, const sf::Vector2f& mouseParent) {
    onMouseRelease.emit(this, button, toLocalOriginSpace(mouseParent));
    Widget::handleMouseRelease(button, mouseParent);
}
void MultilineTextBox::handleTextEntered(uint32_t unicode) {
    Widget::handleTextEntered(unicode);
    if (unicode >= '\u0020' && unicode <= '\u007e') {    // Printable character.
        insertCharacter(unicode);
    }
}
void MultilineTextBox::handleKeyPressed(sf::Keyboard::Key key) {
    Widget::handleKeyPressed(key);
    size_t caretOffset = findCaretOffset(caretPosition_);
    const bool shiftPressed = (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::RShift));

    if (key == sf::Keyboard::Enter) {
        insertCharacter('\u000a');
    } else if (key == sf::Keyboard::Backspace) {
        insertCharacter('\u0008');
    } else if (key == sf::Keyboard::Delete) {
        insertCharacter('\u007f');
    } else if (key == sf::Keyboard::PageUp) {
        updateCaretPosition(0, shiftPressed);
    } else if (key == sf::Keyboard::PageDown) {
        updateCaretPosition(findStringsLength(boxStrings_), shiftPressed);
    } else if (key == sf::Keyboard::Home) {
        updateCaretPosition({0, caretPosition_.y}, shiftPressed);
    } else if (key == sf::Keyboard::End) {
        updateCaretPosition({boxStrings_[caretPosition_.y].getSize(), caretPosition_.y}, shiftPressed);
    } else if (key == sf::Keyboard::Up) {
        if (caretPosition_.y > 0) {
            updateCaretPosition(findCaretOffset({caretPosition_.x, caretPosition_.y - 1}), shiftPressed);
        } else {
            updateCaretPosition(0, shiftPressed);
        }
    } else if (key == sf::Keyboard::Down) {
        if (caretPosition_.y < boxStrings_.size() - 1) {
            updateCaretPosition(findCaretOffset({caretPosition_.x, caretPosition_.y + 1}), shiftPressed);
        } else {
            updateCaretPosition(findStringsLength(boxStrings_), shiftPressed);
        }
    } else if (key == sf::Keyboard::Left) {
        if (selectionStart_.second && !shiftPressed) {
            updateCaretPosition(sortByYFirst(selectionStart_.first, selectionEnd_).first, false);
        } else if (caretOffset > 0) {
            updateCaretPosition(caretOffset - 1, shiftPressed);
        }
    } else if (key == sf::Keyboard::Right) {
        if (selectionStart_.second && !shiftPressed) {
            updateCaretPosition(sortByYFirst(selectionStart_.first, selectionEnd_).second, false);
        } else if (caretOffset < findStringsLength(boxStrings_)) {
            updateCaretPosition(caretOffset + 1, shiftPressed);
        }
    }
}

MultilineTextBox::MultilineTextBox(std::shared_ptr<TextBoxStyle> style) :
    style_(style),
    styleCopied_(false),
    sizeCharacters_(0, 0),
    maxCharacters_(0),
    readOnly_(false),
    boxStrings_{""},
    defaultStrings_{""},
    scroll_(0, 0) {

    updateCaretPosition(0, false);
}

void MultilineTextBox::insertCharacter(uint32_t unicode) {
    if (readOnly_) {
        return;
    }
    size_t caretOffset = findCaretOffset(caretPosition_);
    bool clearedSelection = false;
    if (selectionStart_.second) {
        auto selection = sortByYFirst(selectionStart_.first, selectionEnd_);
        for (size_t y = selection.first.y; y <= selection.second.y; ++y) {
            if (y == selection.first.y) {
                if (y == selection.second.y) {
                    boxStrings_[selection.first.y] = boxStrings_[selection.first.y].substring(0, selection.first.x) + boxStrings_[selection.first.y].substring(selection.second.x);
                } else {
                    boxStrings_[selection.first.y] = boxStrings_[selection.first.y].substring(0, selection.first.x);
                }
            } else if (y > selection.first.y && y < selection.second.y) {
                boxStrings_.erase(boxStrings_.begin() + selection.first.y + 1);
            } else if (y == selection.second.y) {
                boxStrings_[selection.first.y] += boxStrings_[selection.first.y].substring(selection.second.x);
                boxStrings_.erase(boxStrings_.begin() + selection.first.y + 1);    // FIXME: need to check this, indexing issue?
            }
        }
        if (selectionStart_.first != selectionEnd_) {
            clearedSelection = true;
        }
        updateCaretPosition(selection.first, false);
    }

    if (unicode >= '\u0020' && unicode <= '\u007e') {    // Printable character.
        if (maxCharacters_ == 0 || findStringsLength(boxStrings_) < maxCharacters_) {
            boxStrings_[caretPosition_.y].insert(caretPosition_.x, sf::String(unicode));
            updateCaretPosition(findCaretOffset(caretPosition_) + 1, false);
        }
    } else if (unicode == '\u000a') {    // Enter key.
        if (maxCharacters_ == 0 || findStringsLength(boxStrings_) < maxCharacters_) {
            boxStrings_.emplace(boxStrings_.begin() + caretPosition_.y + 1, boxStrings_[caretPosition_.y].substring(caretPosition_.x));
            boxStrings_[caretPosition_.y] = boxStrings_[caretPosition_.y].substring(0, caretPosition_.x);
            updateCaretPosition(caretOffset + 1, false);
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
        }
    }
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
            std::cout << "MultilineTextBox::updateCaretPosition(), selection from (" << selectionStart_.first.x << ", " << selectionStart_.first.y << ") to (" << selectionEnd_.x << ", " << selectionEnd_.y << ")\n";
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
        } else {
            selectionStart_.second = false;
            selectionLines_.clear();
        }
    }

    std::cout << "MultilineTextBox::updateCaretPosition(), caretPosition_ = (" << caretPosition_.x << ", " << caretPosition_.y << "), visibleCaretOffset = " << visibleCaretOffset << ", scroll_ = (" << scroll_.x << ", " << scroll_.y << ")\n";
    std::cout << "boxStrings_ =\n";
    for (size_t i = 0; i < boxStrings_.size(); ++i) {
        std::cout << i << ": [" << boxStrings_[i].toAnsiString() << "]\n";
    }
    requestRedraw();
}

void MultilineTextBox::updateCaretPosition(size_t caretOffset, bool continueSelection) {
    updateCaretPosition(findCaretPosition(caretOffset), continueSelection);
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

void MultilineTextBox::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (!isVisible()) {
        return;
    }
    states.transform *= getTransform();

    style_->box_.setSize(size_);
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
    style_->text_.setPosition(style_->textPadding_.x, style_->textPadding_.y);
    target.draw(style_->text_, states);

    if (isFocused()) {
        for (auto& line : selectionLines_) {
            line.setFillColor({90, 90, 150, 100});    // FIXME: need new style field, also have a different bg and text color for read-only?
            target.draw(line, states);
        }
        if (!readOnly_) {
            style_->caret_.setPosition(caretDrawPosition_);
            target.draw(style_->caret_, states);
        }
    }
}

}
