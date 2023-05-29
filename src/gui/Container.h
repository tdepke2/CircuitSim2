#pragma once

#include <gui/Widget.h>

#include <memory>
#include <SFML/Graphics.hpp>
#include <vector>

namespace gui {

class ContainerBase {
public:
    virtual ~ContainerBase() noexcept = default;

    virtual void addChild(std::shared_ptr<Widget> child) = 0;
    bool removeChild(std::shared_ptr<Widget> child);
    bool removeChild(size_t index);
    const std::vector<std::shared_ptr<Widget>>& getChildren() const;

    // The returned Widget will be visible, but may not be enabled.
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

protected:
    virtual void setGui(Gui* gui) override;
};

}
