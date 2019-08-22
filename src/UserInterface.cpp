#include "Board.h"
#include "Simulator.h"
#include "UserInterface.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

UIComponent::UIComponent() {
    visible = true;
}

UIComponent::~UIComponent() {}

bool UIComponent::update(int mouseX, int mouseY, bool clicked) {
    return false;
}

bool UIComponent::update(Event::TextEvent textEvent) {
    return false;
}

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

bool DropdownMenu::update(int mouseX, int mouseY, bool clicked) {
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
    return false;
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
    field.setString(initialFieldText.substr(0, maxCharacters));
    
    setPosition(x, y);
    this->maxCharacters = maxCharacters;
    selected = false;
}

void TextField::setString(const string& s) {
    field.setString(s.substr(0, maxCharacters));
    caretPosition = min(maxCharacters, static_cast<int>(s.length()));
    caret.setPosition(field.findCharacterPos(caretPosition).x, 2.0f);
}

bool TextField::update(int mouseX, int mouseY, bool clicked) {
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
    return false;
}

bool TextField::update(Event::TextEvent textEvent) {
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
    return false;
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

CheckBox::CheckBox() {}

CheckBox::CheckBox(const string& buttonText, const Color& textColor, unsigned int charSize, float x, float y, const Color& buttonColor, const Color& checkColor, bool startChecked) {
    const float BUTTON_HEIGHT = charSize * 1.5f;
    text.setFont(Board::getFont());
    text.setString(buttonText);
    text.setFillColor(textColor);
    text.setCharacterSize(charSize);
    text.setPosition(BUTTON_HEIGHT + 5.0f, 0.0f);
    
    button.setSize(Vector2f(BUTTON_HEIGHT + text.getLocalBounds().width + 10.0f, BUTTON_HEIGHT));
    button.setFillColor(buttonColor);
    button.setPosition(0.0f, 0.0f);
    
    check.setSize(Vector2f(BUTTON_HEIGHT * 0.8f, BUTTON_HEIGHT * 0.8f));
    if (!startChecked) {
        check.setFillColor(buttonColor);
    } else {
        check.setFillColor(checkColor);
    }
    check.setOutlineColor(Color::Black);
    check.setOutlineThickness(-2.0f);
    check.setPosition(BUTTON_HEIGHT * 0.2f, BUTTON_HEIGHT * 0.1f);
    
    setPosition(x, y);
    this->checkColor = checkColor;
    _checked = startChecked;
}

bool CheckBox::isChecked() const {
    return _checked;
}

void CheckBox::setChecked(bool checked) {
    if (!checked) {
        check.setFillColor(button.getFillColor());
        _checked = false;
    } else {
        check.setFillColor(checkColor);
        _checked = true;
    }
}

bool CheckBox::update(int mouseX, int mouseY, bool clicked) {
    if (visible) {
        mouseX -= static_cast<int>(getPosition().x);
        mouseY -= static_cast<int>(getPosition().y);
        if (button.getGlobalBounds().contains(static_cast<float>(mouseX), static_cast<float>(mouseY)) && clicked) {
            setChecked(!_checked);
        }
    }
    return false;
}

void CheckBox::draw(RenderTarget& target, RenderStates states) const {
    if (visible) {
        states.transform *= getTransform();
        target.draw(button, states);
        target.draw(text, states);
        target.draw(check, states);
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

bool DialogPrompt::update(int mouseX, int mouseY, bool clicked) {
    if (visible) {
        mouseX -= static_cast<int>(getPosition().x);
        mouseY -= static_cast<int>(getPosition().y);
        for (TextButton& b : optionButtons) {
            b.update(mouseX, mouseY, clicked);
        }
        for (TextField& f : optionFields) {
            f.update(mouseX, mouseY, clicked);
        }
        for (CheckBox& c : optionChecks) {
            c.update(mouseX, mouseY, clicked);
        }
    }
    return false;
}

bool DialogPrompt::update(Event::TextEvent textEvent) {
    if (visible) {
        if (textEvent.unicode == 9) {    // If key tab pressed, attempt to switch to next text field.
            for (unsigned int i = 0; i < optionFields.size(); ++i) {
                if (optionFields[i].selected == true) {
                    optionFields[i].selected = false;
                    optionFields[(i + 1) % optionFields.size()].selected = true;
                    break;
                }
            }
        } else if (textEvent.unicode == 13) {    // If key enter pressed, activate the first button (assumed as confirmation action).
            if (optionButtons[0].action != nullptr) {
                optionButtons[0].action(optionButtons[0].actionOption);
            }
        } else {    // Else, send the text event to the fields.
            for (TextField& f : optionFields) {
                f.update(textEvent);
            }
        }
    }
    return false;
}

void DialogPrompt::clearFields() {
    for (TextField& textField : optionFields) {
        textField.setString("");
    }
}

void DialogPrompt::show() {
    assert(!UserInterface::_dialogPromptOpen);
    visible = true;
    if (!optionFields.empty()) {
        for (TextField& f : optionFields) {
            f.selected = false;
        }
        UserInterface::fieldToSelectPtr = &optionFields[0];
    }
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
        for (const CheckBox& c : optionChecks) {
            target.draw(c, states);
        }
    }
}

TextField* UserInterface::fieldToSelectPtr = nullptr;
bool UserInterface::_dialogPromptOpen = false;
vector<DialogPrompt*> UserInterface::_dialogPrompts;
list<TextButton> UserInterface::_messageList;
Clock UserInterface::_messageClock;

bool UserInterface::isDialogPromptOpen() {
    return _dialogPromptOpen;
}

void UserInterface::closeAllDialogPrompts(int option) {
    for (DialogPrompt* dialogPrompt : _dialogPrompts) {
        dialogPrompt->visible = false;
    }
    _dialogPromptOpen = false;
}

void UserInterface::pushMessage(const string& s, bool isError) {
    cout << s << endl;
    if (_messageList.size() >= 4) {
        _messageList.pop_back();
        for (auto listIter = _messageList.begin(); listIter != _messageList.end(); ++listIter) {
            listIter->move(0.0f, 30.0f);
        }
        _messageClock.restart();
    } else if (_messageList.empty()) {
        _messageClock.restart();
    }
    
    Color messageColor;
    if (!isError) {
        messageColor = Color(31, 61, 255, 200);
    } else {
        messageColor = Color(255, 31, 31, 200);
    }
    _messageList.push_front(TextButton(s, Color(0, 0, 0, 200), 16, 10.0f, Simulator::getWindowSize().y - 35.0f - _messageList.size() * 30.0f, messageColor, Color::Black, nullptr));
}

void UserInterface::updateMessages() {
    int32_t currentMilliseconds = _messageClock.getElapsedTime().asMilliseconds();
    if (!_messageList.empty() && currentMilliseconds >= 4000) {
        if (currentMilliseconds >= 6000) {
            _messageList.pop_back();
            for (auto listIter = _messageList.begin(); listIter != _messageList.end(); ++listIter) {
                listIter->move(0.0f, 30.0f);
            }
            _messageClock.restart();
        } else {
            _messageList.back().text.setFillColor(Color(0, 0, 0, static_cast<uint8_t>((6000 - currentMilliseconds) * 0.1f)));
            _messageList.back().button.setFillColor(_messageList.back().color1 - Color(0, 0, 0, static_cast<uint8_t>((currentMilliseconds - 4000) * 0.1f)));
        }
    }
}

UserInterface::UserInterface() {
    topBar.setSize(Vector2f(static_cast<float>(Simulator::getWindowSize().x), 28.0f));
    topBar.setFillColor(Color::White);
    topBar.setPosition(0.0f, 0.0f);
    
    fileMenu = DropdownMenu(TextButton(" File ", Color::Black, 15, 10.0f, 5.0f, Color::White, Color(214, 229, 255), nullptr), Color(240, 240, 240));
    fileMenu.addMenuButton(TextButton("  New          Ctrl+N", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::fileOption, 0));
    fileMenu.addMenuButton(TextButton("  Open...      Ctrl+O", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::fileOption, 1));
    fileMenu.addMenuButton(TextButton("  Save         Ctrl+S", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::fileOption, 2));
    fileMenu.addMenuButton(TextButton("  Save As...", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::fileOption, 3));
    fileMenu.addMenuButton(TextButton("  Rename...", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::fileOption, 4));
    fileMenu.addMenuButton(TextButton("  Resize...", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::fileOption, 5));
    fileMenu.addMenuButton(TextButton("  Configuration...", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::fileOption, 6));
    fileMenu.addMenuButton(TextButton("  Exit", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::fileOption, 7));
    
    viewMenu = DropdownMenu(TextButton(" View ", Color::Black, 15, fileMenu.getPosition().x + fileMenu.button.button.getSize().x, 5.0f, Color::White, Color(214, 229, 255), nullptr), Color(240, 240, 240));
    viewMenu.addMenuButton(TextButton("  Toggle View/Edit Mode      Enter", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::viewOption, 0));
    viewMenu.addMenuButton(TextButton("  Zoom In           Mouse Wheel Up", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::viewOption, 1));
    viewMenu.addMenuButton(TextButton("  Zoom Out        Mouse Wheel Down", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::viewOption, 2));
    viewMenu.addMenuButton(TextButton("  Default Zoom", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::viewOption, 3));
    
    runMenu = DropdownMenu(TextButton(" Run ", Color::Black, 15, viewMenu.getPosition().x + viewMenu.button.button.getSize().x, 5.0f, Color::White, Color(214, 229, 255), nullptr), Color(240, 240, 240));
    runMenu.addMenuButton(TextButton("  Step One Tick             Tab", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::runOption, 0));
    runMenu.addMenuButton(TextButton("  Change Max TPS      Shift+Tab", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::runOption, 1));
    
    toolsMenu = DropdownMenu(TextButton(" Tools ", Color::Black, 15, runMenu.getPosition().x + runMenu.button.button.getSize().x, 5.0f, Color::White, Color(214, 229, 255), nullptr), Color(240, 240, 240));
    toolsMenu.addMenuButton(TextButton("  Select All                   Ctrl+A", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::toolsOption, 0));
    toolsMenu.addMenuButton(TextButton("  Deselect All                    ESC", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::toolsOption, 1));
    toolsMenu.addMenuButton(TextButton("  Rotate CW                         R", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::toolsOption, 2));
    toolsMenu.addMenuButton(TextButton("  Rotate CCW                  Shift+R", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::toolsOption, 3));
    toolsMenu.addMenuButton(TextButton("  Flip Across Vertical              F", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::toolsOption, 4));
    toolsMenu.addMenuButton(TextButton("  Flip Across Horizontal      Shift+F", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::toolsOption, 5));
    toolsMenu.addMenuButton(TextButton("  Toggle State                      E", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::toolsOption, 6));
    toolsMenu.addMenuButton(TextButton("  Edit/Alternative Tile       Shift+E", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::toolsOption, 7));
    toolsMenu.addMenuButton(TextButton("  Cut                          Ctrl+X", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::toolsOption, 8));
    toolsMenu.addMenuButton(TextButton("  Copy                         Ctrl+C", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::toolsOption, 9));
    toolsMenu.addMenuButton(TextButton("  Paste                        Ctrl+V", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::toolsOption, 10));
    toolsMenu.addMenuButton(TextButton("  Delete                          DEL", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::toolsOption, 11));
    toolsMenu.addMenuButton(TextButton("  Wire Tool                         W", Color::Black, 15, 0.0f, 0.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::toolsOption, 12));
    
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
    
    tpsDisplay = TextButton(" Current TPS limit: 30        ", Color::Black, 15, gateMenu.getPosition().x + gateMenu.button.button.getSize().x + 30.0f, 5.0f, Color(10, 230, 10), Color::Black, nullptr);
    
    savePrompt = DialogPrompt("Are you sure? Changes have not been saved.", Color::Black, 15, 50.0f, 78.0f, Color::White, Color(140, 140, 140), Vector2f(418.0f, 75.0f));
    savePrompt.optionButtons.emplace_back("Save", Color::Black, 15, 325.0f, 45.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::fileOption, 2);
    savePrompt.optionButtons.emplace_back("Discard Changes", Color::Black, 15, 150.0f, 45.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::fileOption);
    savePrompt.optionButtons.emplace_back("Cancel", Color::Black, 15, 45.0f, 45.0f, Color(240, 240, 240), Color(188, 214, 255), closeAllDialogPrompts);
    
    renamePrompt = DialogPrompt("Enter the new board name.", Color::Black, 15, 50.0f, 78.0f, Color::White, Color(140, 140, 140), Vector2f(418.0f, 103.0f));
    renamePrompt.optionButtons.emplace_back("Rename", Color::Black, 15, 244.0f, 73.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::fileOption, 4);
    renamePrompt.optionButtons.emplace_back("Cancel", Color::Black, 15, 118.0f, 73.0f, Color(240, 240, 240), Color(188, 214, 255), closeAllDialogPrompts);
    renamePrompt.optionFields.emplace_back("Name: ", "", Color::Black, 15, 20.0f, 33.0f, Color::White, Color(214, 229, 255), 40);
    
    resizePrompt = DialogPrompt("Enter the new board size. Note: if the new size\ntruncates the board, any objects that do not fit\non the new board will be deleted!", Color::Black, 15, 50.0f, 78.0f, Color::White, Color(140, 140, 140), Vector2f(418.0f, 170.0f));
    resizePrompt.optionButtons.emplace_back("Resize", Color::Black, 15, 244.0f, 140.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::fileOption, 5);
    resizePrompt.optionButtons.emplace_back("Cancel", Color::Black, 15, 118.0f, 140.0f, Color(240, 240, 240), Color(188, 214, 255), closeAllDialogPrompts);
    resizePrompt.optionFields.emplace_back("Width:  ", "", Color::Black, 15, 90.0f, 70.0f, Color::White, Color(214, 229, 255), 20);
    resizePrompt.optionFields.emplace_back("Height: ", "", Color::Black, 15, 90.0f, 100.0f, Color::White, Color(214, 229, 255), 20);
    
    relabelPrompt = DialogPrompt("Enter the new label for this switch/button. This\nwill bind the tile to the corresponding keyboard\nkey, a space functions as a null keybind.", Color::Black, 15, 50.0f, 78.0f, Color::White, Color(140, 140, 140), Vector2f(418.0f, 140.0f));
    relabelPrompt.optionButtons.emplace_back("Relabel", Color::Black, 15, 244.0f, 110.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::relabelTarget);
    relabelPrompt.optionButtons.emplace_back("Cancel", Color::Black, 15, 118.0f, 110.0f, Color(240, 240, 240), Color(188, 214, 255), closeAllDialogPrompts);
    relabelPrompt.optionFields.emplace_back("Label (one character): ", "", Color::Black, 15, 112.0f, 70.0f, Color::White, Color(214, 229, 255), 1);
    
    configPrompt = DialogPrompt("Configuration menu.", Color::Black, 15, 50.0f, 78.0f, Color::White, Color(140, 140, 140), Vector2f(418.0f, 260.0f));
    configPrompt.optionButtons.emplace_back("Close", Color::Black, 15, 181.0f, 230.0f, Color(240, 240, 240), Color(188, 214, 255), Simulator::fileOption, 6);
    configPrompt.optionFields.emplace_back("Slow TPS limit:   ", "", Color::Black, 15, 90.0f, 40.0f, Color::White, Color(214, 229, 255), 9);
    configPrompt.optionFields.emplace_back("Medium TPS limit: ", "", Color::Black, 15, 90.0f, 70.0f, Color::White, Color(214, 229, 255), 9);
    configPrompt.optionFields.emplace_back("Fast TPS limit:   ", "", Color::Black, 15, 90.0f, 100.0f, Color::White, Color(214, 229, 255), 9);
    configPrompt.optionChecks.emplace_back("Tri-state logic rules (new boards)", Color::Black, 15, 90.0f, 130.0f, Color::White, Color(80, 80, 80));
    configPrompt.optionChecks.emplace_back("Tri-state logic rules (this board)", Color::Black, 15, 90.0f, 160.0f, Color::White, Color(80, 80, 80));
    configPrompt.optionChecks.emplace_back("Pause on state conflict", Color::Black, 15, 90.0f, 190.0f, Color::White, Color(80, 80, 80));
}

bool UserInterface::update(int mouseX, int mouseY, bool clicked) {
    if (!_dialogPromptOpen) {
        fileMenu.update(mouseX, mouseY, clicked);
        viewMenu.update(mouseX, mouseY, clicked);
        runMenu.update(mouseX, mouseY, clicked);
        toolsMenu.update(mouseX, mouseY, clicked);
        wireMenu.update(mouseX, mouseY, clicked);
        inputMenu.update(mouseX, mouseY, clicked);
        outputMenu.update(mouseX, mouseY, clicked);
        gateMenu.update(mouseX, mouseY, clicked);
    } else {
        savePrompt.update(mouseX, mouseY, clicked);
        renamePrompt.update(mouseX, mouseY, clicked);
        resizePrompt.update(mouseX, mouseY, clicked);
        relabelPrompt.update(mouseX, mouseY, clicked);
        configPrompt.update(mouseX, mouseY, clicked);
    }
    return false;
}

bool UserInterface::update(Event::TextEvent textEvent) {
    if (_dialogPromptOpen) {
        savePrompt.update(textEvent);
        renamePrompt.update(textEvent);
        resizePrompt.update(textEvent);
        relabelPrompt.update(textEvent);
        configPrompt.update(textEvent);
    }
    return false;
}

void UserInterface::update(Event::SizeEvent sizeEvent) {
    topBar.setSize(Vector2f(static_cast<float>(sizeEvent.width), 28.0f));
    int i = _messageList.size() - 1;
    for (auto listIter = _messageList.begin(); listIter != _messageList.end(); ++listIter) {
        listIter->setPosition(10.0f, sizeEvent.height - 35.0f - i * 30.0f);
        --i;
    }
}

void UserInterface::draw(RenderTarget& target, RenderStates states) const {
    states.transform *= getTransform();
    for (auto listIter = _messageList.cbegin(); listIter != _messageList.cend(); ++listIter) {
        target.draw(*listIter, states);
    }
    target.draw(topBar, states);
    target.draw(fileMenu, states);
    target.draw(viewMenu, states);
    target.draw(runMenu, states);
    target.draw(toolsMenu, states);
    target.draw(wireMenu, states);
    target.draw(inputMenu, states);
    target.draw(outputMenu, states);
    target.draw(gateMenu, states);
    target.draw(tpsDisplay, states);
    target.draw(savePrompt, states);
    target.draw(renamePrompt, states);
    target.draw(resizePrompt, states);
    target.draw(relabelPrompt, states);
    target.draw(configPrompt, states);
}