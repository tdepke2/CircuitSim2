#pragma once

#include <gui/Theme.h>

#include <memory>
#include <SFML/Graphics.hpp>

namespace gui {

class DefaultTheme : public Theme {
public:
    static std::shared_ptr<Theme> create();
    std::shared_ptr<ButtonStyle> getButtonStyle() override;

protected:
    DefaultTheme();
};

}
