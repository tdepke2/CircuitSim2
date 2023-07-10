#pragma once

#include <gui/Widget.h>

#include <memory>
#include <SFML/Graphics.hpp>
#include <vector>

namespace gui {

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


// FIXME: is this the right idea? container should be a widget so that the parent of a widget is a container (and therefore a widget).
// we also want Gui to be a container, but not a Widget...
class Container : public ContainerBase, public Widget {
public:
    virtual ~Container() noexcept = default;

    virtual void addChild(std::shared_ptr<Widget> child) override;

    virtual void setVisible(bool visible) override;
    virtual void setEnabled(bool enabled) override;

protected:
    virtual void setParentAndGui(Container* parent, Gui* gui) override;
    virtual void addWidgetUnderMouse(const sf::Vector2f& mouseParent) override;
};

}
