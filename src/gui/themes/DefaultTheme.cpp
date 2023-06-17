#include <gui/Button.h>
#include <gui/Panel.h>
#include <gui/TextBox.h>
#include <gui/themes/DefaultTheme.h>


#include <iostream>



namespace gui {

DefaultTheme::DefaultTheme(const Gui& gui) :
    Theme(gui) {

    if (!consolasFont_.loadFromFile("resources/consolas.ttf")) {
        throw std::runtime_error("\"resources/consolas.ttf\": Unable to load font file.");
    }
    consolasMaxHeightRatio_ = computeTextMaxHeightRatio(consolasFont_);
}

std::shared_ptr<ButtonStyle> DefaultTheme::getButtonStyle() const {
    if (!buttonStyle_) {
        buttonStyle_ = std::make_shared<ButtonStyle>(gui_);
        buttonStyle_->setFillColor({240, 240, 240});

        buttonStyle_->setFont(consolasFont_);
        buttonStyle_->setCharacterSize(15);
        buttonStyle_->setTextFillColor(sf::Color::Black);

        buttonStyle_->setFillColorDown({188, 214, 255});
        buttonStyle_->setTextPadding({8.0f, 1.0f, consolasMaxHeightRatio_});
    }
    return buttonStyle_;
}

std::shared_ptr<PanelStyle> DefaultTheme::getPanelStyle() const {
    if (!panelStyle_) {
        panelStyle_ = std::make_shared<PanelStyle>(gui_);
        panelStyle_->setFillColor(sf::Color::White);
        panelStyle_->setOutlineColor({140, 140, 140});
        panelStyle_->setOutlineThickness(-2.0f);
    }
    return panelStyle_;
}

std::shared_ptr<TextBoxStyle> DefaultTheme::getTextBoxStyle() const {
    if (!textBoxStyle_) {
        textBoxStyle_ = std::make_shared<TextBoxStyle>(gui_);
        textBoxStyle_->setFillColor({240, 240, 240});
        textBoxStyle_->setOutlineColor({190, 190, 190});
        textBoxStyle_->setOutlineThickness(-1.0f);

        textBoxStyle_->setFont(consolasFont_);
        textBoxStyle_->setCharacterSize(15);
        textBoxStyle_->setTextFillColor(sf::Color::Black);

        textBoxStyle_->setCaretSize({2.0f, 16.0f});
        textBoxStyle_->setCaretFillColor({0, 255, 255});
        textBoxStyle_->setTextPadding({8.0f, 1.0f, consolasMaxHeightRatio_});
    }
    return textBoxStyle_;
}

}
