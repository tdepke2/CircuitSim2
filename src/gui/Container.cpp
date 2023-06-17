#include <gui/Container.h>

namespace gui {

ContainerBase::~ContainerBase() noexcept {
    removeAllChildren();


    // FIXME does this make sense? also what happens if we add a widget to two different containers?
    // what about adding it to the same one twice?


}

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
    children_[index]->setParentAndGui(nullptr, nullptr);
    children_.erase(children_.cbegin() + index);
    return true;
}
void ContainerBase::removeAllChildren() {
    for (const auto& c : children_) {
        c->setParentAndGui(nullptr, nullptr);
    }
    children_.clear();
}
bool ContainerBase::sendChildToFront(std::shared_ptr<Widget> child) {
    for (size_t i = 0; i < children_.size(); ++i) {
        if (children_[i] == child) {
            for (size_t j = i + 1; j < children_.size(); ++j) {
                std::swap(children_[j - 1], children_[j]);
            }
            child->requestRedraw();
            return true;
        }
    }
    return false;
}
bool ContainerBase::sendChildToBack(std::shared_ptr<Widget> child) {
    for (size_t i = 0; i < children_.size(); ++i) {
        if (children_[i] == child) {
            for (size_t j = i; j > 0; --j) {
                std::swap(children_[j - 1], children_[j]);
            }
            child->requestRedraw();
            return true;
        }
    }
    return false;
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
    child->setParentAndGui(this, getGui());
}

void Container::setParentAndGui(Container* parent, Gui* gui) {
    Widget::setParentAndGui(parent, gui);
    for (const auto& c : children_) {
        c->setParentAndGui(c->getParent(), gui);
    }
}

}
