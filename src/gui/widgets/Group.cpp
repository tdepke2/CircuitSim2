#include <gui/Gui.h>
#include <gui/Theme.h>
#include <gui/widgets/Group.h>

namespace gui {

std::shared_ptr<Group> Group::create(const sf::String& name) {
    return std::shared_ptr<Group>(new Group(name));
}

sf::FloatRect Group::getLocalBounds() const {
    return {-getOrigin(), {0.0f, 0.0f}};
}
bool Group::isMouseIntersecting(const sf::Vector2f& mouseParent) const {
    auto mouseLocal = toLocalSpace(mouseParent);
    for (auto c = getChildren().crbegin(); c != getChildren().crend(); ++c) {
        if ((*c)->isVisible() && (*c)->isMouseIntersecting(mouseLocal)) {
            return true;
        }
    }
    return false;
}

bool Group::handleMouseMove(const sf::Vector2f& mouseParent) {
    auto mouseLocal = toLocalSpace(mouseParent);
    auto widget = getWidgetUnderMouse(mouseLocal);
    if (widget == nullptr || !widget->handleMouseMove(mouseLocal)) {
        return Container::handleMouseMove(mouseParent);
    } else {
        return true;
    }
}
bool Group::handleMouseWheelScroll(sf::Mouse::Wheel wheel, float delta, const sf::Vector2f& mouseParent) {
    auto mouseLocal = toLocalSpace(mouseParent);
    auto widget = getWidgetUnderMouse(mouseLocal);
    if (widget == nullptr || !widget->handleMouseWheelScroll(wheel, delta, mouseLocal)) {
        return Container::handleMouseWheelScroll(wheel, delta, mouseParent);
    } else {
        return true;
    }
}
void Group::handleMousePress(sf::Mouse::Button button, const sf::Vector2f& mouseParent) {
    auto mouseLocal = toLocalSpace(mouseParent);
    auto widget = getWidgetUnderMouse(mouseLocal);
    if (widget != nullptr) {
        widget->handleMousePress(button, mouseLocal);
    }
    if (widget == nullptr || !widget->isFocusable()) {
        Container::handleMousePress(button, mouseParent);
    }
}
void Group::handleMouseRelease(sf::Mouse::Button button, const sf::Vector2f& mouseParent) {
    auto mouseLocal = toLocalSpace(mouseParent);
    auto widget = getWidgetUnderMouse(mouseLocal);
    if (widget != nullptr) {
        widget->handleMouseRelease(button, mouseLocal);
    }
    if (widget == nullptr || !widget->isFocusable()) {
        Container::handleMouseRelease(button, mouseParent);
    }
}

Group::Group(const sf::String& name) :
    Container(name) {
}

void Group::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (!isVisible()) {
        return;
    }
    states.transform *= getTransform();

    for (const auto& child : getChildren()) {
        target.draw(*child, states);
    }
}

}
