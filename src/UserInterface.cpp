#include "Board.h"
#include "Simulator.h"
#include "UserInterface.h"
#include <cassert>
#include <cmath>
#include <limits>
#include <stdexcept>

#include <iostream>

TextButton::TextButton() {}

TextButton::TextButton(const string& buttonText, const Color& textColor, unsigned int charSize, float x, float y, const Color& color1, const Color& color2, function<void(int)> action, int actionOption) {
    text.setFont(Board::getFont());
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
    if (visible) {
        states.transform *= getTransform();
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

TextField::TextField() {}

TextField::TextField(const string& labelText, const string& initialFieldText, const Color& textColor, unsigned int charSize, float x, float y, const Color& fillColor, const Color& outlineColor, int maxCharacters) {
    label.setFont(Board::getFont());
    label.setString(labelText);
    label.setFillColor(textColor);
    label.setCharacterSize(charSize);
    label.setPosition(0.0f, 0.0f);
    
    string maxSizeField(maxCharacters, ' ');
    field.setFont(Board::getFont());
    field.setString(maxSizeField);
    field.setFillColor(textColor);
    field.setCharacterSize(charSize);
    field.setPosition(label.getLocalBounds().width + 5.0f, 0.0f);
    
    background.setSize(Vector2f(field.getLocalBounds().width + 10.0f, charSize * 1.5f));
    background.setFillColor(fillColor);
    background.setOutlineColor(outlineColor);
    background.setOutlineThickness(-2.0f);
    background.setPosition(label.getLocalBounds().width, 0.0f);
    
    caretPosition = min(maxCharacters, static_cast<int>(initialFieldText.length()));
    caret.setSize(Vector2f(2.0f, charSize * 1.5f - 4.0f));
    caret.setFillColor(textColor);
    caret.setPosition(field.findCharacterPos(caretPosition).x, 2.0f);
    field.setString(initialFieldText);
    
    setPosition(x, y);
    this->maxCharacters = maxCharacters;
    visible = true;
    selected = false;
}

void TextField::update(int mouseX, int mouseY, bool clicked) {
    if (visible && clicked) {
        mouseX -= static_cast<int>(getPosition().x);
        mouseY -= static_cast<int>(getPosition().y);
        if (background.getGlobalBounds().contains(static_cast<float>(mouseX), static_cast<float>(mouseY))) {
            unsigned int closestIndex = 0;
            float closestDistance = numeric_limits<float>::max();
            for (unsigned int i = 0; i <= field.getString().getSize(); ++i) {
                float distance = fabs(mouseX - field.findCharacterPos(i).x);
                if (distance < closestDistance) {
                    closestIndex = i;
                    closestDistance = distance;
                }
            }
            caretPosition = closestIndex;
            caret.setPosition(field.findCharacterPos(caretPosition).x, 2.0f);
            selected = true;
        } else if (selected) {
            selected = false;
        }
    }
}

void TextField::update(Event::TextEvent textEvent) {
    if (visible && selected) {
        string s = field.getString();
        if (textEvent.unicode == 8 && caretPosition > 0) {    // Backspace.
            s.erase(caretPosition - 1, 1);
            field.setString(s);
            --caretPosition;
            caret.setPosition(field.findCharacterPos(caretPosition).x, 2.0f);
        } else if (textEvent.unicode == 27) {    // Escape.
            selected = false;
        } else if (textEvent.unicode >= 32 && textEvent.unicode <= 126 && static_cast<int>(s.length()) < maxCharacters) {    // Printable character.
            s.insert(caretPosition, 1, static_cast<char>(textEvent.unicode));
            field.setString(s);
            ++caretPosition;
            caret.setPosition(field.findCharacterPos(caretPosition).x, 2.0f);
        }
    }
}

void TextField::clear() {
    field.setString("");
    caretPosition = 0;
    caret.setPosition(field.findCharacterPos(caretPosition).x, 2.0f);
}

void TextField::draw(RenderTarget& target, RenderStates states) const {
    if (visible) {
        states.transform *= getTransform();
        target.draw(label, states);
        target.draw(background, states);
        target.draw(field, states);
        if (selected) {
            target.draw(caret, states);
        }
    }
}

DialogPrompt::DialogPrompt() {
    UserInterface::_dialogPrompts.push_back(this);
}

DialogPrompt::DialogPrompt(const string& dialogText, const Color& textColor, unsigned int charSize, float x, float y, const Color& fillColor, const Color& outlineColor, const Vector2f& size) {
    text.setFont(Board::getFont());
    text.setString(dialogText);
    text.setFillColor(textColor);
    text.setCharacterSize(charSize);
    text.setPosition(10.0f, 5.0f);
    
    background.setSize(size);
    background.setFillColor(fillColor);
    background.setOutlineColor(outlineColor);
    background.setOutlineThickness(-2.0f);
    background.setPosition(0.0f, 0.0f);
    
    setPosition(x, y);
    visible = false;
    UserInterface::_dialogPrompts.push_back(this);
}

DialogPrompt::~DialogPrompt() {
    for (auto vectorIter = UserInterface::_dialogPrompts.begin(); vectorIter != UserInterface::_dialogPrompts.end();) {
        if (*vectorIter == this) {
            vectorIter = UserInterface::_dialogPrompts.erase(vectorIter);
        } else {
            ++vectorIter;
        }
    }
}

void DialogPrompt::update(int mouseX, int mouseY, bool clicked) {
    if (visible) {
        mouseX -= static_cast<int>(getPosition().x);
        mouseY -= static_cast<int>(getPosition().y);
        for (TextButton& b : optionButtons) {
            b.update(mouseX, mouseY, clicked);
        }
        for (TextField& f : optionFields) {
            f.update(mouseX, mouseY, clicked);
        }
    }
}

void DialogPrompt::update(Event::TextEvent textEvent) {
    if (visible) {
        for (TextField& f : optionFields) {
            f.update(textEvent);
        }
    }
}

void DialogPrompt::clearFields() {
    for (TextField& textField : optionFields) {
        textField.clear();
    }
}

void DialogPrompt::show() {
    assert(!UserInterface::_dialogPromptOpen);
    visible = true;
    UserInterface::_dialogPromptOpen = true;
}

void DialogPrompt::draw(RenderTarget& target, RenderStates states) const {
    if (visible) {
        states.transform *= getTransform();
        target.draw(background, states);
        target.draw(text, states);
        for (const TextButton& b : optionButtons) {
            target.draw(b, states);
        }
        for (const TextField& f : optionFields) {
            target.draw(f, states);
        }
    }
}

bool UserInterface::_dialogPromptOpen = false;
vector<DialogPrompt*> UserInterface::_dialogPrompts;

bool UserInterface::isDialogPromptOpen() {
    return _dialogPromptOpen;
}

void UserInterface::closeAllDialogPrompts(int option) {
    for (DialogPrompt* dialogPrompt : _dialogPrompts) {
        dialogPrompt->visible = false;
    }
    _dialogPromptOpen = false;
}

UserInterface::UserInterface() {
    topBar.setSize(Vector2f(4000.0f, 28.0f));
    topBar.setFillColor(Color::White);
    topBar.setPosition(0.0f, 0.0f);
    
    fileMenu = DropdownMenu(TextButton(" File ", Color::Black, 15, 10.0f, 5.0f, Color::White, Color(214, 229, 255), nullptr), Color(240, 240, 240));
    fileMenu.addMenuButton(TextButton("  New          Ctrl+N", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::fileOption, 0));
    fileMenu.addMenuButton(TextButton("  Open...      Ctrl+O", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::fileOption, 1));
    fileMenu.addMenuButton(TextButton("  Save         Ctrl+S", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::fileOption, 2));
    fileMenu.addMenuButton(TextButton("  Save As...", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::fileOption, 3));
    fileMenu.addMenuButton(TextButton("  Rename...", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::fileOption, 4));
    fileMenu.addMenuButton(TextButton("  Resize...", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::fileOption, 5));
    fileMenu.addMenuButton(TextButton("  Exit", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::fileOption, 6));
    
    viewMenu = DropdownMenu(TextButton(" View ", Color::Black, 15, fileMenu.getPosition().x + fileMenu.button.button.getSize().x, 5.0f, Color::White, Color(214, 229, 255), nullptr), Color(240, 240, 240));
    viewMenu.addMenuButton(TextButton("  Toggle View/Edit Mode      Enter", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::viewOption, 0));
    viewMenu.addMenuButton(TextButton("  Zoom In           Mouse Wheel Up", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::viewOption, 1));
    viewMenu.addMenuButton(TextButton("  Zoom Out        Mouse Wheel Down", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::viewOption, 2));
    viewMenu.addMenuButton(TextButton("  Default Zoom", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::viewOption, 3));
    
    runMenu = DropdownMenu(TextButton(" Run ", Color::Black, 15, viewMenu.getPosition().x + viewMenu.button.button.getSize().x, 5.0f, Color::White, Color(214, 229, 255), nullptr), Color(240, 240, 240));
    runMenu.addMenuButton(TextButton("  Step Frame                 Tab", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::runOption, 0));
    runMenu.addMenuButton(TextButton("  Change Run Mode      Shift+Tab", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::runOption, 1));
    
    toolsMenu = DropdownMenu(TextButton(" Tools ", Color::Black, 15, runMenu.getPosition().x + runMenu.button.button.getSize().x, 5.0f, Color::White, Color(214, 229, 255), nullptr), Color(240, 240, 240));
    toolsMenu.addMenuButton(TextButton("  Select All                   Ctrl+A", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::toolsOption, 0));
    toolsMenu.addMenuButton(TextButton("  Deselect All                    Esc", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::toolsOption, 1));
    toolsMenu.addMenuButton(TextButton("  Rotate CW                         R", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::toolsOption, 2));
    toolsMenu.addMenuButton(TextButton("  Rotate CCW                  Shift+R", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::toolsOption, 3));
    toolsMenu.addMenuButton(TextButton("  Flip Across Vertical              F", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::toolsOption, 4));
    toolsMenu.addMenuButton(TextButton("  Flip Across Horizontal      Shift+F", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::toolsOption, 5));
    toolsMenu.addMenuButton(TextButton("  Cut                          Ctrl+X", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::toolsOption, 6));
    toolsMenu.addMenuButton(TextButton("  Copy                         Ctrl+C", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::toolsOption, 7));
    toolsMenu.addMenuButton(TextButton("  Paste                        Ctrl+V", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::toolsOption, 8));
    toolsMenu.addMenuButton(TextButton("  Delete                          DEL", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::toolsOption, 9));
    toolsMenu.addMenuButton(TextButton("  Wire Tool                         W", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::toolsOption, 10));
    
    wireMenu = DropdownMenu(TextButton(" Wire ", Color::Black, 15, toolsMenu.getPosition().x + toolsMenu.button.button.getSize().x, 5.0f, Color::White, Color(214, 229, 255), nullptr), Color(240, 240, 240));
    wireMenu.addMenuButton(TextButton("  Blank (Eraser)      Space", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::placeTile, 0));
    wireMenu.addMenuButton(TextButton("  Straight                T", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::placeTile, 1));
    wireMenu.addMenuButton(TextButton("  Corner                  C", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::placeTile, 2));
    wireMenu.addMenuButton(TextButton("  Tee               Shift+T", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::placeTile, 3));
    wireMenu.addMenuButton(TextButton("  Junction                J", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::placeTile, 4));
    wireMenu.addMenuButton(TextButton("  Crossover         Shift+C", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::placeTile, 5));
    
    inputMenu = DropdownMenu(TextButton(" Input ", Color::Black, 15, wireMenu.getPosition().x + wireMenu.button.button.getSize().x, 5.0f, Color::White, Color(214, 229, 255), nullptr), Color(240, 240, 240));
    inputMenu.addMenuButton(TextButton("  Switch            S", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::placeTile, 6));
    inputMenu.addMenuButton(TextButton("  Button      Shift+S", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::placeTile, 7));
    
    outputMenu = DropdownMenu(TextButton(" Output ", Color::Black, 15, inputMenu.getPosition().x + inputMenu.button.button.getSize().x, 5.0f, Color::White, Color(214, 229, 255), nullptr), Color(240, 240, 240));
    outputMenu.addMenuButton(TextButton("  LED      L", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::placeTile, 8));
    
    gateMenu = DropdownMenu(TextButton(" Gate ", Color::Black, 15, outputMenu.getPosition().x + outputMenu.button.button.getSize().x, 5.0f, Color::White, Color(214, 229, 255), nullptr), Color(240, 240, 240));
    gateMenu.addMenuButton(TextButton("  Diode                     D", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::placeTile, 9));
    gateMenu.addMenuButton(TextButton("  Buffer                    B", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::placeTile, 10));
    gateMenu.addMenuButton(TextButton("  NOT (Inverter)      Shift+B", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::placeTile, 11));
    gateMenu.addMenuButton(TextButton("  AND                       A", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::placeTile, 12));
    gateMenu.addMenuButton(TextButton("  NAND                Shift+A", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::placeTile, 13));
    gateMenu.addMenuButton(TextButton("  OR                        O", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::placeTile, 14));
    gateMenu.addMenuButton(TextButton("  NOR                 Shift+O", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::placeTile, 15));
    gateMenu.addMenuButton(TextButton("  XOR                       X", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::placeTile, 16));
    gateMenu.addMenuButton(TextButton("  XNOR                Shift+X", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::placeTile, 17));
    
    upsDisplay = TextButton(" Current UPS limit: 30        ", Color::Black, 15, gateMenu.getPosition().x + gateMenu.button.button.getSize().x + 30.0f, 5.0f, Color(10, 230, 10), Color::Black, nullptr);
    
    //savePrompt = 
    
    //renamePrompt = 
    
    resizePrompt = DialogPrompt("Enter the new board size. Note: if the new size\ntruncates the board, any objects that do not fit\non the new board will be deleted!", Color::Black, 15, 50.0f, 78.0f, Color::White, Color(140, 140, 140), Vector2f(418.0f, 170.0f));
    resizePrompt.optionButtons.emplace_back("Cancel", Color::Black, 15, 118.0f, 140.0f, Color(240, 240, 240), Color(188, 214, 255), closeAllDialogPrompts);
    resizePrompt.optionButtons.emplace_back("Resize", Color::Black, 15, 244.0f, 140.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::fileOption, 5);
    resizePrompt.optionFields.emplace_back("Width:  ", "", Color::Black, 15, 90.0f, 70.0f, Color::White, Color(214, 229, 255), 20);
    resizePrompt.optionFields.emplace_back("Height: ", "", Color::Black, 15, 90.0f, 100.0f, Color::White, Color(214, 229, 255), 20);
    
    //relabelPrompt = 
}

void UserInterface::update(int mouseX, int mouseY, bool clicked) {
    if (!_dialogPromptOpen) {
        fileMenu.update(mouseX, mouseY, clicked);
        viewMenu.update(mouseX, mouseY, clicked);
        runMenu.update(mouseX, mouseY, clicked);
        toolsMenu.update(mouseX, mouseY, clicked);
        wireMenu.update(mouseX, mouseY, clicked);
        inputMenu.update(mouseX, mouseY, clicked);
        outputMenu.update(mouseX, mouseY, clicked);
        gateMenu.update(mouseX, mouseY, clicked);
    }
    resizePrompt.update(mouseX, mouseY, clicked);
}

void UserInterface::update(Event::TextEvent textEvent) {
    resizePrompt.update(textEvent);
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
    target.draw(upsDisplay, states);
    target.draw(resizePrompt, states);
}