#pragma once

#include <gui/Theme.h>

#include <memory>
#include <SFML/Graphics.hpp>

namespace gui {

class DefaultTheme : public Theme {
public:
    virtual ~DefaultTheme() = default;

    static std::shared_ptr<Theme> create();
    virtual std::shared_ptr<ButtonStyle> getButtonStyle() override;

protected:
    DefaultTheme();

private:
    sf::Font font_;
};

}
