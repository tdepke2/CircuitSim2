#include <gui/Widget.h>

namespace gui {

/*std::shared_ptr<Widget> Widget::create() {
    return std::make_shared<Widget>();
}*/

void Widget::setParent(Container* parent) {
    parent_ = parent;
}

Container* Widget::getParent() const {
    return parent_;
}

bool Widget::isMouseHovering(int x, int y) const {
    return getBounds().contains(x, y);
}
void Widget::handleMousePress(sf::Mouse::Button button, int x, int y) {

}
void Widget::handleMouseRelease(sf::Mouse::Button button, int x, int y) {

}

Widget::Widget() :
    parent_(nullptr) {
}

}
