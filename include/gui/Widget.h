#pragma once

#include <gui/Signal.h>

#include <memory>
#include <SFML/Graphics.hpp>

namespace gui {
    class Container;
    class Gui;
}

namespace gui {

/**
 * Abstract base class for a graphical widget that can be drawn to a `Gui`.
 * 
 * A number of `Signal` members are provided to bind function callbacks in
 * response to events, use `Signal::connect()` for this purpose. It is also safe
 * to call `Signal::disconnectAll()` without affecting the internal behavior of
 * a widget.
 * 
 * The functions beginning with "handle" are used internally to respond to
 * events and such. When adding overrides to these in derived classes, the base
 * class version should be called at the beginning or end depending on the
 * context (e.g. `handleMousePress()` at the beginning, `handleMouseRelease()`
 * at the end).
 */
class Widget : public std::enable_shared_from_this<Widget>, public sf::Drawable {
public:
    virtual ~Widget() noexcept = default;
    Widget(const Widget& rhs) = delete;
    Widget& operator=(const Widget& rhs) = delete;

    // sf::Transformable interface.
    // The functions from sf::Transformable are reimplemented here instead of
    // having Widget inherit from the class so that we can track changes to
    // state that require a redraw of the object.
    void setPosition(float x, float y);
    void setPosition(const sf::Vector2f& position);
    void setRotation(float angle);
    void setScale(float factorX, float factorY);
    void setScale(const sf::Vector2f& factors);
    void setOrigin(float x, float y);
    void setOrigin(const sf::Vector2f& origin);
    const sf::Vector2f& getPosition() const;
    float getRotation() const;
    const sf::Vector2f& getScale() const;
    const sf::Vector2f& getOrigin() const;
    void move(float offsetX, float offsetY);
    void move(const sf::Vector2f& offset);
    void rotate(float angle);
    void scale(float factorX, float factorY);
    void scale(const sf::Vector2f& factor);
    const sf::Transform& getTransform() const;
    const sf::Transform& getInverseTransform() const;

    void setName(const sf::String& name);
    const sf::String& getName() const;
    Container* getParent() const;
    Gui* getGui() const;
    virtual void setVisible(bool visible);
    bool isVisible() const;
    // If a widget is not enabled, it no longer receives events.
    virtual void setEnabled(bool enabled);
    bool isEnabled() const;
    void setFocused(bool focused);
    bool isFocused() const;
    bool isMouseHovering() const;

    void sendToFront();
    void sendToBack();

    void requestRedraw() const;

    // Converts a point into the `Widget`s local coordinate system.
    sf::Vector2f toLocalSpace(const sf::Vector2f& point) const {
        return getInverseTransform().transformPoint(point);
    }
    // Also converts to local coordinates like `toLocalSpace()`, but offsets the
    // point by the origin to match up with the local bounds.
    sf::Vector2f toLocalOriginSpace(const sf::Vector2f& point) const {
        return getInverseTransform().transformPoint(point) - getOrigin();
    }

    // Get the bounding rectangle in local space, based on the origin and size of the `Widget`.
    virtual sf::FloatRect getLocalBounds() const = 0;
    virtual bool isMouseIntersecting(const sf::Vector2f& mouseParent) const;

    virtual void handleMouseMove(const sf::Vector2f& mouseParent);
    virtual void handleMouseWheelScroll(sf::Mouse::Wheel wheel, float delta, const sf::Vector2f& mouseParent);
    virtual void handleMousePress(sf::Mouse::Button button, const sf::Vector2f& mouseParent);
    virtual void handleMouseRelease(sf::Mouse::Button button, const sf::Vector2f& mouseParent);
    virtual void handleTextEntered(uint32_t unicode);
    virtual void handleKeyPressed(const sf::Event::KeyEvent& key);

    virtual void handleMouseEntered();
    virtual void handleMouseLeft();
    virtual void handleFocusChange(bool focused);

    Signal<Widget*> onMouseEnter;
    Signal<Widget*> onMouseLeave;
    Signal<Widget*> onFocusGained;
    Signal<Widget*> onFocusLost;

protected:
    Widget(const sf::String& name);

    virtual void setParentAndGui(Container* parent, Gui* gui);
    virtual void addWidgetUnderMouse(const sf::Vector2f& mouseParent);

private:
    sf::Transformable transformable_;
    sf::String name_;
    Container* parent_;
    Gui* gui_;
    bool visible_, enabled_, focused_;
    bool mouseHover_;

    friend class Container;
    friend class ContainerBase;
    friend class Gui;
};

} // namespace gui
