#pragma once

#include <gui/Theme.h>

#include <memory>
#include <SFML/Graphics.hpp>

namespace gui {

class DefaultTheme : public Theme {
public:
    virtual ~DefaultTheme() noexcept = default;

    static std::shared_ptr<Theme> create();
    virtual std::shared_ptr<ButtonStyle> getButtonStyle() override;
    virtual std::shared_ptr<PanelStyle> getPanelStyle() override;
    virtual std::shared_ptr<TextBoxStyle> getTextBoxStyle() override;

protected:
    DefaultTheme();

private:
    sf::Font consolasFont_;
};

}
