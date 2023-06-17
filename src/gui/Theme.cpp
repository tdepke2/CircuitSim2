#include <gui/Theme.h>

#include <iostream>

namespace gui {

Theme::Theme(const Gui& gui) :
    gui_(gui) {
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
