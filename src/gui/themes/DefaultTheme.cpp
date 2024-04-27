#include <gui/themes/DefaultTheme.h>
#include <gui/widgets/Button.h>
#include <gui/widgets/CheckBox.h>
#include <gui/widgets/ColorPicker.h>
#include <gui/widgets/DialogBox.h>
#include <gui/widgets/Label.h>
#include <gui/widgets/MenuBar.h>
#include <gui/widgets/MultilineTextBox.h>
#include <gui/widgets/Panel.h>
#include <gui/widgets/RadioButton.h>
#include <gui/widgets/Slider.h>
#include <gui/widgets/TextBox.h>


#include <iostream>



namespace {

const sf::Color colorVeryLightGray(240, 240, 240);
const sf::Color colorLightGray(219, 219, 219);
const sf::Color colorLightGray2(180, 180, 180);
const sf::Color colorGray(153, 153, 153);
const sf::Color colorGray2(140, 140, 140);
const sf::Color colorGray3(110, 110, 110);

const sf::Color colorLightBlue(170, 204, 244);
const sf::Color colorLightBlueTransparent(38, 123, 227, 100);
const sf::Color colorDarkBlue(66, 150, 250);
const sf::Color colorDarkBlue2(15, 135, 250);

}

namespace gui {

DefaultTheme::DefaultTheme(const Gui& gui) :
    Theme(gui) {

    if (!consolasFont_.loadFromFile("resources/consolas.ttf")) {
        throw std::runtime_error("\"resources/consolas.ttf\": Unable to load font file.");
    }
    consolasMaxHeightRatio_ = computeTextMaxHeightRatio(consolasFont_);
}

std::shared_ptr<Style> DefaultTheme::loadStyle(const sf::String& widgetName) const {
    std::cout << "Loading style for \"" << widgetName.toAnsiString() << "\".\n";
    if (widgetName == "Button") {
        return makeButtonStyle();
    } else if (widgetName == "CheckBox") {
        return makeCheckBoxStyle();
    } else if (widgetName == "ColorPicker") {
        return makeColorPickerStyle();
    } else if (widgetName == "DialogBox") {
        return makeDialogBoxStyle();
    } else if (widgetName == "Label") {
        return makeLabelStyle();
    } else if (widgetName == "MenuBar") {
        return makeMenuBarStyle();
    } else if (widgetName == "MultilineTextBox") {
        return makeMultilineTextBoxStyle();
    } else if (widgetName == "Panel") {
        return makePanelStyle();
    } else if (widgetName == "RadioButton") {
        return makeRadioButtonStyle();
    } else if (widgetName == "Slider") {
        return makeSliderStyle();
    } else if (widgetName == "TextBox") {
        return makeTextBoxStyle();
    }

    // Normally if we don't find the widget, then fall back to the base class
    // version. In this case the base class doesn't implement it though.
    //return Theme::loadStyle(widgetName);
    return nullptr;
}

std::shared_ptr<ButtonStyle> DefaultTheme::makeButtonStyle() const {
    auto style = std::make_shared<ButtonStyle>(gui_);
    style->setFillColor(sf::Color::White);

    style->setFont(consolasFont_);
    style->setCharacterSize(15);
    style->setTextFillColor(sf::Color::Black);

    style->setFillColorHover(colorLightBlueTransparent);
    style->setFillColorDown(colorDarkBlue);
    style->setTextPadding({8.0f, 1.0f, consolasMaxHeightRatio_});

    return style;
}

std::shared_ptr<CheckBoxStyle> DefaultTheme::makeCheckBoxStyle() const {
    auto style = std::make_shared<CheckBoxStyle>(gui_);
    style->setFillColor(sf::Color::White);
    style->setOutlineColor(sf::Color::Black);
    style->setOutlineThickness(-1.0f);

    style->setFont(consolasFont_);
    style->setCharacterSize(15);
    style->setTextFillColor(sf::Color::Black);

    style->setFillColorHover(colorLightBlueTransparent);
    style->setFillColorChecked(colorDarkBlue);
    style->setTextPadding({8.0f, 1.0f, consolasMaxHeightRatio_});

    return style;
}

std::shared_ptr<ColorPickerStyle> DefaultTheme::makeColorPickerStyle() const {
    auto style = std::make_shared<ColorPickerStyle>(gui_);
    style->setOutlineColor(sf::Color::White);
    style->setOutlineThickness(-2.0f);

    style->setDotRadius(5.0f);

    return style;
}

std::shared_ptr<DialogBoxStyle> DefaultTheme::makeDialogBoxStyle() const {
    auto style = std::make_shared<DialogBoxStyle>(gui_);
    style->setFillColor(colorVeryLightGray);
    style->setOutlineColor(colorGray2);
    style->setOutlineThickness(-2.0f);

    style->setTitleBarFillColor(colorLightGray2);
    style->setTitleBarHeight(20.0f);
    style->setTitlePadding({0.0f, 0.0f});
    style->setButtonPadding({10.0f, 10.0f});

    return style;
}

std::shared_ptr<LabelStyle> DefaultTheme::makeLabelStyle() const {
    auto style = std::make_shared<LabelStyle>(gui_);

    style->setFont(consolasFont_);
    style->setCharacterSize(15);
    style->setTextFillColor(sf::Color::Black);

    style->setTextPadding({8.0f, 1.0f, consolasMaxHeightRatio_});

    return style;
}

std::shared_ptr<MenuBarStyle> DefaultTheme::makeMenuBarStyle() const {
    auto style = std::make_shared<MenuBarStyle>(gui_);
    style->setBarFillColor(colorLightGray);

    style->setMenuFillColor(sf::Color::White);
    style->setMenuOutlineColor(colorGray2);
    style->setMenuOutlineThickness(-1.0f);

    style->setFont(consolasFont_);
    style->setCharacterSize(15);
    style->setTextFillColor(sf::Color::Black);

    style->setBarTextPadding({10.0f, 3.0f, consolasMaxHeightRatio_});
    style->setMenuTextPadding({18.0f, 3.0f, consolasMaxHeightRatio_});
    style->setMinLeftRightTextWidth(40.0f);
    style->setDisabledTextFillColor(colorGray3);
    style->setHighlightFillColor(colorLightBlue);
    style->setDisabledHighlightFillColor(colorLightGray2);

    return style;
}

std::shared_ptr<MultilineTextBoxStyle> DefaultTheme::makeMultilineTextBoxStyle() const {
    auto style = std::make_shared<MultilineTextBoxStyle>(gui_);
    style->setFillColor(sf::Color::White);
    style->setOutlineColor(colorGray);
    style->setOutlineThickness(-1.0f);

    style->setFont(consolasFont_);
    style->setCharacterSize(15);
    style->setTextFillColor(sf::Color::Black);

    style->setReadOnlyFillColor(colorLightGray2);
    style->setDefaultTextFillColor(colorGray3);
    style->setCaretSize({2.0f, consolasMaxHeightRatio_ * style->getCharacterSize()});
    style->setCaretFillColor(colorDarkBlue);
    style->setTextPadding({8.0f, 1.0f, consolasMaxHeightRatio_});

    style->setHighlightFillColor(colorLightBlueTransparent);

    return style;
}

std::shared_ptr<PanelStyle> DefaultTheme::makePanelStyle() const {
    auto style = std::make_shared<PanelStyle>(gui_);
    style->setFillColor(colorVeryLightGray);
    style->setOutlineColor(colorGray2);
    style->setOutlineThickness(-2.0f);

    return style;
}

std::shared_ptr<RadioButtonStyle> DefaultTheme::makeRadioButtonStyle() const {
    auto style = std::make_shared<RadioButtonStyle>(gui_);
    style->setFillColor(sf::Color::White);
    style->setOutlineColor(sf::Color::Black);
    style->setOutlineThickness(-1.0f);

    style->setFont(consolasFont_);
    style->setCharacterSize(15);
    style->setTextFillColor(sf::Color::Black);

    style->setFillColorHover(colorLightBlueTransparent);
    style->setFillColorChecked(colorDarkBlue);
    style->setTextPadding({8.0f, 1.0f, consolasMaxHeightRatio_});

    style->setDiamond(true);

    return style;
}

std::shared_ptr<SliderStyle> DefaultTheme::makeSliderStyle() const {
    auto style = std::make_shared<SliderStyle>(gui_);
    style->setFillColor(sf::Color::White);

    style->setThumbFillColor(colorDarkBlue);

    style->setFillColorHover(colorLightBlueTransparent);
    style->setFillColorDown(colorDarkBlue2);
    style->setThumbMinWidth(10.0f);

    return style;
}

std::shared_ptr<TextBoxStyle> DefaultTheme::makeTextBoxStyle() const {
    auto style = std::make_shared<TextBoxStyle>(gui_);
    style->setFillColor(sf::Color::White);
    style->setOutlineColor(colorGray);
    style->setOutlineThickness(-1.0f);

    style->setFont(consolasFont_);
    style->setCharacterSize(15);
    style->setTextFillColor(sf::Color::Black);

    style->setReadOnlyFillColor(colorLightGray2);
    style->setDefaultTextFillColor(colorGray3);
    style->setCaretSize({2.0f, consolasMaxHeightRatio_ * style->getCharacterSize()});
    style->setCaretFillColor(colorDarkBlue);
    style->setTextPadding({8.0f, 1.0f, consolasMaxHeightRatio_});

    return style;
}

}
