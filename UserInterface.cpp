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
    mouseX -= static_cast<int>(getPosition().x);
    mouseY -= static_cast<int>(getPosition().y);
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
    background.setSize(Vector2f(0.0f, 4.0f));
    background.setFillColor(backgroundColor);
    background.setPosition(0.0f, button.button.getSize().y);
    setPosition(button.getPosition());
    this->button.setPosition(0.0f, 0.0f);
    maxMenuButtonWidth = 0.0f;
    visible = false;
}

void DropdownMenu::addMenuButton(const TextButton& menuButton) {
    menuButtons.push_back(menuButton);
    menuButtons.back().setPosition(background.getPosition() + Vector2f(5.0f, background.getSize().y - 2.0f));
    Vector2f newBackgroundSize(background.getSize().x, background.getSize().y + menuButton.button.getSize().y);
    if (newBackgroundSize.x < menuButton.button.getSize().x + 10.0f) {
        newBackgroundSize.x = menuButton.button.getSize().x + 10.0f;
        maxMenuButtonWidth = menuButton.button.getSize().x;
        for (int i = menuButtons.size() - 2; i >= 0; --i) {
            menuButtons[i].button.setSize(Vector2f(maxMenuButtonWidth, menuButtons[i].button.getSize().y));
        }
    } else if (menuButton.button.getSize().x < maxMenuButtonWidth) {
        menuButtons.back().button.setSize(Vector2f(maxMenuButtonWidth, menuButton.button.getSize().y));
    }
    background.setSize(newBackgroundSize);
}

void DropdownMenu::update(int mouseX, int mouseY, bool clicked) {
    mouseX -= static_cast<int>(getPosition().x);
    mouseY -= static_cast<int>(getPosition().y);
    if (visible) {
        for (TextButton& b : menuButtons) {
            b.update(mouseX, mouseY, clicked);
        }
    }
    if (button.update(mouseX, mouseY, clicked)) {
        visible = !visible;
    } else if (clicked) {
        visible = false;
    }
}

void DropdownMenu::draw (RenderTarget& target, RenderStates states) const {
    states.transform *= getTransform();
    target.draw(button, states);
    if (visible) {
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
    
    topBar.setSize(Vector2f(800.0f, 40.0f));
    topBar.setFillColor(Color::White);
    topBar.setPosition(0.0f, 0.0f);
    fileMenu = DropdownMenu(TextButton(font, "File", Color::Black, 15, 10.0f, 10.0f, Color::White, Color(214, 229, 255), nullptr), Color(240, 240, 240));
    fileMenu.addMenuButton(TextButton(font, "Open...             Ctrl+O", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::doThing));
    fileMenu.addMenuButton(TextButton(font, "Cool button", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), nullptr));
    fileMenu.addMenuButton(TextButton(font, "Click here", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), nullptr));
}

void UserInterface::update(int mouseX, int mouseY, bool clicked) {
    fileMenu.update(mouseX, mouseY, clicked);
}

void UserInterface::draw (RenderTarget& target, RenderStates states) const {
    states.transform *= getTransform();
    target.draw(topBar, states);
    target.draw(fileMenu, states);
}