#pragma once

#include <gui/Signal.h>

#include <memory>
#include <SFML/Graphics.hpp>

namespace gui {
    class Container;
}

namespace gui {

class Widget : public sf::Drawable {
public:
    //static std::shared_ptr<Widget> create();
    virtual ~Widget() = default;

    void setParent(Container* parent);
    Container* getParent() const;

    virtual sf::FloatRect getBounds() const = 0;
    virtual bool isMouseHovering(int x, int y) const;
    virtual void handleMousePress(sf::Mouse::Button button, int x, int y);
    virtual void handleMouseRelease(sf::Mouse::Button button, int x, int y);

    Signal<Widget*> onMouseEnter;
    Signal<Widget*> onMouseLeave;
    // FIXME signals for focus/unfocus?

protected:
    Widget();

    Container* parent_;
};

}
