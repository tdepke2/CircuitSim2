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

    virtual std::shared_ptr<ButtonStyle> getButtonStyle() const = 0;
    virtual std::shared_ptr<PanelStyle> getPanelStyle() const = 0;
    virtual std::shared_ptr<TextBoxStyle> getTextBoxStyle() const = 0;

protected:
    mutable std::shared_ptr<ButtonStyle> buttonStyle_;
    mutable std::shared_ptr<PanelStyle> panelStyle_;
    mutable std::shared_ptr<TextBoxStyle> textBoxStyle_;
};

}
