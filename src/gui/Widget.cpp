#include <gui/Gui.h>
#include <gui/Widget.h>


#include <iostream>



namespace gui {

void Widget::setPosition(float x, float y) {
    transformable_.setPosition(x, y);
    requestRedraw();
}
void Widget::setPosition(const sf::Vector2f& position) {
    setPosition(position.x, position.y);
}
void Widget::setRotation(float angle) {
    transformable_.setRotation(angle);
    requestRedraw();
}
void Widget::setScale(float factorX, float factorY) {
    transformable_.setScale(factorX, factorY);
    requestRedraw();
}
void Widget::setScale(const sf::Vector2f& factors) {
    setScale(factors.x, factors.y);
}
void Widget::setOrigin(float x, float y) {
    transformable_.setOrigin(x, y);
    requestRedraw();
}
void Widget::setOrigin(const sf::Vector2f& origin) {
    setOrigin(origin.x, origin.y);
}
const sf::Vector2f& Widget::getPosition() const {
    return transformable_.getPosition();
}
float Widget::getRotation() const {
    return transformable_.getRotation();
}
const sf::Vector2f& Widget::getScale() const {
    return transformable_.getScale();
}
const sf::Vector2f& Widget::getOrigin() const {
    return transformable_.getOrigin();
}
void Widget::move(float offsetX, float offsetY) {
    transformable_.move(offsetX, offsetY);
    requestRedraw();
}
void Widget::move(const sf::Vector2f& offset) {
    move(offset.x, offset.y);
}
void Widget::rotate(float angle) {
    transformable_.rotate(angle);
    requestRedraw();
}
void Widget::scale(float factorX, float factorY) {
    transformable_.scale(factorX, factorY);
    requestRedraw();
}
void Widget::scale(const sf::Vector2f& factor) {
    scale(factor.x, factor.y);
}
const sf::Transform& Widget::getTransform() const {
    return transformable_.getTransform();
}
const sf::Transform& Widget::getInverseTransform() const {
    return transformable_.getInverseTransform();
}

void Widget::setName(const sf::String& name) {
    name_ = name;
}

const sf::String& Widget::getName() const {
    return name_;
}

Container* Widget::getParent() const {
    return parent_;
}

Gui* Widget::getGui() const {
    return gui_;
}

void Widget::setVisible(bool visible) {
    if (!visible) {
        setFocused(false);
        if (mouseHover_) {
            handleMouseLeft();
        }
    }
    if (visible_ != visible) {
        requestRedraw();
    }
    visible_ = visible;
}

bool Widget::isVisible() const {
    return visible_;
}

void Widget::setEnabled(bool enabled) {
    if (!enabled) {
        mouseHover_ = false;
        setFocused(false);
    }
    if (enabled_ != enabled) {
        requestRedraw();
    }
    enabled_ = enabled;
}

bool Widget::isEnabled() const {
    return enabled_;
}

void Widget::setFocused(bool focused) {
    if (focused_ == focused || gui_ == nullptr) {
        return;
    } else if (focused) {
        gui_->requestWidgetFocus(shared_from_this());
    } else {
        gui_->requestWidgetFocus(nullptr);
    }
}

bool Widget::isFocused() const {
    return focused_;
}

void Widget::setFocusable(bool focusable) {
    if (!focusable) {
        setFocused(false);
    }
    focusable_ = focusable;
}

bool Widget::isFocusable() const {
    return focusable_;
}

bool Widget::isMouseHovering() const {
    return mouseHover_;
}

void Widget::sendToFront() {
    if (parent_ != nullptr) {
        parent_->sendChildToFront(shared_from_this());
    } else if (gui_ != nullptr) {
        gui_->sendChildToFront(shared_from_this());
    }
}

void Widget::sendToBack() {
    if (parent_ != nullptr) {
        parent_->sendChildToBack(shared_from_this());
    } else if (gui_ != nullptr) {
        gui_->sendChildToBack(shared_from_this());
    }
}

void Widget::requestRedraw() const {
    std::cout << "Widget::requestRedraw() from " << this << (gui_ != nullptr ? "" : " (gui_ null)") << "\n";
    if (gui_ != nullptr) {
        gui_->requestRedraw();
    }
}

sf::Vector2f Widget::toGuiSpace(sf::Vector2f point) const {
    const Container* container = parent_;
    while (container != nullptr) {
        point = container->getTransform().transformPoint(point);
        container = container->getParent();
    }
    return point;
}

sf::Vector2f Widget::fromGuiSpace(const sf::Vector2f& point) const {
    if (parent_ == nullptr) {
        return point;
    }
    return parent_->getInverseTransform().transformPoint(parent_->fromGuiSpace(point));
}

bool Widget::isMouseIntersecting(const sf::Vector2f& mouseParent) const {
    return getLocalBounds().contains(toLocalOriginSpace(mouseParent));
}
bool Widget::handleMouseMove(const sf::Vector2f& /*mouseParent*/) {
    return false;
}
bool Widget::handleMouseWheelScroll(sf::Mouse::Wheel /*wheel*/, float /*delta*/, const sf::Vector2f& /*mouseParent*/) {
    return false;
}
void Widget::handleMousePress(sf::Mouse::Button button, const sf::Vector2f& /*mouseParent*/) {
    if (button <= sf::Mouse::Middle && focusable_) {
        setFocused(true);
    }
}
void Widget::handleMouseRelease(sf::Mouse::Button /*button*/, const sf::Vector2f& /*mouseParent*/) {

}
bool Widget::handleTextEntered(uint32_t /*unicode*/) {
    return false;
}
bool Widget::handleKeyPressed(const sf::Event::KeyEvent& /*key*/) {
    return false;
}

void Widget::handleMouseEntered() {
    mouseHover_ = true;
    onMouseEnter.emit(this);
}
void Widget::handleMouseLeft() {
    mouseHover_ = false;
    onMouseLeave.emit(this);
}
void Widget::handleFocusChange(bool focused) {
    focused_ = focused;
    if (focused) {
        onFocusGained.emit(this);
    } else {
        onFocusLost.emit(this);
    }
}

Widget::Widget(const sf::String& name) :
    name_(name),
    parent_(nullptr),
    gui_(nullptr),
    visible_(true),
    enabled_(true),
    focused_(false),
    focusable_(true),
    mouseHover_(false) {
}

void Widget::setParentAndGui(Container* parent, Gui* gui) {
    parent_ = parent;
    if (gui_ != nullptr) {
        gui_->requestRedraw();
    }
    gui_ = gui;
    if (gui != nullptr) {
        gui->requestRedraw();
    }
}

void Widget::addWidgetUnderMouse(const sf::Vector2f& /*mouseParent*/) {
    if (gui_ != nullptr) {
        gui_->addWidgetUnderMouse(shared_from_this());
    }
}

}
