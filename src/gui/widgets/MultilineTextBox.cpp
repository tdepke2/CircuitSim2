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

    updateCaretPosition(0);
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
    updateCaretPosition(0);
}
void MultilineTextBox::setDefaultText(const sf::String& text) {
    defaultStrings_ = splitString(text);
    updateCaretPosition(0);
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
void MultilineTextBox::handleMousePress(sf::Mouse::Button button, const sf::Vector2f& mouseParent) {
    Widget::handleMousePress(button, mouseParent);
    const auto mouseLocal = toLocalOriginSpace(mouseParent);
    if (button <= sf::Mouse::Button::Middle) {
        onClick.emit(this, mouseLocal);
    }
    onMousePress.emit(this, button, mouseLocal);

    // FIXME
    /*style_->text_.setString(visibleString_);
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
    updateCaretPosition(closestIndex + scroll_.x);*/

    updateCaretPosition(0);
}
void MultilineTextBox::handleMouseRelease(sf::Mouse::Button button, const sf::Vector2f& mouseParent) {
    onMouseRelease.emit(this, button, toLocalOriginSpace(mouseParent));
    Widget::handleMouseRelease(button, mouseParent);
}
void MultilineTextBox::handleTextEntered(uint32_t unicode) {
    Widget::handleTextEntered(unicode);
    if (!readOnly_ && unicode >= '\u0020' && unicode <= '\u007e') {    // Printable character.
        if (maxCharacters_ == 0 || findStringsLength(boxStrings_) < maxCharacters_) {
            boxStrings_[caretPosition_.y].insert(caretPosition_.x, sf::String(unicode));
            updateCaretPosition(findCaretOffset(caretPosition_) + 1);
        }
    }
}
void MultilineTextBox::handleKeyPressed(sf::Keyboard::Key key) {
    Widget::handleKeyPressed(key);
    size_t caretOffset = findCaretOffset(caretPosition_);
    if (key == sf::Keyboard::Enter) {
        if (!readOnly_ && (maxCharacters_ == 0 || findStringsLength(boxStrings_) < maxCharacters_)) {
            boxStrings_.emplace(boxStrings_.begin() + caretPosition_.y + 1, boxStrings_[caretPosition_.y].substring(caretPosition_.x));
            boxStrings_[caretPosition_.y] = boxStrings_[caretPosition_.y].substring(0, caretPosition_.x);
            updateCaretPosition(caretOffset + 1);
        }
    } else if (key == sf::Keyboard::Backspace) {
        if (!readOnly_ && caretOffset > 0) {
            if (caretPosition_.x == 0) {
                boxStrings_[caretPosition_.y - 1] += boxStrings_[caretPosition_.y];
                boxStrings_.erase(boxStrings_.begin() + caretPosition_.y);
            } else {
                boxStrings_[caretPosition_.y].erase(caretPosition_.x - 1, 1);
            }
            if (scroll_.x > 0 && scroll_.x + sizeCharacters_.x > findLongestLength(boxStrings_)) {
                --scroll_.x;
            }
            updateCaretPosition(caretOffset - 1);
        }
    } else if (key == sf::Keyboard::Delete) {
        if (!readOnly_ && caretOffset < findStringsLength(boxStrings_)) {
            if (caretPosition_.x == boxStrings_[caretPosition_.y].getSize()) {
                boxStrings_[caretPosition_.y] += boxStrings_[caretPosition_.y + 1];
                boxStrings_.erase(boxStrings_.begin() + caretPosition_.y + 1);
            } else {
                boxStrings_[caretPosition_.y].erase(caretPosition_.x, 1);
            }
            if (scroll_.x > 0 && scroll_.x + sizeCharacters_.x > findLongestLength(boxStrings_)) {
                --scroll_.x;
            }
            updateCaretPosition(caretPosition_);
        }
    } else if (key == sf::Keyboard::End && caretOffset != findStringsLength(boxStrings_)) {
        updateCaretPosition(findStringsLength(boxStrings_));
    } else if (key == sf::Keyboard::Home && caretOffset != 0) {
        updateCaretPosition(0);
    } else if (key == sf::Keyboard::Up) {
        if (caretPosition_.y > 0) {
            updateCaretPosition(findCaretOffset({caretPosition_.x, caretPosition_.y - 1}));
        } else if (caretPosition_.x > 0) {
            updateCaretPosition(0);
        }
    } else if (key == sf::Keyboard::Down) {
        if (caretPosition_.y < boxStrings_.size() - 1) {
            updateCaretPosition(findCaretOffset({caretPosition_.x, caretPosition_.y + 1}));
        } else if (caretPosition_.x < boxStrings_[caretPosition_.y].getSize()) {
            updateCaretPosition(findStringsLength(boxStrings_));
        }
    } else if (key == sf::Keyboard::Left) {
        //if (readOnly_ && scroll_.x > 0) {
        //    updateCaretPosition(scroll_.x - 1);
        //} else if (!readOnly_ && caretPosition_ > 0) {
        //    updateCaretPosition(caretPosition_ - 1);
        //}
        if (caretOffset > 0) {
            updateCaretPosition(caretOffset - 1);
        }
    } else if (key == sf::Keyboard::Right) {
        //if (readOnly_ && scroll_.x + sizeCharacters_.x < boxString_.getSize()) {
        //    updateCaretPosition(scroll_.x + sizeCharacters_.x + 1);
        //} else if (!readOnly_ && caretPosition_ < boxString_.getSize()) {
        //    updateCaretPosition(caretPosition_ + 1);
        //}
        if (caretOffset < findStringsLength(boxStrings_)) {
            updateCaretPosition(caretOffset + 1);
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

    updateCaretPosition(0);
}

void MultilineTextBox::updateCaretPosition(const sf::Vector2<size_t>& caretPosition) {
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
    for (size_t i = scroll_.y; i < std::min(boxStrings_.size(), sizeCharacters_.y + scroll_.y); ++i) {
        if (i > scroll_.y) {
            visibleString_ += "\n";
        }
        if (i == caretPosition_.y) {
            visibleCaretOffset = visibleString_.getSize() + caretPosition_.x - scroll_.x;
        }
        visibleString_ += boxStrings_[i].substring(std::min(boxStrings_[i].getSize(), scroll_.x), sizeCharacters_.x);
    }

    style_->text_.setString(visibleString_);
    style_->text_.setPosition(style_->textPadding_.x, style_->textPadding_.y);
    caretDrawPosition_ = style_->text_.findCharacterPos(visibleCaretOffset);

    std::cout << "MultilineTextBox::updateCaretPosition(), caretPosition_ = (" << caretPosition_.x << ", " << caretPosition_.y << "), visibleCaretOffset = " << visibleCaretOffset << ", scroll_ = (" << scroll_.x << ", " << scroll_.y << ")\n";
    std::cout << "boxStrings_ =\n";
    for (size_t i = 0; i < boxStrings_.size(); ++i) {
        std::cout << i << ": [" << boxStrings_[i].toAnsiString() << "]\n";
    }
    requestRedraw();
}

void MultilineTextBox::updateCaretPosition(size_t caretOffset) {
    updateCaretPosition(findCaretPosition(caretOffset));
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
        style_->text_.setString(defaultString_.substring(0, sizeCharacters_.x));
        style_->text_.setFillColor(style_->defaultTextColor_);
    } else {
        style_->text_.setString(visibleString_);
        style_->text_.setFillColor(style_->textColor_);
    }
    style_->text_.setPosition(style_->textPadding_.x, style_->textPadding_.y);
    target.draw(style_->text_, states);
    if (isFocused() && !readOnly_) {
        style_->caret_.setPosition(caretDrawPosition_);
        target.draw(style_->caret_, states);
    }
}

}
