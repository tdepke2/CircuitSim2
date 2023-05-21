#pragma once

#include <memory>
#include <SFML/Graphics.hpp>

namespace gui {

class Widget : public sf::Drawable {
public:
    //static std::shared_ptr<Widget> create();
    virtual ~Widget() = default;

    virtual sf::FloatRect getBounds() const = 0;
    virtual bool isMouseHovering(int x, int y) const;
    virtual void handleMousePress(sf::Mouse::Button button, int x, int y);
    virtual void handleMouseRelease(sf::Mouse::Button button, int x, int y);

protected:
    Widget() = default;
    //virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const = 0;
};

}
