#include "Simulator.h"
#include "UserInterface.h"
#include <stdexcept>

TextButton::TextButton() {}

TextButton::TextButton(const Font& font, const string& buttonText, const Color& textColor, unsigned int charSize, float x, float y, const Color& color1, const Color& color2, function<void(int)> action, int actionOption) {
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
    this->actionOption = actionOption;
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
                action(actionOption);
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

void TextButton::draw(RenderTarget& target, RenderStates states) const {
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
    menuButtons.back().setPosition(background.getPosition() + Vector2f(2.0f, background.getSize().y - 2.0f));
    Vector2f newBackgroundSize(background.getSize().x, background.getSize().y + menuButton.button.getSize().y);
    if (newBackgroundSize.x < menuButton.button.getSize().x + 4.0f) {
        newBackgroundSize.x = menuButton.button.getSize().x + 4.0f;
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
        bool clickDone = clicked, buttonSelected = false;
        for (TextButton& b : menuButtons) {
            if (b.update(mouseX, mouseY, clickDone)) {    // Only one button in the menu can be pressed.
                clickDone = false;
            }
            if (b.selected && !buttonSelected) {
                buttonSelected = true;
            } else if (b.selected) {
                b.button.setFillColor(b.color1);
                b.selected = false;
            }
        }
    }
    if (button.update(mouseX, mouseY, clicked)) {
        visible = !visible;
    } else if (clicked) {
        visible = false;
    }
}

void DropdownMenu::draw(RenderTarget& target, RenderStates states) const {
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
    if (!font.loadFromFile("resources/consolas.ttf")) {
        throw runtime_error("\"resources/consolas.ttf\": Unable to load font file.");
    }
    
    topBar.setSize(Vector2f(4000.0f, 28.0f));
    topBar.setFillColor(Color::White);
    topBar.setPosition(0.0f, 0.0f);
    
    fileMenu = DropdownMenu(TextButton(font, " File ", Color::Black, 15, 10.0f, 5.0f, Color::White, Color(214, 229, 255), nullptr), Color(240, 240, 240));
    fileMenu.addMenuButton(TextButton(font, "  New          Ctrl+N", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::fileOption, 0));
    fileMenu.addMenuButton(TextButton(font, "  Open...      Ctrl+O", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::fileOption, 1));
    fileMenu.addMenuButton(TextButton(font, "  Save         Ctrl+S", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::fileOption, 2));
    fileMenu.addMenuButton(TextButton(font, "  Save As...", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::fileOption, 3));
    fileMenu.addMenuButton(TextButton(font, "  Rename...", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::fileOption, 4));
    fileMenu.addMenuButton(TextButton(font, "  Resize...", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::fileOption, 5));
    fileMenu.addMenuButton(TextButton(font, "  Exit", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::fileOption, 6));
    
    viewMenu = DropdownMenu(TextButton(font, " View ", Color::Black, 15, fileMenu.getPosition().x + fileMenu.button.button.getSize().x, 5.0f, Color::White, Color(214, 229, 255), nullptr), Color(240, 240, 240));
    viewMenu.addMenuButton(TextButton(font, "  Toggle View/Edit Mode      Enter", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::viewOption, 0));
    viewMenu.addMenuButton(TextButton(font, "  Zoom In           Mouse Wheel Up", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::viewOption, 1));
    viewMenu.addMenuButton(TextButton(font, "  Zoom Out        Mouse Wheel Down", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::viewOption, 2));
    viewMenu.addMenuButton(TextButton(font, "  Default Zoom", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::viewOption, 3));
    
    runMenu = DropdownMenu(TextButton(font, " Run ", Color::Black, 15, viewMenu.getPosition().x + viewMenu.button.button.getSize().x, 5.0f, Color::White, Color(214, 229, 255), nullptr), Color(240, 240, 240));
    runMenu.addMenuButton(TextButton(font, "  Step Frame                 Tab", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::runOption, 0));
    runMenu.addMenuButton(TextButton(font, "  Change Run Mode      Shift+Tab", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::runOption, 1));
    
    toolsMenu = DropdownMenu(TextButton(font, " Tools ", Color::Black, 15, runMenu.getPosition().x + runMenu.button.button.getSize().x, 5.0f, Color::White, Color(214, 229, 255), nullptr), Color(240, 240, 240));
    toolsMenu.addMenuButton(TextButton(font, "  Select All                   Ctrl+A", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::toolsOption, 0));
    toolsMenu.addMenuButton(TextButton(font, "  Deselect All                    Esc", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::toolsOption, 1));
    toolsMenu.addMenuButton(TextButton(font, "  Rotate CW                         R", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::toolsOption, 2));
    toolsMenu.addMenuButton(TextButton(font, "  Rotate CCW                  Shift+R", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::toolsOption, 3));
    toolsMenu.addMenuButton(TextButton(font, "  Flip Across Vertical              F", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::toolsOption, 4));
    toolsMenu.addMenuButton(TextButton(font, "  Flip Across Horizontal      Shift+F", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::toolsOption, 5));
    toolsMenu.addMenuButton(TextButton(font, "  Cut                          Ctrl+X", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::toolsOption, 6));
    toolsMenu.addMenuButton(TextButton(font, "  Copy                         Ctrl+C", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::toolsOption, 7));
    toolsMenu.addMenuButton(TextButton(font, "  Paste                        Ctrl+V", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::toolsOption, 8));
    toolsMenu.addMenuButton(TextButton(font, "  Delete                          DEL", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::toolsOption, 9));
    toolsMenu.addMenuButton(TextButton(font, "  Wire Tool                         W", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::toolsOption, 10));
    
    wireMenu = DropdownMenu(TextButton(font, " Wire ", Color::Black, 15, toolsMenu.getPosition().x + toolsMenu.button.button.getSize().x, 5.0f, Color::White, Color(214, 229, 255), nullptr), Color(240, 240, 240));
    wireMenu.addMenuButton(TextButton(font, "  Blank (Eraser)      Space", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::placeTile, 0));
    wireMenu.addMenuButton(TextButton(font, "  Straight                T", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::placeTile, 1));
    wireMenu.addMenuButton(TextButton(font, "  Corner                  C", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::placeTile, 2));
    wireMenu.addMenuButton(TextButton(font, "  Tee               Shift+T", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::placeTile, 3));
    wireMenu.addMenuButton(TextButton(font, "  Junction                J", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::placeTile, 4));
    wireMenu.addMenuButton(TextButton(font, "  Crossover         Shift+C", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::placeTile, 5));
    
    inputMenu = DropdownMenu(TextButton(font, " Input ", Color::Black, 15, wireMenu.getPosition().x + wireMenu.button.button.getSize().x, 5.0f, Color::White, Color(214, 229, 255), nullptr), Color(240, 240, 240));
    inputMenu.addMenuButton(TextButton(font, "  Switch            S", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::placeTile, 6));
    inputMenu.addMenuButton(TextButton(font, "  Button      Shift+S", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::placeTile, 7));
    
    outputMenu = DropdownMenu(TextButton(font, " Output ", Color::Black, 15, inputMenu.getPosition().x + inputMenu.button.button.getSize().x, 5.0f, Color::White, Color(214, 229, 255), nullptr), Color(240, 240, 240));
    outputMenu.addMenuButton(TextButton(font, "  LED      L", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::placeTile, 8));
    
    gateMenu = DropdownMenu(TextButton(font, " Gate ", Color::Black, 15, outputMenu.getPosition().x + outputMenu.button.button.getSize().x, 5.0f, Color::White, Color(214, 229, 255), nullptr), Color(240, 240, 240));
    gateMenu.addMenuButton(TextButton(font, "  Diode                     D", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::placeTile, 9));
    gateMenu.addMenuButton(TextButton(font, "  Buffer                    B", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::placeTile, 10));
    gateMenu.addMenuButton(TextButton(font, "  NOT (Inverter)      Shift+B", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::placeTile, 11));
    gateMenu.addMenuButton(TextButton(font, "  AND                       A", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::placeTile, 12));
    gateMenu.addMenuButton(TextButton(font, "  NAND                Shift+A", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::placeTile, 13));
    gateMenu.addMenuButton(TextButton(font, "  OR                        O", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::placeTile, 14));
    gateMenu.addMenuButton(TextButton(font, "  NOR                 Shift+O", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::placeTile, 15));
    gateMenu.addMenuButton(TextButton(font, "  XOR                       X", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::placeTile, 16));
    gateMenu.addMenuButton(TextButton(font, "  XNOR                Shift+X", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::placeTile, 17));
}

void UserInterface::update(int mouseX, int mouseY, bool clicked) {
    fileMenu.update(mouseX, mouseY, clicked);
    viewMenu.update(mouseX, mouseY, clicked);
    runMenu.update(mouseX, mouseY, clicked);
    toolsMenu.update(mouseX, mouseY, clicked);
    wireMenu.update(mouseX, mouseY, clicked);
    inputMenu.update(mouseX, mouseY, clicked);
    outputMenu.update(mouseX, mouseY, clicked);
    gateMenu.update(mouseX, mouseY, clicked);
}

void UserInterface::draw(RenderTarget& target, RenderStates states) const {
    states.transform *= getTransform();
    target.draw(topBar, states);
    target.draw(fileMenu, states);
    target.draw(viewMenu, states);
    target.draw(runMenu, states);
    target.draw(toolsMenu, states);
    target.draw(wireMenu, states);
    target.draw(inputMenu, states);
    target.draw(outputMenu, states);
    target.draw(gateMenu, states);
}