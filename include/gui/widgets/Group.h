#pragma once

#include <gui/Container.h>

#include <memory>
#include <SFML/Graphics.hpp>

namespace gui {

/**
 * An invisible widget that just groups other widgets.
 */
class Group : public Container {
    using baseClass = Container;

public:
    static std::shared_ptr<Group> create(const sf::String& name = "");
    virtual ~Group() = default;

    virtual sf::FloatRect getLocalBounds() const override;
    virtual bool isMouseIntersecting(const sf::Vector2f& mouseParent) const override;

    virtual bool handleMouseMove(const sf::Vector2f& mouseParent) override;
    virtual bool handleMouseWheelScroll(sf::Mouse::Wheel wheel, float delta, const sf::Vector2f& mouseParent) override;
    virtual void handleMousePress(sf::Mouse::Button button, const sf::Vector2f& mouseParent) override;
    virtual void handleMouseRelease(sf::Mouse::Button button, const sf::Vector2f& mouseParent) override;

protected:
    Group(const sf::String& name);

private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};

} // namespace gui
