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
    virtual std::shared_ptr<PanelStyle> getPanelStyle() const override;
    virtual std::shared_ptr<TextBoxStyle> getTextBoxStyle() const override;
    virtual std::shared_ptr<MenuBarStyle> getMenuBarStyle() const override;

private:
    sf::Font consolasFont_;
    float consolasMaxHeightRatio_;
};

} // namespace gui
