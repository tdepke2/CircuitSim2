#pragma once

#include <memory>
#include <SFML/Graphics.hpp>

namespace gui {
    class ButtonStyle;
    class PanelStyle;
    class TextBoxStyle;
}

namespace gui {

class Theme {
public:
    virtual ~Theme() noexcept = default;

    virtual std::shared_ptr<ButtonStyle> getButtonStyle() = 0;
    virtual std::shared_ptr<PanelStyle> getPanelStyle() = 0;
    virtual std::shared_ptr<TextBoxStyle> getTextBoxStyle() = 0;

protected:
    Theme();

    std::shared_ptr<ButtonStyle> buttonStyle_;
    std::shared_ptr<PanelStyle> panelStyle_;
    std::shared_ptr<TextBoxStyle> textBoxStyle_;
};

}
