#pragma once

#include <gui/Button.h>

#include <memory>
#include <SFML/Graphics.hpp>

namespace gui {

class Theme {
public:
    virtual std::shared_ptr<ButtonStyle> getButtonStyle() = 0;

protected:
    Theme();

    std::shared_ptr<ButtonStyle> buttonStyle_;
};

}
