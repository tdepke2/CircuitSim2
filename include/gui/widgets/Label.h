#pragma once

#include <gui/Signal.h>
#include <gui/Widget.h>

#include <memory>
#include <SFML/Graphics.hpp>

namespace gui {
    class Gui;
    class Theme;
}

namespace gui {

/**
 * Visual styling for `Label`.
 * 
 * One instance is shared between objects that use the same style, private
 * members in this class operate as flyweights.
 */
class LabelStyle {
public:
    LabelStyle(const Gui& gui);

    // sf::Text interface.
    void setFont(const sf::Font& font);
    void setCharacterSize(unsigned int size);
    void setLineSpacing(float spacingFactor);
    void setLetterSpacing(float spacingFactor);
    void setTextStyle(uint32_t style);
    void setTextFillColor(const sf::Color& color);
    const sf::Font* getFont() const;
    unsigned int getCharacterSize() const;
    float getLineSpacing() const;
    float getLetterSpacing() const;
    uint32_t getTextStyle() const;
    const sf::Color& getTextFillColor() const;

    void setTextPadding(const sf::Vector3f& padding);
    const sf::Vector3f& getTextPadding() const;

    std::shared_ptr<LabelStyle> clone() const;

private:
    const Gui& gui_;
    sf::Text text_;
    sf::Vector3f textPadding_;

    friend class Label;
};


/**
 * Basic text which can be used to label other widgets on the `Gui`.
 */
class Label : public Widget {
public:
    static std::shared_ptr<Label> create(const Theme& theme);
    static std::shared_ptr<Label> create(std::shared_ptr<LabelStyle> style);
    virtual ~Label() noexcept = default;

    void setLabel(const sf::String& label);
    const sf::Vector2f& getSize() const;
    const sf::String& getLabel() const;

    void setStyle(std::shared_ptr<LabelStyle> style);
    // Getting the style makes a local copy. Changes to this style will therefore not effect the theme.
    // To get the global style, get it from the theme.
    std::shared_ptr<LabelStyle> getStyle();

    virtual sf::FloatRect getLocalBounds() const override;

protected:
    Label(std::shared_ptr<LabelStyle> style);

private:
    void computeResize() const;
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    std::shared_ptr<LabelStyle> style_;
    bool styleCopied_;

    mutable sf::Vector2f size_;
    sf::String label_;
};

} // namespace gui
