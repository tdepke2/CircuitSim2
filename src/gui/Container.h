#pragma once

#include <gui/ContainerBase.h>
#include <gui/Widget.h>

#include <memory>
#include <SFML/Graphics.hpp>

namespace gui {

/**
 * Specialization of a `Widget` that is also a container for other widgets.
 * 
 * This class is kept separate from `ContainerBase` so that the `Gui` can be a
 * container and not a widget.
 */
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
