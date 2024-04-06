#pragma once

#include <gui/Theme.h>

#include <memory>
#include <SFML/Graphics.hpp>

namespace gui {

class DefaultTheme : public Theme {
public:
    DefaultTheme(const Gui& gui);
    virtual ~DefaultTheme() noexcept = default;

    // FIXME: this is getting frustrating, could we just have one template getStyle() function?
    // add template specializations for each type? then declare members in here for each widget instead of in the base class
    //   won't work, the base class would need those specializations.
    // should all the styles derive from a base class?

    // would be nice if users could create custom widgets, and add these into an existing theme.

    virtual std::shared_ptr<ButtonStyle> getButtonStyle() const override;
    virtual std::shared_ptr<CheckBoxStyle> getCheckBoxStyle() const override;
    virtual std::shared_ptr<DialogBoxStyle> getDialogBoxStyle() const override;
    virtual std::shared_ptr<LabelStyle> getLabelStyle() const override;
    virtual std::shared_ptr<MenuBarStyle> getMenuBarStyle() const override;
    virtual std::shared_ptr<MultilineTextBoxStyle> getMultilineTextBoxStyle() const override;
    virtual std::shared_ptr<PanelStyle> getPanelStyle() const override;
    virtual std::shared_ptr<RadioButtonStyle> getRadioButtonStyle() const override;
    virtual std::shared_ptr<TextBoxStyle> getTextBoxStyle() const override;

private:
    sf::Font consolasFont_;
    float consolasMaxHeightRatio_;
};

} // namespace gui
