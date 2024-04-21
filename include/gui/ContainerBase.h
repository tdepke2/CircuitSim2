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
    virtual ~ContainerBase();

    virtual void addChild(std::shared_ptr<Widget> child) = 0;
    bool removeChild(const std::shared_ptr<Widget>& child);
    bool removeChild(size_t index);
    void removeAllChildren();
    bool hasChild(const std::shared_ptr<Widget>& child) const;
    std::shared_ptr<Widget> getChild(const sf::String& name, bool recursive = true) const;
    template<typename T>
    std::shared_ptr<T> getChild(const sf::String& name, bool recursive = true) const {
        return std::dynamic_pointer_cast<T>(getChild(name, recursive));
    }
    std::shared_ptr<Widget> getChild(size_t index) const;
    template<typename T>
    std::shared_ptr<T> getChild(size_t index) const {
        return std::dynamic_pointer_cast<T>(getChild(index));
    }
    const std::vector<std::shared_ptr<Widget>>& getChildren() const;
    /**
     * Adjusts the ordering of widgets to ensure the specified child draws on
     * top of others. Moving to the front means drawing last, so the child
     * actually goes to the back of the container.
     */
    bool sendChildToFront(const std::shared_ptr<Widget>& child);
    bool sendChildToBack(const std::shared_ptr<Widget>& child);

    /**
     * The returned Widget will be visible, but may not be enabled. Note that
     * the given mouse coordinates are in local space (if that applies).
     */
    Widget* getWidgetUnderMouse(const sf::Vector2f& mouseLocal) const;

protected:
    std::vector<std::shared_ptr<Widget>> children_;
};

} // namespace gui
