#include "Simulator.h"
#include "UserInterface.h"
#include <stdexcept>

TextButton::TextButton() {}

TextButton::TextButton(const Font& font, const string& buttonText, const Color& textColor, unsigned int charSize, float x, float y, const Color& color1, const Color& color2, function<void(void)> action) {
    text.setFont(font);
    text.setString(buttonText);
    text.setFillColor(textColor);
    text.setCharacterSize(charSize);
    text.setPosition(5.0f, 0.0f);
    
    button.setSize(Vector2f(text.getLocalBounds().width + 10.0f, charSize * 1.5f));
    button.setFillColor(color1);
    button.setPosition(0.0f, 0.0f);
    setPosition(x, y);
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
            if (action != nullptr) {
                action();
            }
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

void TextButton::draw (RenderTarget& target, RenderStates states) const {
    states.transform *= getTransform();
    if (visible) {
        target.draw(button, states);
        target.draw(text, states);
    }
}

DropdownMenu::DropdownMenu() {}

DropdownMenu::DropdownMenu(const TextButton& button, const Color& backgroundColor) {
    this->button = button;
    background.setSize(Vector2f(0.0f, 0.0f));
    background.setFillColor(backgroundColor);
    background.setPosition(0.0f, button.button.getSize().y);
    setPosition(button.getPosition());
    visible = false;
}

void DropdownMenu::addMenuButton(const TextButton& menuButton) {
    menuButtons.push_back(menuButton);
    menuButtons.back().setPosition(0.0f, background.getSize().y);
    Vector2f newBackgroundSize(background.getSize().x, background.getSize().y + menuButton.button.getSize().y);
    if (background.getSize().x < menuButton.button.getSize().x) {
        newBackgroundSize.x = menuButton.button.getSize().x;
    }
    background.setSize(newBackgroundSize);
}

void DropdownMenu::update(int mouseX, int mouseY, bool clicked) {
    visible = true;
}

void DropdownMenu::draw (RenderTarget& target, RenderStates states) const {
    states.transform *= getTransform();
    if (visible) {
        target.draw(button, states);
        target.draw(background, states);
        for (const TextButton& b : menuButtons) {
            target.draw(b, states);
        }
    }
}

UserInterface::UserInterface() {
    if (!font.loadFromFile("resources/arial.ttf")) {
        throw runtime_error("\"resources/arial.ttf\": Unable to load font file.");
    }
    test = DropdownMenu(TextButton(font, "File", Color::White, 15, 5.0f, 5.0f, Color::Black, Color::Blue, nullptr), Color::Green);
    test.addMenuButton(TextButton(font, "Open...        Ctrl+O", Color::White, 15, 5.0f, 5.0f, Color::Black, Color::Blue, Simulator::doThing));
    
    test2 = TextButton(font, "Option", Color::White, 15, 100.0f, 70.0f, Color::Black, Color::Blue, nullptr);
    
    topBar.setSize(Vector2f(800.0f, 40.0f));
    topBar.setFillColor(Color(128, 128, 128));
    topBar.setPosition(0.0f, 0.0f);
}

void UserInterface::update(int mouseX, int mouseY, bool clicked) {
    test.update(mouseX, mouseY, clicked);
    test2.update(mouseX, mouseY, clicked);
}

void UserInterface::draw (RenderTarget& target, RenderStates states) const {
    states.transform *= getTransform();
    target.draw(topBar, states);
    target.draw(test, states);
    target.draw(test2, states);
}