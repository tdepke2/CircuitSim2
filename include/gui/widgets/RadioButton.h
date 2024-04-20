#pragma once

#include <gui/Style.h>
#include <gui/widgets/Button.h>
#include <gui/widgets/CheckBox.h>

#include <memory>
#include <SFML/Graphics.hpp>

namespace gui {
    class Gui;
    class Theme;
}

namespace gui {

/**
 * Visual styling for `RadioButton`.
 * 
 * One instance is shared between objects that use the same style, private
 * members in this class operate as flyweights.
 */
class RadioButtonStyle : public CheckBoxStyle {
public:
    RadioButtonStyle(const Gui& gui);

    void setDiamond(bool diamond);
    bool isDiamond() const;

    std::shared_ptr<RadioButtonStyle> clone() const;

private:
    bool isDiamond_;

    friend class RadioButton;
};


/**
 * A button that is mutually exclusive. Setting the button checked will uncheck
 * all other `RadioButton` widgets contained in the parent. A `Group` widget can
 * be used to separate radio buttons from other unrelated ones.
 */
class RadioButton : public Button {
public:
    static std::shared_ptr<RadioButton> create(const Theme& theme, const sf::String& name = "");
    static std::shared_ptr<RadioButton> create(std::shared_ptr<RadioButtonStyle> style, const sf::String& name = "");
    virtual ~RadioButton() noexcept = default;

    void setPressed(bool pressed) = delete;
    bool isPressed() const = delete;
    void setChecked(bool checked);
    bool isChecked() const;
    void uncheckRadioButtons();

    void setStyle(std::shared_ptr<RadioButtonStyle> style);
    // Getting the style makes a local copy. Changes to this style will therefore not effect the theme.
    // To get the global style, get it from the theme.
    std::shared_ptr<RadioButtonStyle> getStyle();

    virtual void handleMouseRelease(sf::Mouse::Button button, const sf::Vector2f& mouseParent) override;

protected:
    RadioButton(std::shared_ptr<RadioButtonStyle> style, const sf::String& name);

private:
    virtual void computeResize() const override;
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    std::shared_ptr<RadioButtonStyle> style_;
    bool styleCopied_;
    bool isChecked_;
};

} // namespace gui
