#include <gui/Container.h>
#include <gui/Widget.h>

namespace gui {

void Container::addChild(std::shared_ptr<Widget> c) {
    children_.push_back(c);
    c->setParent(this);
}

const std::vector<std::shared_ptr<Widget>>& Container::getChildren() const {
    return children_;
}

void Container::forwardMousePress(sf::Mouse::Button button, int x, int y) {
    for (const auto& child : children_) {
        if (child->isMouseHovering(x, y)) {
            child->handleMousePress(button, x, y);
        }
    }
}

}
