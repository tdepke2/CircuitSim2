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

size_t getStringsLength(const std::vector<sf::String>& stringArray) {
    size_t length = 0;
    for (const auto& x : stringArray) {
        length += x.getSize() + 1;
    }
    return (length > 0 ? length - 1 : length);
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
    const auto rowCol = findCaretRowColumn();
    if (!readOnly_ && unicode >= '\u0020' && unicode <= '\u007e') {    // Printable character.
        if (maxCharacters_ == 0 || getStringsLength(boxStrings_) < maxCharacters_) {
            boxStrings_[rowCol.first].insert(rowCol.second, sf::String(unicode));
            updateCaretPosition(caretPosition_ + 1);
        }
    }
}
void MultilineTextBox::handleKeyPressed(sf::Keyboard::Key key) {
    Widget::handleKeyPressed(key);
    const auto rowCol = findCaretRowColumn();
    if (key == sf::Keyboard::Enter) {
        if (!readOnly_ && (maxCharacters_ == 0 || getStringsLength(boxStrings_) < maxCharacters_)) {
            boxStrings_.emplace(boxStrings_.begin() + rowCol.first + 1, boxStrings_[rowCol.first].substring(rowCol.second));
            boxStrings_[rowCol.first] = boxStrings_[rowCol.first].substring(0, rowCol.second);
            updateCaretPosition(caretPosition_ + 1);
        }
    } else if (key == sf::Keyboard::Backspace) {
        if (!readOnly_ && caretPosition_ > 0) {
            //if (scroll_.x > 0 && scroll_.x + sizeCharacters_.x >= boxStrings_[rowCol.first].getSize()) {
            //    --scroll_.x;
            //}
            if (rowCol.second == 0) {
                boxStrings_[rowCol.first - 1] += boxStrings_[rowCol.first];
                boxStrings_.erase(boxStrings_.begin() + rowCol.first);
            } else {
                boxStrings_[rowCol.first].erase(rowCol.second - 1, 1);
            }
            updateCaretPosition(caretPosition_ - 1);
        }
    } else if (key == sf::Keyboard::Delete) {
        if (!readOnly_ && caretPosition_ < getStringsLength(boxStrings_)) {
            //if (scroll_.x > 0 && scroll_.x + sizeCharacters_.x >= boxString_.getSize()) {
            //    --scroll_.x;
            //}
            if (rowCol.second == boxStrings_[rowCol.first].getSize()) {
                boxStrings_[rowCol.first] += boxStrings_[rowCol.first + 1];
                boxStrings_.erase(boxStrings_.begin() + rowCol.first + 1);
            } else {
                boxStrings_[rowCol.first].erase(rowCol.second, 1);
            }
            updateCaretPosition(caretPosition_);
        }
    } else if (key == sf::Keyboard::End && caretPosition_ != getStringsLength(boxStrings_)) {
        updateCaretPosition(getStringsLength(boxStrings_));
    } else if (key == sf::Keyboard::Home && caretPosition_ != 0) {
        updateCaretPosition(0);
    } else if (key == sf::Keyboard::Up) {
        if (rowCol.first > 0) {
            updateCaretPosition(findCaretPosition(rowCol.first - 1, rowCol.second));
        } else if (rowCol.second > 0) {
            updateCaretPosition(0);
        }
    } else if (key == sf::Keyboard::Down) {
        if (rowCol.first < boxStrings_.size() - 1) {
            updateCaretPosition(findCaretPosition(rowCol.first + 1, rowCol.second));
        } else if (rowCol.second < boxStrings_[rowCol.first].getSize()) {
            updateCaretPosition(getStringsLength(boxStrings_));
        }
    } else if (key == sf::Keyboard::Left) {
        //if (readOnly_ && scroll_.x > 0) {
        //    updateCaretPosition(scroll_.x - 1);
        //} else if (!readOnly_ && caretPosition_ > 0) {
        //    updateCaretPosition(caretPosition_ - 1);
        //}
        if (caretPosition_ > 0) {
            updateCaretPosition(caretPosition_ - 1);
        }
    } else if (key == sf::Keyboard::Right) {
        //if (readOnly_ && scroll_.x + sizeCharacters_.x < boxString_.getSize()) {
        //    updateCaretPosition(scroll_.x + sizeCharacters_.x + 1);
        //} else if (!readOnly_ && caretPosition_ < boxString_.getSize()) {
        //    updateCaretPosition(caretPosition_ + 1);
        //}
        if (caretPosition_ < getStringsLength(boxStrings_)) {
            updateCaretPosition(caretPosition_ + 1);
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

void MultilineTextBox::updateCaretPosition(size_t caretPosition) {
    caretPosition_ = caretPosition;
    auto rowCol = findCaretRowColumn();
    if (rowCol.second < scroll_.x) {
        scroll_.x = rowCol.second;
    } else if (rowCol.second > scroll_.x + sizeCharacters_.x) {
        scroll_.x = rowCol.second - sizeCharacters_.x;
    }
    if (rowCol.first < scroll_.y) {
        scroll_.y = rowCol.first;
    } else if (rowCol.first > scroll_.y + sizeCharacters_.y) {
        scroll_.y = rowCol.first - sizeCharacters_.y;
    }
    visibleString_ = "";
    size_t visibleCaretPos = 0;
    for (size_t i = scroll_.y; i < std::min(boxStrings_.size(), sizeCharacters_.y); ++i) {
        if (i > scroll_.y) {
            visibleString_ += "\n";
        }
        if (i == rowCol.first) {
            visibleCaretPos = visibleString_.getSize() + rowCol.second - scroll_.x;
        }
        visibleString_ += boxStrings_[i].substring(std::min(boxStrings_[i].getSize(), scroll_.x), sizeCharacters_.x);
    }

    style_->text_.setString(visibleString_);
    style_->text_.setPosition(style_->textPadding_.x, style_->textPadding_.y);
    caretDrawPosition_ = style_->text_.findCharacterPos(visibleCaretPos);

    std::cout << "MultilineTextBox::updateCaretPosition(), caretPosition_ = " << caretPosition_ << ", visibleCaretPos = " << visibleCaretPos << ", scroll_ = (" << scroll_.x << ", " << scroll_.y << ")\n";
    std::cout << "boxStrings_ =\n";
    for (size_t i = 0; i < boxStrings_.size(); ++i) {
        std::cout << i << ": [" << boxStrings_[i].toAnsiString() << "]\n";
    }
    requestRedraw();
}

std::pair<size_t, size_t> MultilineTextBox::findCaretRowColumn() const {
    size_t row = 0;
    size_t caretRemaining = caretPosition_;
    while (caretRemaining > boxStrings_[row].getSize()) {
        caretRemaining -= boxStrings_[row].getSize() + 1;
        ++row;
    }
    return {row, caretRemaining};
}

size_t MultilineTextBox::findCaretPosition(size_t row, size_t column) const {
    size_t pos = 0;
    row = std::min(row, boxStrings_.size() - 1);
    for (size_t i = 0; i < row; ++i) {
        pos += boxStrings_[i].getSize() + 1;
    }
    return pos + std::min(column, boxStrings_[row].getSize());
}

void MultilineTextBox::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (!isVisible()) {
        return;
    }
    states.transform *= getTransform();

    style_->box_.setSize(size_);
    target.draw(style_->box_, states);
    // FIXME
    //if (boxString_.isEmpty() && !defaultString_.isEmpty() && (!isFocused() || readOnly_)) {
    //    style_->text_.setString(defaultString_.substring(0, sizeCharacters_.x));
    //    style_->text_.setFillColor(style_->defaultTextColor_);
    //} else {
        style_->text_.setString(visibleString_);
        style_->text_.setFillColor(style_->textColor_);
    //}
    style_->text_.setPosition(style_->textPadding_.x, style_->textPadding_.y);
    target.draw(style_->text_, states);
    if (isFocused() && !readOnly_) {
        style_->caret_.setPosition(caretDrawPosition_);
        target.draw(style_->caret_, states);
    }
}

}
