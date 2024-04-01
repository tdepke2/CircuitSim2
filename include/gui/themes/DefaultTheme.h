#pragma once

#include <gui/Theme.h>

#include <memory>
#include <SFML/Graphics.hpp>

namespace gui {

class DefaultTheme : public Theme {
public:
    DefaultTheme(const Gui& gui);
    virtual ~DefaultTheme() noexcept = default;

    virtual std::shared_ptr<ButtonStyle> getButtonStyle() const override;
    virtual std::shared_ptr<CheckBoxStyle> getCheckBoxStyle() const override;
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
