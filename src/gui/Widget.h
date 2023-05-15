#pragma once

#include <SFML/Graphics.hpp>
#include <memory>

namespace gui {

class Widget : public sf::Drawable {
public:
    //static std::shared_ptr<Widget> create();

protected:
    Widget();
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const = 0;
};

}
