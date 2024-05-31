#include <gui/Debug.h>
#include <gui/Gui.h>
#include <gui/Theme.h>
#include <gui/widgets/Button.h>
#include <gui/widgets/DialogBox.h>
#include <gui/widgets/Label.h>
#include <gui/widgets/MultilineTextBox.h>
#include <gui/widgets/TextBox.h>

#include <limits>

namespace gui {

DialogBoxStyle::DialogBoxStyle(const Gui& gui) :
    Style(gui) {
}

// sf::Shape interface.
void DialogBoxStyle::setTexture(const sf::Texture* texture, bool resetRect) {
    rect_.setTexture(texture, resetRect);
    gui_.requestRedraw();
}
void DialogBoxStyle::setTextureRect(const sf::IntRect& rect) {
    rect_.setTextureRect(rect);
    gui_.requestRedraw();
}
void DialogBoxStyle::setFillColor(const sf::Color& color) {
    rect_.setFillColor(color);
    gui_.requestRedraw();
}
void DialogBoxStyle::setOutlineColor(const sf::Color& color) {
    rect_.setOutlineColor(color);
    gui_.requestRedraw();
}
void DialogBoxStyle::setOutlineThickness(float thickness) {
    rect_.setOutlineThickness(thickness);
    gui_.requestRedraw();
}
const sf::Texture* DialogBoxStyle::getTexture() const {
    return rect_.getTexture();
}
const sf::IntRect& DialogBoxStyle::getTextureRect() const {
    return rect_.getTextureRect();
}
const sf::Color& DialogBoxStyle::getFillColor() const {
    return rect_.getFillColor();
}
const sf::Color& DialogBoxStyle::getOutlineColor() const {
    return rect_.getOutlineColor();
}
float DialogBoxStyle::getOutlineThickness() const {
    return rect_.getOutlineThickness();
}

void DialogBoxStyle::setTitleBarTexture(const sf::Texture* texture, bool resetRect) {
    titleBar_.setTexture(texture, resetRect);
    gui_.requestRedraw();
}
void DialogBoxStyle::setTitleBarTextureRect(const sf::IntRect& rect) {
    titleBar_.setTextureRect(rect);
    gui_.requestRedraw();
}
void DialogBoxStyle::setTitleBarFillColor(const sf::Color& color) {
    titleBar_.setFillColor(color);
    gui_.requestRedraw();
}
void DialogBoxStyle::setTitleBarOutlineColor(const sf::Color& color) {
    titleBar_.setOutlineColor(color);
    gui_.requestRedraw();
}
void DialogBoxStyle::setTitleBarOutlineThickness(float thickness) {
    titleBar_.setOutlineThickness(thickness);
    gui_.requestRedraw();
}
const sf::Texture* DialogBoxStyle::getTitleBarTexture() const {
    return titleBar_.getTexture();
}
const sf::IntRect& DialogBoxStyle::getTitleBarTextureRect() const {
    return titleBar_.getTextureRect();
}
const sf::Color& DialogBoxStyle::getTitleBarFillColor() const {
    return titleBar_.getFillColor();
}
const sf::Color& DialogBoxStyle::getTitleBarOutlineColor() const {
    return titleBar_.getOutlineColor();
}
float DialogBoxStyle::getTitleBarOutlineThickness() const {
    return titleBar_.getOutlineThickness();
}

void DialogBoxStyle::setTitleBarHeight(float height) {
    titleBarHeight_ = height;
    gui_.requestRedraw();
}
void DialogBoxStyle::setTitlePadding(const sf::Vector2f& titlePadding) {
    titlePadding_ = titlePadding;
    gui_.requestRedraw();
}
void DialogBoxStyle::setButtonPadding(const sf::Vector2f& buttonPadding) {
    buttonPadding_ = buttonPadding;
    gui_.requestRedraw();
}
float DialogBoxStyle::getTitleBarHeight() const {
    return titleBarHeight_;
}
const sf::Vector2f& DialogBoxStyle::getTitlePadding() const {
    return titlePadding_;
}
const sf::Vector2f& DialogBoxStyle::getButtonPadding() const {
    return buttonPadding_;
}

std::shared_ptr<DialogBoxStyle> DialogBoxStyle::clone() const {
    return std::make_shared<DialogBoxStyle>(*this);
}



std::shared_ptr<DialogBox> DialogBox::create(const Theme& theme, const sf::String& name) {
    return std::shared_ptr<DialogBox>(new DialogBox(theme.getStyle<DialogBoxStyle>("DialogBox"), name));
}
std::shared_ptr<DialogBox> DialogBox::create(std::shared_ptr<DialogBoxStyle> style, const sf::String& name) {
    return std::shared_ptr<DialogBox>(new DialogBox(style, name));
}

void DialogBox::setSize(const sf::Vector2f& size) {
    size_ = size;
    updateTitle();
    updateButtons();
    requestRedraw();
}
void DialogBox::setDraggable(bool draggable) {
    draggable_ = draggable;
}
void DialogBox::setTitle(std::shared_ptr<Label> title) {
    if (!hasChild(title)) {
        addChild(title);
    }
    title->setFocusable(false);
    title_ = title;
    updateTitle();
}
void DialogBox::setSubmitButton(size_t index, std::shared_ptr<Button> button) {
    setOptionButton(index, button);
    submitIndex_ = (button != nullptr ? index : std::numeric_limits<size_t>::max());
}
void DialogBox::setCancelButton(size_t index, std::shared_ptr<Button> button) {
    setOptionButton(index, button);
    cancelIndex_ = (button != nullptr ? index : std::numeric_limits<size_t>::max());
}
void DialogBox::setOptionButton(size_t index, std::shared_ptr<Button> button) {
    if (button != nullptr) {
        if (!hasChild(button)) {
            addChild(button);
        }
        if (index >= optionButtons_.size()) {
            optionButtons_.resize(index + 1);
        }
        optionButtons_[index] = button;
    } else if (index < optionButtons_.size()) {
        removeChild(optionButtons_[index]);
        optionButtons_[index] = nullptr;
        bool foundButton = false;
        for (size_t i = index; i < optionButtons_.size(); ++i) {
            if (optionButtons_[i] != nullptr) {
                foundButton = true;
                break;
            }
        }
        if (!foundButton) {
            optionButtons_.resize(index);
        }
    }
    updateButtons();
}
void DialogBox::setTitleAlignment(Alignment titleAlignment) {
    titleAlignment_ = titleAlignment;
    updateTitle();
}
void DialogBox::setButtonAlignment(Alignment buttonAlignment) {
    buttonAlignment_ = buttonAlignment;
    updateButtons();
}
const sf::Vector2f& DialogBox::getSize() const {
    return size_;
}
bool DialogBox::isDraggable() const {
    return draggable_;
}
std::shared_ptr<Label> DialogBox::getTitle() const {
    return title_;
}
std::shared_ptr<Button> DialogBox::getSubmitButton() const {
    return getOptionButton(submitIndex_);
}
std::shared_ptr<Button> DialogBox::getCancelButton() const {
    return getOptionButton(cancelIndex_);
}
std::shared_ptr<Button> DialogBox::getOptionButton(size_t index) const {
    if (index < optionButtons_.size()) {
        return optionButtons_[index];
    }
    return nullptr;
}
DialogBox::Alignment DialogBox::getTitleAlignment() const {
    return titleAlignment_;
}
DialogBox::Alignment DialogBox::getButtonAlignment() const {
    return buttonAlignment_;
}

void DialogBox::setVisible(bool visible) {
    if (visible) {
        focusNextTextBox();
    }
    baseClass::setVisible(visible);
}

void DialogBox::setStyle(std::shared_ptr<DialogBoxStyle> style) {
    style_ = style;
    styleCopied_ = false;
    requestRedraw();
}
std::shared_ptr<DialogBoxStyle> DialogBox::getStyle() {
    if (!styleCopied_) {
        style_ = style_->clone();
        styleCopied_ = true;
    }
    return style_;
}

sf::FloatRect DialogBox::getLocalBounds() const {
    return {-getOrigin(), size_};
}
bool DialogBox::isMouseIntersecting(const sf::Vector2f& mouseParent) const {
    return Widget::isMouseIntersecting(mouseParent);
}

bool DialogBox::handleMouseMove(const sf::Vector2f& mouseParent) {
    if (baseClass::handleMouseMove(mouseParent)) {
        return true;
    }

    if (dragPoint_.second) {
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            // Clamp the mouse position to the window area.
            sf::Vector2f mouseAbsolute = toGuiSpace(mouseParent);
            mouseAbsolute.x = std::min(std::max(mouseAbsolute.x, 0.0f), static_cast<float>(getGui()->getSize().x));
            mouseAbsolute.y = std::min(std::max(mouseAbsolute.y, 0.0f), static_cast<float>(getGui()->getSize().y));

            setPosition(fromGuiSpace(mouseAbsolute) - dragPoint_.first + initialPosition_);
        } else {
            dragPoint_.second = false;
        }
    }

