#include <gui/ContainerBase.h>

namespace gui {

ContainerBase::~ContainerBase() noexcept {
    removeAllChildren();


    // FIXME does this make sense? also what happens if we add a widget to two different containers?
    // what about adding it to the same one twice?


}

bool ContainerBase::removeChild(const std::shared_ptr<Widget>& child) {
    for (size_t i = 0; i < children_.size(); ++i) {
        if (children_[i] == child) {
            return removeChild(i);
        }
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

std::shared_ptr<Widget> ContainerBase::getChild(const sf::String& name, bool recursive) const {
    for (const auto& c : children_) {
        if (c->getName() == name) {
            return c;
        }
    }

    // FIXME how to do recursive?

    return nullptr;
}

std::shared_ptr<Widget> ContainerBase::getChild(size_t index) const {
    if (index >= children_.size()) {
        return nullptr;
    }
    return children_[index];
}

const std::vector<std::shared_ptr<Widget>>& ContainerBase::getChildren() const {
    return children_;
}

bool ContainerBase::sendChildToFront(const std::shared_ptr<Widget>& child) {
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

bool ContainerBase::sendChildToBack(const std::shared_ptr<Widget>& child) {
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

Widget* ContainerBase::getWidgetUnderMouse(const sf::Vector2f& mouseLocal) const {
    // Find the top-most visible child.
    for (auto c = children_.crbegin(); c != children_.crend(); ++c) {
        if ((*c)->isVisible() && (*c)->isMouseHovering(mouseLocal)) {
            return (*c).get();
        }
    }
    return nullptr;
}

}
