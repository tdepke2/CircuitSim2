#pragma once

//class Widget;

#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

namespace gui {

class Widget;

class Gui : public sf::Drawable {
public:
    Gui();
    void addWidget(std::shared_ptr<Widget> w);

private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

    std::vector<std::shared_ptr<Widget>> _widgets;
};

}
