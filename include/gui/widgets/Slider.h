#pragma once

#include <gui/Signal.h>
#include <gui/Style.h>
#include <gui/Widget.h>

#include <memory>
#include <SFML/Graphics.hpp>
#include <utility>

namespace gui {
    class Gui;
    class Theme;
}

namespace gui {

/**
 * Visual styling for `Slider`.
 * 
 * One instance is shared between objects that use the same style, private
 * members in this class operate as flyweights.
 */
class SliderStyle : public Style {
public:
    SliderStyle(const Gui& gui);

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

    // sf::Shape interface.
    void setThumbTexture(const sf::Texture* texture, bool resetRect = false);
    void setThumbTextureRect(const sf::IntRect& rect);
    void setThumbFillColor(const sf::Color& color);
    void setThumbOutlineColor(const sf::Color& color);
    void setThumbOutlineThickness(float thickness);
    const sf::Texture* getThumbTexture() const;
    const sf::IntRect& getThumbTextureRect() const;
    const sf::Color& getThumbFillColor() const;
    const sf::Color& getThumbOutlineColor() const;
    float getThumbOutlineThickness() const;

    void setThumbMinWidth(float thumbMinWidth);
    float getThumbMinWidth() const;

    std::shared_ptr<SliderStyle> clone() const;

private:
    sf::RectangleShape rect_;
    sf::RectangleShape thumb_;
    float thumbMinWidth_;

    friend class Slider;
};


/**
 * FIXME
 */
class Slider : public Widget {
public:
    static std::shared_ptr<Slider> create(const Theme& theme, const sf::String& name = "");
    static std::shared_ptr<Slider> create(std::shared_ptr<SliderStyle> style, const sf::String& name = "");
    virtual ~Slider() noexcept = default;

    void setSize(const sf::Vector2f& size);
    void setRange(const std::pair<float, float>& range);
    void setValue(float value);
    void setStep(float step);
    const sf::Vector2f& getSize() const;
    const std::pair<float, float>& getRange() const;
    float getValue() const;
    float getStep() const;

    void setStyle(std::shared_ptr<SliderStyle> style);
    // Getting the style makes a local copy. Changes to this style will therefore not effect the theme.
    // To get the global style, get it from the theme.
    std::shared_ptr<SliderStyle> getStyle();

    virtual sf::FloatRect getLocalBounds() const override;

    virtual bool handleMouseMove(const sf::Vector2f& mouseParent) override;
    virtual void handleMousePress(sf::Mouse::Button button, const sf::Vector2f& mouseParent) override;
    virtual void handleMouseRelease(sf::Mouse::Button button, const sf::Vector2f& mouseParent) override;

    Signal<Widget*, float> onValueChange;

protected:
    Slider(std::shared_ptr<SliderStyle> style, const sf::String& name);

private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    std::shared_ptr<SliderStyle> style_;
    bool styleCopied_;

    sf::Vector2f size_;
    std::pair<float, float> range_;
    float value_, step_;
    bool isDragging_;
};

} // namespace gui
