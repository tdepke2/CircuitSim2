#include <gui/Button.h>
#include <gui/Panel.h>
#include <gui/TextBox.h>
#include <gui/themes/DefaultTheme.h>

namespace gui {

std::shared_ptr<Theme> DefaultTheme::create() {
    return std::shared_ptr<Theme>(new DefaultTheme);
}

std::shared_ptr<ButtonStyle> DefaultTheme::getButtonStyle() {
    if (!buttonStyle_) {
        buttonStyle_ = std::make_shared<ButtonStyle>();
        buttonStyle_->setFillColor({240, 240, 240});

        buttonStyle_->setFont(consolasFont_);
        buttonStyle_->setCharacterSize(15);
        buttonStyle_->setTextFillColor(sf::Color::Black);

        buttonStyle_->setFillColorDown({188, 214, 255});
        buttonStyle_->setTextPadding({8.0f, 1.0f});
    }
    return buttonStyle_;
}

std::shared_ptr<PanelStyle> DefaultTheme::getPanelStyle() {
    if (!panelStyle_) {
        panelStyle_ = std::make_shared<PanelStyle>();
        panelStyle_->setFillColor(sf::Color::White);
        panelStyle_->setOutlineColor({140, 140, 140});
        panelStyle_->setOutlineThickness(-2.0f);
    }
    return panelStyle_;
}

std::shared_ptr<TextBoxStyle> DefaultTheme::getTextBoxStyle() {
    if (!textBoxStyle_) {
        textBoxStyle_ = std::make_shared<TextBoxStyle>();
        textBoxStyle_->setFillColor({240, 240, 240});

        textBoxStyle_->setFont(consolasFont_);
        textBoxStyle_->setCharacterSize(15);
        textBoxStyle_->setTextFillColor(sf::Color::Black);

        textBoxStyle_->setCaretSize({2.0f, 16.0f});
        textBoxStyle_->setCaretFillColor({0, 255, 255});
        textBoxStyle_->setTextPadding({8.0f, 1.0f});
    }
    return textBoxStyle_;
}

DefaultTheme::DefaultTheme() {
    if (!consolasFont_.loadFromFile("resources/consolas.ttf")) {
        throw std::runtime_error("\"resources/consolas.ttf\": Unable to load font file.");
    }
}

}
