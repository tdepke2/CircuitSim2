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
        buttonStyle_->setFillColor(sf::Color::Red);
        buttonStyle_->setFillColorDown(sf::Color::Blue);
        buttonStyle_->setFont(consolasFont_);
        buttonStyle_->setTextPadding(sf::Vector2f(8.0f, 1.0f));
    }
    return buttonStyle_;
}

std::shared_ptr<PanelStyle> DefaultTheme::getPanelStyle() {
    if (!panelStyle_) {
        panelStyle_ = std::make_shared<PanelStyle>();
        panelStyle_->setFillColor(sf::Color::Magenta);
    }
    return panelStyle_;
}

DefaultTheme::DefaultTheme() {
    if (!consolasFont_.loadFromFile("resources/consolas.ttf")) {
        throw std::runtime_error("\"resources/consolas.ttf\": Unable to load font file.");
    }
}

}
