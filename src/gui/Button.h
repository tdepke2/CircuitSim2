#pragma once

#include "SFML/Graphics/RectangleShape.hpp"
#include <gui/Widget.h>

#include <SFML/Graphics.hpp>
#include <memory>

namespace gui {

class Button : public Widget {
public:
    static std::shared_ptr<Button> create();

protected:
    Button();

private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

    sf::RectangleShape _rect;
};

}
