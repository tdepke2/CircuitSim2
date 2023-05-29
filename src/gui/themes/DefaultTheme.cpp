#include <gui/Button.h>
#include <gui/Panel.h>
#include <gui/themes/DefaultTheme.h>

namespace gui {

std::shared_ptr<Theme> DefaultTheme::create() {
    return std::shared_ptr<Theme>(new DefaultTheme);
}

std::shared_ptr<ButtonStyle> DefaultTheme::getButtonStyle() {
    if (!buttonStyle_) {
        buttonStyle_ = std::make_shared<ButtonStyle>();
        buttonStyle_->setFillColor(sf::Color(240, 240, 240));

        buttonStyle_->setFont(consolasFont_);
        buttonStyle_->setCharacterSize(15);
        buttonStyle_->setTextFillColor(sf::Color::Black);

        buttonStyle_->setFillColorDown(sf::Color(188, 214, 255));
        buttonStyle_->setTextPadding(sf::Vector2f(8.0f, 1.0f));
    }
    return buttonStyle_;
}

std::shared_ptr<PanelStyle> DefaultTheme::getPanelStyle() {
    if (!panelStyle_) {
        panelStyle_ = std::make_shared<PanelStyle>();
        panelStyle_->setFillColor(sf::Color::White);
        panelStyle_->setOutlineColor(sf::Color(140, 140, 140));
        panelStyle_->setOutlineThickness(-2.0f);
    }
    return panelStyle_;
}

DefaultTheme::DefaultTheme() {
    if (!consolasFont_.loadFromFile("resources/consolas.ttf")) {
        throw std::runtime_error("\"resources/consolas.ttf\": Unable to load font file.");
    }
}

}
