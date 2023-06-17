#pragma once

#include <gui/Signal.h>

#include <memory>
#include <SFML/Graphics.hpp>

namespace gui {
    class Container;
    class Gui;
}

namespace gui {

class Widget : public std::enable_shared_from_this<Widget>, public sf::Drawable {
public:
    //static std::shared_ptr<Widget> create();
    virtual ~Widget() noexcept = default;

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

    Container* getParent() const;
    Gui* getGui() const;
    void setVisible(bool visible);
    bool isVisible() const;
    void setEnabled(bool enabled);
    bool isEnabled() const;
    void setFocused(bool focused);
    bool isFocused() const;

    void sendToFront();
    void sendToBack();

    void requestRedraw() const;

    // Converts a point to the `Widget`s local coordinate system, offset by the origin to match up with the local bounds.
    sf::Vector2f toLocalSpace(const sf::Vector2f& point) const;

    // Get the bounding rectangle in local space, based on the origin and size of the `Widget`.
    virtual sf::FloatRect getLocalBounds() const = 0;
    virtual bool isMouseHovering(const sf::Vector2f& mouseLocal) const;
    virtual void handleMouseMove(const sf::Vector2f& mouseLocal);
    virtual void handleMousePress(sf::Mouse::Button button, const sf::Vector2f& mouseLocal);
    virtual void handleMouseRelease(sf::Mouse::Button button, const sf::Vector2f& mouseLocal);
    virtual void handleTextEntered(uint32_t unicode);
    virtual void handleKeyPressed(sf::Keyboard::Key key);

    virtual void handleMouseEntered();
    virtual void handleMouseLeft();
    virtual void handleFocusChange(bool focused);

    Signal<Widget*> onMouseEnter;
    Signal<Widget*> onMouseLeave;
    Signal<Widget*> onFocusGained;
    Signal<Widget*> onFocusLost;

protected:
    Widget();

    virtual void setParentAndGui(Container* parent, Gui* gui);

private:
    sf::Transformable transformable_;
    Container* parent_;
    Gui* gui_;
    bool visible_, enabled_, focused_;

    friend class Container;
    friend class ContainerBase;
    friend class Gui;
};

}
