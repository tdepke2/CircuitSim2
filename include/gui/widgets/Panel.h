#pragma once

#include <gui/Style.h>
#include <gui/widgets/Group.h>

#include <memory>
#include <SFML/Graphics.hpp>

namespace gui {
    class Gui;
    class Theme;
}

namespace gui {

/**
 * Visual styling for `Panel`.
 * 
 * One instance is shared between objects that use the same style, private
 * members in this class operate as flyweights.
 */
class PanelStyle : public Style {
public:
    PanelStyle(const Gui& gui);
    virtual ~PanelStyle() = default;

    // sf::Shape interface.
    void setTexture(const sf::Texture* texture, bool resetRect = false);
    void setTextureRect(const sf::IntRect& rect);
    void setFillColor(const sf::Color& color);
    void setOutlineColor(const sf::Color& color);
    void setOutlineThickness(float thickness);
    const sf::Texture* getTexture() const;
    const sf::IntRect& getTextureRect() const;
    const sf::Color& getFillColor() const;
    const sf::Color& getOutlineColor() const;
    float getOutlineThickness() const;

    std::shared_ptr<PanelStyle> clone() const;

private:
    sf::RectangleShape rect_;

    friend class Panel;
};


/**
 * A rectangular box that contains other widgets.
 */
class Panel : public Group {
    using baseClass = Group;

public:
    static std::shared_ptr<Panel> create(const Theme& theme, const sf::String& name = "");
    static std::shared_ptr<Panel> create(std::shared_ptr<PanelStyle> style, const sf::String& name = "");
    virtual ~Panel() = default;

    void setSize(const sf::Vector2f& size);
    const sf::Vector2f& getSize() const;

    void setStyle(std::shared_ptr<PanelStyle> style);
    // Getting the style makes a local copy. Changes to this style will therefore not effect the theme.
    // To get the global style, get it from the theme.
    std::shared_ptr<PanelStyle> getStyle();

    virtual sf::FloatRect getLocalBounds() const override;
    virtual bool isMouseIntersecting(const sf::Vector2f& mouseParent) const override;

protected:
    Panel(std::shared_ptr<PanelStyle> style, const sf::String& name);

private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    std::shared_ptr<PanelStyle> style_;
    bool styleCopied_;
    sf::Vector2f size_;
};

} // namespace gui
