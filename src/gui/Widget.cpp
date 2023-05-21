#include <gui/Widget.h>

namespace gui {

/*std::shared_ptr<Widget> Widget::create() {
    return std::make_shared<Widget>();
}*/

bool Widget::isMouseHovering(int x, int y) const {
    return getBounds().contains(x, y);
}
void Widget::handleMousePress(sf::Mouse::Button button, int x, int y) {

}
void Widget::handleMouseRelease(sf::Mouse::Button button, int x, int y) {}

}
