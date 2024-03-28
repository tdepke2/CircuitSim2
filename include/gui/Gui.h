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
 * top-most visible widget the mouse touches (which will include any `Container`
 * types the widget is in). Text events are only sent to the focused widget.
 * Also, for mouse events to propagate through a `Container`, the mouse must be
 * touching the `Container` (a widget outside the bounds of its parent
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
    // FIXME make non-copyable?

    void setSmooth(bool smooth);
    bool isSmooth() const;

    virtual void addChild(const std::shared_ptr<Widget>& child) override;
    void processEvent(const sf::Event& event);

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

} // namespace gui
