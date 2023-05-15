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
        //buttonStyle_->setFont();
    }
    return buttonStyle_;
}

DefaultTheme::DefaultTheme() {

}

}
