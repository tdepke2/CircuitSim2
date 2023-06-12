#pragma once

#include <gui/Theme.h>

#include <memory>
#include <SFML/Graphics.hpp>

namespace gui {

class DefaultTheme : public Theme {
public:
    DefaultTheme();
    virtual ~DefaultTheme() noexcept = default;

    virtual std::shared_ptr<ButtonStyle> getButtonStyle() const override;
    virtual std::shared_ptr<PanelStyle> getPanelStyle() const override;
    virtual std::shared_ptr<TextBoxStyle> getTextBoxStyle() const override;

private:
    sf::Font consolasFont_;
};

}
