#include <gui/Button.h>

namespace gui {

std::shared_ptr<Button> Button::create() {
    return std::shared_ptr<Button>(new Button);
}

Button::Button() : _rect(sf::Vector2f(40.0, 20.0)) {
    _rect.setFillColor(sf::Color::Red);
}

void Button::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    target.draw(_rect, states);
}

}
