#include "Simulator.h"
#include "UserInterface.h"
#include <stdexcept>

TextButton::TextButton() {}

TextButton::TextButton(const Font& font, const string& buttonText, const Color& textColor, unsigned int charSize, float x, float y, const Color& color1, const Color& color2, function<void(void)> action) {
    text.setFont(font);
    text.setString(buttonText);
    text.setFillColor(textColor);
    text.setCharacterSize(charSize);
    text.setPosition(x + 5.0f, y);
    
    button.setSize(Vector2f(text.getLocalBounds().width + 10.0f, charSize * 1.5f));
    button.setFillColor(color1);
    button.setPosition(x, y);
    this->color1 = color1;
    this->color2 = color2;
    this->action = action;
    visible = true;
    selected = false;
}

bool TextButton::update(int mouseX, int mouseY, bool clicked) {
    if (!visible) {
        return false;
    }
    if (button.getGlobalBounds().contains(static_cast<float>(mouseX), static_cast<float>(mouseY))) {
        if (clicked) {
            action();
            button.setFillColor(color1);
            selected = false;
            return true;
        } else if (!selected) {
            button.setFillColor(color2);
            selected = true;
        }
    } else if (selected) {
        button.setFillColor(color1);
        selected = false;
    }
    return false;
}

UserInterface::UserInterface() {
    if (!font.loadFromFile("resources/arial.ttf")) {
        throw runtime_error("\"resources/arial.ttf\": Unable to load font file.");
    }
    testButton = TextButton(font, "Open...        Ctrl+O", Color::White, 15, 5.0f, 5.0f, Color::Black, Color::Blue, Simulator::doThing);
    
    topBar.setSize(Vector2f(800.0f, 40.0f));
    topBar.setFillColor(Color(128, 128, 128));
    topBar.setPosition(0.0f, 0.0f);
}

void UserInterface::update(int mouseX, int mouseY, bool clicked) {
    testButton.update(mouseX, mouseY, clicked);
}

void UserInterface::draw (RenderTarget& target, RenderStates states) const {
    states.transform *= getTransform();
    target.draw(topBar, states);
    if (testButton.visible) {
        target.draw(testButton.button, states);
        target.draw(testButton.text, states);
    }
}