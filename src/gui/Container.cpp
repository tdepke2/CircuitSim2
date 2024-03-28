#include <gui/Container.h>

namespace gui {

void Container::addChild(const std::shared_ptr<Widget>& child) {
    children_.push_back(child);
    child->setParentAndGui(this, getGui());
}

void Container::setVisible(bool visible) {
    if (!visible) {
        for (const auto& c : children_) {
            c->setFocused(false);
        }
    }
    Widget::setVisible(visible);
}

void Container::setEnabled(bool enabled) {
    if (!enabled) {
        for (const auto& c : children_) {
            c->setFocused(false);
        }
    }
    Widget::setEnabled(enabled);
}

Container::Container(const sf::String& name) :
    Widget(name) {
}

void Container::setParentAndGui(Container* parent, Gui* gui) {
    Widget::setParentAndGui(parent, gui);
    for (const auto& c : children_) {
        c->setParentAndGui(c->getParent(), gui);
    }
}

void Container::addWidgetUnderMouse(const sf::Vector2f& mouseParent) {
    Widget::addWidgetUnderMouse(mouseParent);
    auto mouseLocal = toLocalSpace(mouseParent);
    auto widget = getWidgetUnderMouse(mouseLocal);
    if (widget != nullptr) {
        widget->addWidgetUnderMouse(mouseLocal);
    }
}

}
