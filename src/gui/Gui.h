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
    virtual ~Gui() = default;

    void handleEvent(const sf::Event& event);

private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};

}
