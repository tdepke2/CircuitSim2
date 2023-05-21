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
        buttonStyle_->setFont(font_);
        buttonStyle_->setTextPadding(sf::Vector2f(8.0f, 1.0f));
    }
    return buttonStyle_;
}

DefaultTheme::DefaultTheme() {
    if (!font_.loadFromFile("resources/consolas.ttf")) {
        throw std::runtime_error("\"resources/consolas.ttf\": Unable to load font file.");
    }
}

}
