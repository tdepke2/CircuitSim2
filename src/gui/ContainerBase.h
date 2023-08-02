#pragma once

#include <gui/Widget.h>

#include <memory>
#include <SFML/Graphics.hpp>
#include <vector>

namespace gui {

/**
 * Abstract base class for objects that store `Widget` types. The `Widget`
 * children are stored in a simple array structure (and drawn in this order).
 */
class ContainerBase {
public:
    virtual ~ContainerBase() noexcept;

    virtual void addChild(std::shared_ptr<Widget> child) = 0;
    bool removeChild(std::shared_ptr<Widget> child);
    bool removeChild(size_t index);
    void removeAllChildren();
    // Adjusts the ordering of widgets to ensure the specified child draws on top of others.
    // Moving to the front means drawing last, so the child actually goes to the back of the container.
    bool sendChildToFront(std::shared_ptr<Widget> child);
    bool sendChildToBack(std::shared_ptr<Widget> child);
    const std::vector<std::shared_ptr<Widget>>& getChildren() const;

    // The returned Widget will be visible, but may not be enabled. Note that
    // the given mouse coordinates are in local space (if that applies).
    Widget* getWidgetUnderMouse(const sf::Vector2f& mouseLocal) const;

protected:
    std::vector<std::shared_ptr<Widget>> children_;
};

}