    return isMouseIntersecting(mouseParent) && (isFocused() || findChildWithFocus() != nullptr);
}
void DialogBox::handleMousePress(sf::Mouse::Button button, const sf::Vector2f& mouseParent) {
    baseClass::handleMousePress(button, mouseParent);
    const auto mouseLocal = toLocalOriginSpace(mouseParent);
    if (button == sf::Mouse::Left && draggable_ && mouseLocal.y <= style_->titleBarHeight_ - style_->rect_.getOutlineThickness()) {
        dragPoint_ = {mouseParent, true};
        initialPosition_ = getPosition();
    }
}
void DialogBox::handleMouseRelease(sf::Mouse::Button button, const sf::Vector2f& mouseParent) {
    if (button == sf::Mouse::Left) {
        dragPoint_.second = false;
    }
    baseClass::handleMouseRelease(button, mouseParent);
}
bool DialogBox::handleKeyPressed(const sf::Event::KeyEvent& key) {
    bool eventConsumed = baseClass::handleKeyPressed(key);
    if (key.code == sf::Keyboard::Enter) {
        const auto& submitButton = getSubmitButton();
        if (submitButton != nullptr) {
            submitButton->onClick.emit(submitButton.get(), sf::Vector2f(0.0f, 0.0f));
        }
    } else if (key.code == sf::Keyboard::Escape) {
        const auto& cancelButton = getCancelButton();
        if (cancelButton != nullptr) {
            cancelButton->onClick.emit(cancelButton.get(), sf::Vector2f(0.0f, 0.0f));
        }
        return eventConsumed;    // Do not consume escape event.
    } else if (key.code == sf::Keyboard::Tab) {
        focusNextTextBox();
    } else {
        return eventConsumed;
    }
    return true;
}

