#pragma once

#include <gui/Signal.h>

#include <memory>
#include <SFML/Graphics.hpp>

namespace gui {
    class Container;
    class Gui;
}

namespace gui {

class Widget : public std::enable_shared_from_this<Widget>, public sf::Drawable, public sf::Transformable {
public:
    //static std::shared_ptr<Widget> create();
    virtual ~Widget() noexcept = default;

    Container* getParent() const;
    Gui* getGui() const;
    void setVisible(bool visible);
    bool isVisible() const;
    void setEnabled(bool enabled);
    bool isEnabled() const;

    // Converts a point to the `Widget`s local coordinate system, offset by the origin to match up with the local bounds.
    sf::Vector2f toLocalSpace(const sf::Vector2f& point) const;

    // Get the bounding rectangle in local space, based on the origin and size of the `Widget`.
    virtual sf::FloatRect getLocalBounds() const = 0;
    virtual bool isMouseHovering(const sf::Vector2f& mouseLocal) const;
    virtual void handleMouseMove(const sf::Vector2f& mouseLocal);
    virtual void handleMousePress(sf::Mouse::Button button, const sf::Vector2f& mouseLocal);
    virtual void handleMouseRelease(sf::Mouse::Button button, const sf::Vector2f& mouseLocal);

    virtual void handleMouseEntered();
    virtual void handleMouseLeft();

    Signal<Widget*> onMouseEnter;
    Signal<Widget*> onMouseLeave;
    // FIXME signals for focus/unfocus?

protected:
    Widget();

    void setParent(Container* parent);
    virtual void setGui(Gui* gui);

private:
    Container* parent_;
    Gui* gui_;
    bool visible_, enabled_;

    friend class Container;
    friend class ContainerBase;
    friend class Gui;
};

}
