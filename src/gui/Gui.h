#pragma once

#include <gui/Container.h>

#include <memory>
#include <SFML/Graphics.hpp>
#include <vector>

namespace gui {
    class Widget;
}

namespace gui {

class Gui : public sf::Drawable, public Container {
public:
    Gui();

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};

}
