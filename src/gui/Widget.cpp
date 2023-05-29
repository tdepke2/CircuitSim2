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

}
void Widget::handleMouseRelease(sf::Mouse::Button button, const sf::Vector2f& mouseLocal) {

}

void Widget::handleMouseEntered() {
    onMouseEnter.emit(this);
}
void Widget::handleMouseLeft() {
    onMouseLeave.emit(this);
}

Widget::Widget() :
    parent_(nullptr),
    gui_(nullptr),
    visible_(true),
    enabled_(true) {
}

void Widget::setParent(Container* parent) {
    parent_ = parent;
}

void Widget::setGui(Gui* gui) {
    gui_ = gui;
}

}
