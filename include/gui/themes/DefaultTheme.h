#pragma once

#include <gui/Theme.h>

#include <memory>
#include <SFML/Graphics.hpp>

namespace gui {
    class ButtonStyle;
    class CheckBoxStyle;
    class DialogBoxStyle;
    class LabelStyle;
    class MenuBarStyle;
    class MultilineTextBoxStyle;
    class PanelStyle;
    class RadioButtonStyle;
    class SliderStyle;
    class TextBoxStyle;
}

namespace gui {

class DefaultTheme : public Theme {
public:
    DefaultTheme(const Gui& gui);
    virtual ~DefaultTheme() = default;

protected:
    virtual std::shared_ptr<Style> loadStyle(const sf::String& widgetName) const override;

private:
    std::shared_ptr<ButtonStyle> makeButtonStyle() const;
    std::shared_ptr<CheckBoxStyle> makeCheckBoxStyle() const;
    std::shared_ptr<DialogBoxStyle> makeDialogBoxStyle() const;
    std::shared_ptr<LabelStyle> makeLabelStyle() const;
    std::shared_ptr<MenuBarStyle> makeMenuBarStyle() const;
    std::shared_ptr<MultilineTextBoxStyle> makeMultilineTextBoxStyle() const;
    std::shared_ptr<PanelStyle> makePanelStyle() const;
    std::shared_ptr<RadioButtonStyle> makeRadioButtonStyle() const;
    std::shared_ptr<SliderStyle> makeSliderStyle() const;
    std::shared_ptr<TextBoxStyle> makeTextBoxStyle() const;

    sf::Font consolasFont_;
    float consolasMaxHeightRatio_;
};

} // namespace gui
