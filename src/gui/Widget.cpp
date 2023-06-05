#include <gui/Gui.h>
#include <gui/Widget.h>

namespace gui {

/*std::shared_ptr<Widget> Widget::create() {
    return std::make_shared<Widget>();
}*/

Container* Widget::getParent() const {
    return parent_;
}

Gui* Widget::getGui() const {
    return gui_;
}

void Widget::setVisible(bool visible) {
    visible_ = visible;
}

bool Widget::isVisible() const {
    return visible_;
}

void Widget::setEnabled(bool enabled) {
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

sf::Vector2f Widget::toLocalSpace(const sf::Vector2f& point) const {
    return getInverseTransform().transformPoint(point) - getOrigin();
}

bool Widget::isMouseHovering(const sf::Vector2f& mouseLocal) const {
    return getLocalBounds().contains(toLocalSpace(mouseLocal));
}
void Widget::handleMouseMove(const sf::Vector2f& mouseLocal) {
    if (gui_ == nullptr) {
        return;
    }
    gui_->addWidgetUnderMouse(shared_from_this());
}
void Widget::handleMousePress(sf::Mouse::Button button, const sf::Vector2f& mouseLocal) {
    if (button <= sf::Mouse::Button::Middle) {
        setFocused(true);
    }
}
void Widget::handleMouseRelease(sf::Mouse::Button button, const sf::Vector2f& mouseLocal) {

}
void Widget::handleTextEntered(uint32_t unicode) {

}

void Widget::handleMouseEntered() {
    onMouseEnter.emit(this);
}
void Widget::handleMouseLeft() {
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

Widget::Widget() :
    parent_(nullptr),
    gui_(nullptr),
    visible_(true),
    enabled_(true),
    focused_(false) {
}

void Widget::setParent(Container* parent) {
    parent_ = parent;
}

void Widget::setGui(Gui* gui) {
    gui_ = gui;
}

}
