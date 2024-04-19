#include <gui/themes/DefaultTheme.h>
#include <gui/widgets/Button.h>
#include <gui/widgets/CheckBox.h>
#include <gui/widgets/DialogBox.h>
#include <gui/widgets/Label.h>
#include <gui/widgets/MenuBar.h>
#include <gui/widgets/MultilineTextBox.h>
#include <gui/widgets/Panel.h>
#include <gui/widgets/RadioButton.h>
#include <gui/widgets/Slider.h>
#include <gui/widgets/TextBox.h>


#include <iostream>



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
    style->setFillColor({240, 240, 240});

    style->setFont(consolasFont_);
    style->setCharacterSize(15);
    style->setTextFillColor(sf::Color::Black);

    style->setFillColorHover({186, 222, 255, 100});
    style->setFillColorDown({188, 214, 255});
    style->setTextPadding({8.0f, 1.0f, consolasMaxHeightRatio_});

    return style;
}

std::shared_ptr<CheckBoxStyle> DefaultTheme::makeCheckBoxStyle() const {
    auto style = std::make_shared<CheckBoxStyle>(gui_);
    style->setFillColor({240, 240, 240});
    style->setOutlineColor(sf::Color::Black);
    style->setOutlineThickness(-2.0f);

    style->setFont(consolasFont_);
    style->setCharacterSize(15);
    style->setTextFillColor(sf::Color::Black);

    style->setFillColorHover({186, 222, 255, 100});
    style->setFillColorChecked({80, 80, 80});
    style->setTextPadding({8.0f, 1.0f, consolasMaxHeightRatio_});

    return style;
}

std::shared_ptr<DialogBoxStyle> DefaultTheme::makeDialogBoxStyle() const {
    auto style = std::make_shared<DialogBoxStyle>(gui_);
    style->setFillColor(sf::Color::White);
    style->setOutlineColor({140, 140, 140});
    style->setOutlineThickness(-2.0f);

    style->setTitleBarFillColor({80, 80, 80});
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
    style->setBarFillColor({240, 240, 240});

    style->setMenuFillColor({240, 240, 240});
    style->setMenuOutlineColor({140, 140, 140});
    style->setMenuOutlineThickness(-1.0f);

    style->setFont(consolasFont_);
    style->setCharacterSize(15);
    style->setTextFillColor(sf::Color::Black);

    style->setBarTextPadding({10.0f, 3.0f, consolasMaxHeightRatio_});
    style->setMenuTextPadding({18.0f, 3.0f, consolasMaxHeightRatio_});
    style->setMinLeftRightTextWidth(40.0f);
    style->setDisabledTextFillColor({127, 127, 127});
    style->setHighlightFillColor({188, 214, 255});
    style->setDisabledHighlightFillColor({180, 180, 180});

    return style;
}

std::shared_ptr<MultilineTextBoxStyle> DefaultTheme::makeMultilineTextBoxStyle() const {
    auto style = std::make_shared<MultilineTextBoxStyle>(gui_);
    style->setFillColor({240, 240, 240});
    style->setOutlineColor({190, 190, 190});
    style->setOutlineThickness(-1.0f);

    style->setFont(consolasFont_);
    style->setCharacterSize(15);
    style->setTextFillColor(sf::Color::Black);

    style->setReadOnlyFillColor({180, 180, 180});
    style->setDefaultTextFillColor({100, 100, 100});
    style->setCaretSize({2.0f, consolasMaxHeightRatio_ * style->getCharacterSize()});
    style->setCaretFillColor({0, 255, 255});
    style->setTextPadding({8.0f, 1.0f, consolasMaxHeightRatio_});

    style->setHighlightFillColor({90, 90, 150, 100});

    return style;
}

std::shared_ptr<PanelStyle> DefaultTheme::makePanelStyle() const {
    auto style = std::make_shared<PanelStyle>(gui_);
    style->setFillColor(sf::Color::White);
    style->setOutlineColor({140, 140, 140});
    style->setOutlineThickness(-2.0f);

    return style;
}

std::shared_ptr<RadioButtonStyle> DefaultTheme::makeRadioButtonStyle() const {
    auto style = std::make_shared<RadioButtonStyle>(gui_);
    style->setFillColor({240, 240, 240});
    style->setOutlineColor(sf::Color::Black);
    style->setOutlineThickness(-2.0f);

    style->setFont(consolasFont_);
    style->setCharacterSize(15);
    style->setTextFillColor(sf::Color::Black);

    style->setFillColorChecked({80, 80, 80});
    style->setTextPadding({8.0f, 1.0f, consolasMaxHeightRatio_});

    return style;
}

std::shared_ptr<SliderStyle> DefaultTheme::makeSliderStyle() const {
    auto style = std::make_shared<SliderStyle>(gui_);
    style->setFillColor({240, 240, 240});

    style->setThumbFillColor({80, 80, 80});

    style->setThumbMinWidth(10.0f);

    return style;
}

std::shared_ptr<TextBoxStyle> DefaultTheme::makeTextBoxStyle() const {
    auto style = std::make_shared<TextBoxStyle>(gui_);
    style->setFillColor({240, 240, 240});
    style->setOutlineColor({190, 190, 190});
    style->setOutlineThickness(-1.0f);

    style->setFont(consolasFont_);
    style->setCharacterSize(15);
    style->setTextFillColor(sf::Color::Black);

    style->setReadOnlyFillColor({180, 180, 180});
    style->setDefaultTextFillColor({100, 100, 100});
    style->setCaretSize({2.0f, consolasMaxHeightRatio_ * style->getCharacterSize()});
    style->setCaretFillColor({0, 255, 255});
    style->setTextPadding({8.0f, 1.0f, consolasMaxHeightRatio_});

    return style;
}

}
