#pragma once

#include <gui/Container.h>

#include <memory>
#include <set>
#include <SFML/Graphics.hpp>
#include <vector>

namespace gui {
    class Widget;
}

namespace gui {

class Gui : public ContainerBase, public sf::Drawable {
public:
    Gui(sf::RenderWindow& window);
    virtual ~Gui() noexcept = default;

    virtual void addChild(std::shared_ptr<Widget> child) override;
    void handleEvent(const sf::Event& event);

    // Internal
    void addWidgetUnderMouse(std::shared_ptr<Widget> widget);

    // Internal
    void requestWidgetFocus(std::shared_ptr<Widget> widget);

private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    sf::RenderWindow& window_;
    std::set<std::shared_ptr<Widget>> widgetsUnderMouse_, lastWidgetsUnderMouse_;
    std::shared_ptr<Widget> focusedWidget_;
};

}
