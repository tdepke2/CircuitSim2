#include <gui/Gui.h>
#include <gui/Theme.h>
#include <gui/widgets/Button.h>
#include <gui/widgets/DialogBox.h>
#include <gui/widgets/Label.h>
#include <gui/widgets/TextBox.h>





#include <iostream>




namespace gui {

DialogBoxStyle::DialogBoxStyle(const Gui& gui) :
    gui_(gui) {
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

std::shared_ptr<DialogBoxStyle> DialogBoxStyle::clone() const {
    return std::make_shared<DialogBoxStyle>(*this);
}



std::shared_ptr<DialogBox> DialogBox::create(const Theme& theme, const sf::String& name) {
    return std::shared_ptr<DialogBox>(new DialogBox(theme.getDialogBoxStyle(), name));
}
std::shared_ptr<DialogBox> DialogBox::create(std::shared_ptr<DialogBoxStyle> style, const sf::String& name) {
    return std::shared_ptr<DialogBox>(new DialogBox(style, name));
}

void DialogBox::setSize(const sf::Vector2f& size) {
    size_ = size;
    requestRedraw();
}
void DialogBox::setTitle(std::shared_ptr<Label> title) {
    if (!hasChild(title)) {
        addChild(title);
    }
    title_ = title;
}
void DialogBox::setSubmitButton(std::shared_ptr<Button> button) {
    if (!hasChild(button)) {
        addChild(button);
    }
    submitButton_ = button;
}
void DialogBox::setCancelButton(std::shared_ptr<Button> button) {
    if (!hasChild(button)) {
        addChild(button);
    }
    cancelButton_ = button;
}
const sf::Vector2f& DialogBox::getSize() const {
    return size_;
}
std::shared_ptr<Label> DialogBox::getTitle() const {
    return title_;
}
std::shared_ptr<Button> DialogBox::getSubmitButton() const {
    return submitButton_;
}
std::shared_ptr<Button> DialogBox::getCancelButton() const {
    return cancelButton_;
}

void DialogBox::setVisible(bool visible) {
    if (visible) {
        focusNextTextBox();
    }
    Group::setVisible(visible);
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
bool DialogBox::handleKeyPressed(const sf::Event::KeyEvent& key) {
    bool eventConsumed = Widget::handleKeyPressed(key);
    if (key.code == sf::Keyboard::Enter) {
        if (submitButton_ != nullptr) {
            submitButton_->onClick.emit(submitButton_.get(), sf::Vector2f(0.0f, 0.0f));
        }
    } else if (key.code == sf::Keyboard::Escape) {
        if (cancelButton_ != nullptr) {
            cancelButton_->onClick.emit(cancelButton_.get(), sf::Vector2f(0.0f, 0.0f));
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
    Group(name),
    style_(style),
    styleCopied_(false) {
}

void DialogBox::focusNextTextBox() {
    size_t currentFocus = getChildren().size() - 1;
    for (size_t i = 0; i < getChildren().size(); ++i) {
        if (getChild(i)->isFocused()) {
            currentFocus = i;
            break;
        }
    }
    std::cout << "currentFocus = " << currentFocus << "\n";

    for (size_t i = 0; i < getChildren().size(); ++i) {
        size_t offsetIndex = (i + currentFocus + 1) % getChildren().size();
        const auto textBox = dynamic_cast<TextBox*>(getChild(offsetIndex).get());
        if (textBox != nullptr) {
            textBox->setFocused(true);
            break;
        }
    }
}

void DialogBox::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (!isVisible()) {
        return;
    }
    states.transform *= getTransform();

    style_->rect_.setSize(size_);
    target.draw(style_->rect_, states);

    for (const auto& child : getChildren()) {
        target.draw(*child, states);
    }
}

}
