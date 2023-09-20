#include <DebugScreen.h>

DebugScreen::DebugScreen(const sf::Font& font, unsigned int characterSize) :
    visible_(false),
    font_(font),
    characterSize_(characterSize),
    nextFieldPos_(2.0f, 0.0f) {

    fields_.reserve(static_cast<size_t>(Field::count));
    for (size_t i = 0; i < static_cast<size_t>(Field::count); ++i) {
        fields_.emplace_back(addField(false));
    }
}

void DebugScreen::setVisible(bool visible) {
    visible_ = visible;
}

bool DebugScreen::isVisible() const {
    return visible_;
}

sf::Text& DebugScreen::getField(Field field) {
    return fields_[static_cast<size_t>(field)];
}

sf::Text& DebugScreen::getField(const std::string& customName) {
    auto custom = customFields_.find(customName);
    if (custom != customFields_.end()) {
        return custom->second;
    }
    return customFields_.emplace(customName, addField(true)).first->second;
}

sf::Text DebugScreen::addField(bool isCustom) {
    sf::Text text("", font_, characterSize_);
    text.setPosition(nextFieldPos_);
    nextFieldPos_.y += 1.0f * characterSize_;
    if (!isCustom) {
        text.setFillColor(sf::Color::White);
    } else {
        text.setFillColor(sf::Color::Cyan);
    }
    return text;
}

void DebugScreen::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (!visible_) {
        return;
    }

    for (const auto& field : fields_) {
        target.draw(field, states);
    }
    for (const auto& custom : customFields_) {
        target.draw(custom.second, states);
    }
}
