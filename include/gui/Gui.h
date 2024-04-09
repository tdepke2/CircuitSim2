#pragma once

#include <gui/Container.h>
#include <gui/Signal.h>

#include <memory>
#include <set>
#include <SFML/Graphics.hpp>
#include <vector>

namespace gui {
    class Widget;
}

namespace gui {

/**
 * Top-level context for the graphical user interface. The `Gui` stores `Widget`
 * children for updating and drawing them.
 * 
 * Note that drawing is done in a lazy fashion. A `Widget` calls
 * `requestRedraw()` to trigger drawing during the next call to `Gui::draw()`.
 * The redraw operation draws all of the widgets, so it's ideal to only request
 * drawing during event handling instead of every frame.
 * 
 * Events are processed in a simple manner. Mouse events are sent only to the
 * top-most visible widget the mouse touches and each of its `Container` derived
 * parents. Keyboard events are sent only to the focused widget and the parents.
 * Events can be consumed by a child which blocks further propagation to
 * parents. Also, for mouse events to propagate through a `Container`, the mouse
 * must be touching the `Container` (a widget outside the bounds of its parent
 * `Container` will not work).
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
    Gui(const Gui& rhs) = delete;
    Gui& operator=(const Gui& rhs) = delete;

    void setSize(const sf::Vector2u& size);
    sf::Vector2u getSize() const;
    void setSmooth(bool smooth);
    bool isSmooth() const;

    virtual void addChild(std::shared_ptr<Widget> child) override;
    // Returns true if event was consumed (and should not be processed further).
    bool processEvent(const sf::Event& event);

    // Internal
    void addWidgetUnderMouse(std::shared_ptr<Widget> widget);

    // Internal
    void requestWidgetFocus(std::shared_ptr<Widget> widget);

    // Internal
    void requestRedraw() const;

    Signal<Gui*, sf::RenderWindow&, const sf::Vector2u&> onWindowResized;

private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    sf::RenderWindow& window_;
    mutable sf::RenderTexture renderTexture_;
    mutable bool redrawPending_;
    sf::Sprite renderSprite_;

    std::set<std::shared_ptr<Widget>> widgetsUnderMouse_, lastWidgetsUnderMouse_;
    std::shared_ptr<Widget> focusedWidget_;
};

} // namespace gui
