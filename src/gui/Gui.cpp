#include <gui/Gui.h>
#include <gui/Widget.h>

namespace gui {

Gui::Gui() {

}

void Gui::addWidget(std::shared_ptr<Widget> w) {
    _widgets.push_back(w);
}

void Gui::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    for (auto w : _widgets) {
        target.draw(*w, states);
    }
}

}
