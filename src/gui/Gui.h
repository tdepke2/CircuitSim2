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

// FIXME need some notes about how events are sent to target widgets, and how the container passes events to children only within its local bounds (that might also need to be fixed btw?)

/**
 * Top-level context for the graphical user interface. The `Gui` stores `Widget`
 * children for updating and drawing them.
 * 
 * Note that drawing is done in a lazy fashion. A `Widget` calls
 * `requestRedraw()` to trigger drawing during the next call to `Gui::draw()`.
 * The redraw operation draws all of the widgets, so it's ideal to only request
 * drawing during event handling instead of every frame.
 * 
 * Much of the GUI framework has taken inspiration from other SFML libraries
 * like TGUI (https://github.com/texus/TGUI/) and SFGUI
 * (https://github.com/TankOs/SFGUI). The TGUI docs are quite good and have been
 * used as a reference for some design aspects.
 */
class Gui : public ContainerBase, public sf::Drawable {
public:
    Gui(sf::RenderWindow& window);
    virtual ~Gui() noexcept = default;

    void setSmooth(bool smooth);
    bool isSmooth() const;

    virtual void addChild(std::shared_ptr<Widget> child) override;
    void handleEvent(const sf::Event& event);

    // Internal
    void addWidgetUnderMouse(std::shared_ptr<Widget> widget);

    // Internal
    void requestWidgetFocus(std::shared_ptr<Widget> widget);

    // Internal
    void requestRedraw() const;

private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    sf::RenderWindow& window_;
    mutable sf::RenderTexture renderTexture_;
    mutable bool redrawPending_;
    sf::Sprite renderSprite_;

    std::set<std::shared_ptr<Widget>> widgetsUnderMouse_, lastWidgetsUnderMouse_;
    std::shared_ptr<Widget> focusedWidget_;
};

}
