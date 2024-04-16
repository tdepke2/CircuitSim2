#include <gui/Theme.h>

#include <iostream>
#include <stdexcept>

namespace gui {

Theme::Theme(const Gui& gui) :
    gui_(gui),
    styles_() {
}

void Theme::addStyle(const sf::String& widgetName, std::shared_ptr<Style> style) {
    styles_[widgetName] = style;
}

bool Theme::removeStyle(const sf::String& widgetName) {
    return styles_.erase(widgetName) > 0;
}

std::shared_ptr<Style> Theme::getStyle(const sf::String& widgetName) const {
    auto styleIter = styles_.find(widgetName);
    if (styleIter != styles_.end()) {
        return styleIter->second;
    }

    auto style = loadStyle(widgetName);
    if (style == nullptr) {
        throw std::runtime_error("theme failed to find style for \"" + widgetName.toAnsiString() + "\".");
    }
    styles_[widgetName] = style;
    return style;
}

float Theme::computeTextMaxHeightRatio(const sf::Font& font) const {
    std::string characterSample = "";
    for (char c = '\u0020'; c < '\u007f'; ++c) {
        characterSample.push_back(c);
    }
    std::cout << "characterSample = [" << characterSample << "]\n";

    sf::Text text(characterSample, font);
    const auto bounds = text.getLocalBounds();
    return (bounds.top + bounds.height) / text.getCharacterSize();
}

}
