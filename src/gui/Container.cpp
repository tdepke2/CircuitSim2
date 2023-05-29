#include <gui/Container.h>

namespace gui {

bool ContainerBase::removeChild(std::shared_ptr<Widget> child) {
    size_t i = 0;
    for (const auto& c : children_) {
        if (c == child) {
            return removeChild(i);
        }
        ++i;
    }
    return false;
}
bool ContainerBase::removeChild(size_t index) {
    if (index >= children_.size()) {
        return false;
    }
    children_[index]->setParent(nullptr);
    children_[index]->setGui(nullptr);
    children_.erase(children_.cbegin() + index);
    return true;
}
const std::vector<std::shared_ptr<Widget>>& ContainerBase::getChildren() const {
    return children_;
}

Widget* ContainerBase::getWidgetUnderMouse(const sf::Vector2f& mouseLocal) const {
    // Find the top-most visible child.
    for (auto c = children_.crbegin(); c != children_.crend(); ++c) {
        if ((*c)->isVisible() && (*c)->isMouseHovering(mouseLocal)) {
            return (*c).get();
        }
    }
    return nullptr;
}

void Container::addChild(std::shared_ptr<Widget> child) {
    children_.push_back(child);
    child->setParent(this);
    child->setGui(getGui());
}

void Container::setGui(Gui* gui) {
    Widget::setGui(gui);
    for (const auto& c : children_) {
        c->setGui(gui);
    }
}

}
