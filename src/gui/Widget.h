#pragma once

#include <memory>
#include <SFML/Graphics.hpp>

namespace gui {

class Widget : public sf::Drawable {
public:
    //static std::shared_ptr<Widget> create();
    virtual ~Widget() = default;

protected:
    Widget();
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const = 0;
};

}