DialogBox::DialogBox(std::shared_ptr<DialogBoxStyle> style, const sf::String& name) :
    baseClass(name),
    style_(style),
    styleCopied_(false),
    draggable_(true),
    dragPoint_({0.0f, 0.0f}, false),
    submitIndex_(std::numeric_limits<size_t>::max()),
    cancelIndex_(std::numeric_limits<size_t>::max()),
    titleAlignment_(Alignment::center),
    buttonAlignment_(Alignment::right) {
}

void DialogBox::updateTitle() {
    if (title_ == nullptr) {
        return;
    }
    if (titleAlignment_ == Alignment::left) {
        title_->setPosition(style_->titlePadding_.x, style_->titlePadding_.y);
    } else if (titleAlignment_ == Alignment::center) {
        title_->setPosition(size_.x / 2.0f - title_->getSize().x / 2.0f, style_->titlePadding_.y);
    } else if (titleAlignment_ == Alignment::right) {
        title_->setPosition(size_.x - title_->getSize().x - style_->titlePadding_.x, style_->titlePadding_.y);
    }
}

void DialogBox::updateButtons() {
    GUI_DEBUG << "DialogBox::updateButtons() called with " << optionButtons_.size() << " buttons.\n";
    if (optionButtons_.empty()) {
        return;
    }
    float totalButtonsLength = -style_->buttonPadding_.x;
    for (const auto& button : optionButtons_) {
        if (button != nullptr) {
            totalButtonsLength += button->getSize().x + style_->buttonPadding_.x;
        }
    }

    float currentPos = 0.0f;
    if (buttonAlignment_ == Alignment::left) {
        currentPos = style_->buttonPadding_.x;
    } else if (buttonAlignment_ == Alignment::center) {
        currentPos = size_.x / 2.0f - totalButtonsLength / 2.0f;
    } else if (buttonAlignment_ == Alignment::right) {
        currentPos = size_.x - totalButtonsLength - style_->buttonPadding_.x;
    }

    for (const auto& button : optionButtons_) {
        if (button != nullptr) {
            button->setPosition(currentPos, size_.y - button->getSize().y - style_->buttonPadding_.y);
            currentPos += button->getSize().x + style_->buttonPadding_.x;
        }
    }
}

void DialogBox::focusNextTextBox() {
    size_t currentFocus = getChildren().size() - 1;
    for (size_t i = 0; i < getChildren().size(); ++i) {
        if (getChild(i)->isFocused()) {
            currentFocus = i;
            break;
        }
    }
    GUI_DEBUG << "currentFocus = " << currentFocus << "\n";

    for (size_t i = 0; i < getChildren().size(); ++i) {
        size_t offsetIndex = (i + currentFocus + 1) % getChildren().size();
        const auto textBox = dynamic_cast<TextBox*>(getChild(offsetIndex).get());
        if (textBox != nullptr) {
            textBox->setFocused(true);
            return;
        }
        const auto multilineTextBox = dynamic_cast<MultilineTextBox*>(getChild(offsetIndex).get());
        if (multilineTextBox != nullptr) {
            multilineTextBox->setFocused(true);
            multilineTextBox->selectAll();
            return;
        }
    }

    // No text box found, focus the dialog itself.
    setFocused(true);
}

void DialogBox::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (!isVisible()) {
        return;
    }
    states.transform *= getTransform();

    style_->rect_.setSize(size_);
    target.draw(style_->rect_, states);
    float outlineThickness = style_->rect_.getOutlineThickness();
    style_->titleBar_.setPosition(-outlineThickness, -outlineThickness);
    style_->titleBar_.setSize({size_.x + outlineThickness * 2.0f, style_->titleBarHeight_});
    target.draw(style_->titleBar_, states);

    for (const auto& child : getChildren()) {
        target.draw(*child, states);
    }
}

}
