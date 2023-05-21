#include <gui/Gui.h>
#include <gui/Widget.h>

namespace gui {

void Gui::handleEvent(const sf::Event& event) {
    if (event.type == sf::Event::MouseButtonPressed) {
        for (auto child : getChildren()) {
            if (child->isMouseHovering(event.mouseButton.x, event.mouseButton.y)) {
                child->handleMousePress(event.mouseButton.button, event.mouseButton.x, event.mouseButton.y);
            }
        }
    }
}

void Gui::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    for (auto child : getChildren()) {
        target.draw(*child, states);
    }
}

}
