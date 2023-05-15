#include <gui/Gui.h>
#include <gui/Widget.h>

namespace gui {

Gui::Gui() {

}

void Gui::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    for (auto child : getChildren()) {
        target.draw(*child, states);
    }
}

}
