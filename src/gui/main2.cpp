#include <gui/Button.h>
#include <gui/Gui.h>
#include <gui/MenuBar.h>
#include <gui/Panel.h>
#include <gui/TextBox.h>
#include <gui/themes/DefaultTheme.h>

#include <array>
#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <SFML/Graphics.hpp>
#include <string>

std::map<gui::Widget*, std::string> widgetNames;

// Signal callbacks (for debugging).
void mouseEnter(gui::Widget* w) {
    std::cout << "  onMouseEnter(\t\t" << w << " " << widgetNames[w] << ")\n";
}
void mouseLeave(gui::Widget* w) {
    std::cout << "  onMouseLeave(\t\t" << w << " " << widgetNames[w] << ")\n";
}
void focusGained(gui::Widget* w) {
    std::cout << "  onFocusGained(\t" << w << " " << widgetNames[w] << ")\n";
}
void focusLost(gui::Widget* w) {
    std::cout << "  onFocusLost(\t\t" << w << " " << widgetNames[w] << ")\n";
}
void mousePress(gui::Widget* w, sf::Mouse::Button button, const sf::Vector2f& pos) {
    std::cout << "  onMousePress(\t\t" << w << " " << widgetNames[w] << ", \t"
    << button << ", \t(" << pos.x << ", " << pos.y << "))\n";
}
void mouseRelease(gui::Widget* w, sf::Mouse::Button button, const sf::Vector2f& pos) {
    std::cout << "  onMouseRelease(\t" << w << " " << widgetNames[w] << ", \t"
    << button << ", \t(" << pos.x << ", " << pos.y << "))\n";
}
void click(gui::Widget* w, const sf::Vector2f& pos) {
    std::cout << "  onClick(\t\t" << w << " " << widgetNames[w] << ", \t\t("
    << pos.x << ", " << pos.y << "))\n";
}
void enterPressed(gui::Widget* w, const sf::String& text) {
    std::cout << "  onEnterPressed(\t" << w << " " << widgetNames[w] << ", \t\""
    << text.toAnsiString() << "\")\n";
}
void menuItemClick(gui::Widget* w, const gui::MenuList& menu, size_t index) {
    std::cout << "  onMenuItemClick(\t" << w << " " << widgetNames[w] << ", \t"
    << &menu << ", " << index << ")\n";
}

void connectDebugSignals(gui::Widget* widget, const std::string& name) {
    assert(widgetNames.emplace(widget, name).second);
    widget->onMouseEnter.connect(mouseEnter);
    widget->onMouseLeave.connect(mouseLeave);
    widget->onFocusGained.connect(focusGained);
    widget->onFocusLost.connect(focusLost);
}
void connectDebugSignals(gui::Button* button, const std::string& name) {
    connectDebugSignals(dynamic_cast<gui::Widget*>(button), name);
    button->onMousePress.connect(mousePress);
    button->onMouseRelease.connect(mouseRelease);
    button->onClick.connect(click);
}
void connectDebugSignals(gui::Panel* panel, const std::string& name) {
    connectDebugSignals(dynamic_cast<gui::Widget*>(panel), name);
}
void connectDebugSignals(gui::TextBox* textBox, const std::string& name) {
    connectDebugSignals(dynamic_cast<gui::Widget*>(textBox), name);
    textBox->onMousePress.connect(mousePress);
    textBox->onMouseRelease.connect(mouseRelease);
    textBox->onClick.connect(click);
    textBox->onEnterPressed.connect(enterPressed);
}
void connectDebugSignals(gui::MenuBar* menuBar, const std::string& name) {
    connectDebugSignals(dynamic_cast<gui::Widget*>(menuBar), name);
    menuBar->onMousePress.connect(mousePress);
    menuBar->onMouseRelease.connect(mouseRelease);
    menuBar->onClick.connect(click);
    menuBar->onMenuItemClick.connect(menuItemClick);
}

void createButtonDemo(gui::Gui& myGui, const gui::Theme& theme) {
    auto greetButton = gui::Button::create(theme);
    connectDebugSignals(greetButton.get(), "greetButton");
    greetButton->setLabel(sf::String("hello!"));
    greetButton->setRotation(18.0f);
    greetButton->setOrigin(greetButton->getSize() * 0.5f);
    greetButton->setPosition(130.0f, 100.0f);
    greetButton->setScale(1.6f, 2.5f);
    greetButton->onClick.connect([=]{ greetButton->setLabel("nice to meet you"); });
    myGui.addChild(greetButton);

    auto hideButton = gui::Button::create(theme);
    connectDebugSignals(hideButton.get(), "hideButton");
    hideButton->setLabel("click to hide");
    hideButton->setPosition(10.0f, 10.0f);
    hideButton->onClick.connect([=]{ hideButton->setVisible(false); });
    myGui.addChild(hideButton);

    auto disableButton = gui::Button::create(theme);
    connectDebugSignals(disableButton.get(), "disableButton");
    disableButton->setLabel("click to disable");
    disableButton->setPosition(200.0f, 10.0f);
    disableButton->onClick.connect([=]{ disableButton->setEnabled(false); });
    myGui.addChild(disableButton);

    auto resetButton = gui::Button::create(theme);
    connectDebugSignals(resetButton.get(), "resetButton");
    resetButton->setLabel("click to reset others");
    resetButton->setPosition(10.0f, 70.0f);
    resetButton->onClick.connect([=]{ hideButton->setVisible(true); disableButton->setEnabled(true); });
    myGui.addChild(resetButton);

    auto buttonA = gui::Button::create(theme);
    connectDebugSignals(buttonA.get(), "buttonA");
    buttonA->setLabel("A (front)");
    buttonA->setPosition(10.0f, 170.0f);
    buttonA->onClick.connect([=]{ buttonA->sendToFront(); });
    myGui.addChild(buttonA);

    auto buttonB = gui::Button::create(theme);
    connectDebugSignals(buttonB.get(), "buttonB");
    buttonB->setLabel("B (front)");
    buttonB->setPosition(50.0f, 170.0f);
    buttonB->onClick.connect([=]{ buttonB->sendToFront(); });
    myGui.addChild(buttonB);

    auto buttonC = gui::Button::create(theme);
    connectDebugSignals(buttonC.get(), "buttonC");
    buttonC->setLabel("C (back)");
    buttonC->setPosition(30.0f, 185.0f);
    buttonC->onClick.connect([=]{ buttonC->sendToBack(); });
    myGui.addChild(buttonC);


    auto moveButton = gui::Button::create(theme);
    connectDebugSignals(moveButton.get(), "moveButton");
    moveButton->setLabel("Hover to move");
    moveButton->setPosition(200.0f, 70.0f);
    moveButton->onMouseEnter.connect([=]{ moveButton->setPosition(200.0f, 60.0f); moveButton->setRotation(15); });
    moveButton->onMouseLeave.connect([=]{ moveButton->setPosition(200.0f, 70.0f); moveButton->setRotation(0); });
    myGui.addChild(moveButton);


    auto customStyled1 = gui::Button::create(theme);
    connectDebugSignals(customStyled1.get(), "customStyled1");
    auto customButtonStyle = customStyled1->getStyle();
    customButtonStyle->setTextPadding({0, 0, customButtonStyle->getTextPadding().z});
    customStyled1->setLabel("custom");
    customStyled1->setPosition(200.0f, 120.0f);
    customStyled1->onMouseEnter.connect([=]{ customButtonStyle->setTextStyle(sf::Text::Bold); });
    customStyled1->onMouseLeave.connect([=]{ customButtonStyle->setTextStyle(sf::Text::Regular); });
    myGui.addChild(customStyled1);

    auto customStyled2 = gui::Button::create(customStyled1->getStyle());
    connectDebugSignals(customStyled2.get(), "customStyled2");
    customStyled2->setLabel("styled");
    customStyled2->setPosition(200.0f, 150.0f);
    customStyled2->onMouseEnter.connect([=]{ customStyled1->getStyle()->setTextStyle(sf::Text::Bold); });
    customStyled2->onMouseLeave.connect([=]{ customStyled1->getStyle()->setTextStyle(sf::Text::Regular); });
    myGui.addChild(customStyled2);

    // This button switches to a different style on hover and also disables auto-resize.
    auto customStyled3 = gui::Button::create(customStyled1->getStyle());
    connectDebugSignals(customStyled3.get(), "customStyled3");
    customStyled3->setLabel("buttons");
    customStyled3->setAutoResize(false);
    customStyled3->setPosition(200.0f, 180.0f);
    customStyled3->onMouseEnter.connect([=]{ customStyled3->getStyle()->setTextStyle(sf::Text::Underlined); customStyled3->getStyle()->setFillColor({90, 90, 200}); });
    customStyled3->onMouseLeave.connect([=]{ customStyled3->getStyle()->setTextStyle(sf::Text::Regular); });
    myGui.addChild(customStyled3);

    customButtonStyle->setFillColor({20, 20, 200});

}

void createTextBoxDemo(gui::Gui& myGui, const gui::Theme& theme) {
    auto hideBox = gui::TextBox::create(theme);
    connectDebugSignals(hideBox.get(), "hideBox");
    hideBox->setWidthCharacters(16);
    hideBox->setText("right click hide");
    hideBox->setPosition(10.0f, 10.0f);
    hideBox->onMousePress.connect([=](gui::Widget* /*w*/, sf::Mouse::Button button, const sf::Vector2f& /*pos*/){
        if (button == sf::Mouse::Right) {
            hideBox->setVisible(false);
        }
    });
    myGui.addChild(hideBox);

    auto disableBox = gui::TextBox::create(theme);
    connectDebugSignals(disableBox.get(), "disableBox");
    disableBox->setWidthCharacters(16);
    disableBox->setText("right click disable");
    disableBox->setPosition(200.0f, 10.0f);
    disableBox->onMousePress.connect([=](gui::Widget* /*w*/, sf::Mouse::Button button, const sf::Vector2f& /*pos*/){
        if (button == sf::Mouse::Right) {
            disableBox->setEnabled(false);
        }
    });
    myGui.addChild(disableBox);

    auto resetBox = gui::TextBox::create(theme);
    connectDebugSignals(resetBox.get(), "resetBox");
    resetBox->setWidthCharacters(16);
    resetBox->setReadOnly(true);
    resetBox->setDefaultText("enter to reset  (trimmed text)");
    resetBox->setPosition(10.0f, 70.0f);
    resetBox->onEnterPressed.connect([=]{ hideBox->setVisible(true); disableBox->setEnabled(true); });
    myGui.addChild(resetBox);

    auto maxCharBox = gui::TextBox::create(theme);
    connectDebugSignals(maxCharBox.get(), "maxCharBox");
    maxCharBox->setWidthCharacters(16);
    maxCharBox->setMaxCharacters(9);
    maxCharBox->setDefaultText("max chars");
    maxCharBox->setPosition(200.0f, 90.0f);
    maxCharBox->setRotation(-33.0f);
    myGui.addChild(maxCharBox);
    maxCharBox->sendToBack();
}

void createMenuBarDemo(gui::Gui& myGui, const gui::Theme& theme) {
    auto menuBar = gui::MenuBar::create(theme);
    connectDebugSignals(menuBar.get(), "menuBar");
    menuBar->setWidth(300.0f);
    menuBar->setPosition(150.0f, 50.0f);
    menuBar->setRotation(-9.0f);
    menuBar->setOrigin(100.0f, 15.0f);

    gui::MenuList menuList("menu1");
    menuList.items.emplace_back("hi i am:", "menu1");
    menuList.items.emplace_back("insert left", "");
    menuList.items.emplace_back("insert right", "");
    menuList.items.emplace_back("remove left", "");
    menuList.items.emplace_back("remove right", "");
    menuList.items.emplace_back("remove me", "");
    menuList.items.emplace_back("???", "", false);
    menuList.items.emplace_back("ping", "");
    menuList.items.emplace_back("pong", "", false);
    menuBar->insertMenu(menuList);

    menuBar->onMenuItemClick.connect([=](gui::Widget* /*w*/, const gui::MenuList& menu, size_t index){
        size_t menuIndex = menuBar->findMenuIndex(menu.name);
        gui::MenuList newMenu = menu;
        if (newMenu.name.getSize() > 4) {
            newMenu.name = "menu" + std::to_string(std::stoi(newMenu.name.substring(4).toAnsiString()) + 1);
            newMenu.items[0].rightText = newMenu.name;
        }

        if (menu.items[index].leftText == "insert left") {
            menuBar->insertMenu(newMenu, menuIndex);
        } else if (menu.items[index].leftText == "insert right") {
            menuBar->insertMenu(newMenu, menuIndex + 1);
        } else if (menu.items[index].leftText == "remove left") {
            if (menuIndex > 0) {
                menuBar->removeMenu(menuIndex - 1);
            } else {
                std::cout << "No menu on the left side to remove!\n";
            }
        } else if (menu.items[index].leftText == "remove right") {
            menuBar->removeMenu(menuIndex + 1);
        } else if (menu.items[index].leftText == "remove me") {
            menuBar->removeMenu(menuIndex);
        } else if (menu.items[index].leftText == "ping") {
            newMenu = menu;
            newMenu.items[index].rightText = "bounced";
            newMenu.items[index].enabled = false;
            newMenu.items[index + 1].enabled = true;
            menuBar->setMenu(newMenu, menuIndex);
        } else if (menu.items[index].leftText == "pong") {
            newMenu = menu;
            newMenu.items[index].rightText = "bounced again";
            newMenu.items[index].enabled = false;
            newMenu.items[index - 1].enabled = true;
            menuBar->setMenu(newMenu, menuIndex);
        } else {
            std::cout << "That menu option doesn't do anything :(\n";
        }
    });

    menuList.name = "file";
    menuList.items.clear();
    menuList.items.emplace_back("quit", "Alt+Q");
    menuBar->insertMenu(menuList);

    myGui.addChild(menuBar);
}

void createFullDemo(gui::Gui& /*myGui*/, const gui::Theme& /*theme*/) {
    
}

int main() {
    sf::RenderWindow window(sf::VideoMode(400, 300), "GUI Test");

    gui::Gui myGui(window);

    gui::DefaultTheme theme(myGui);

    const std::array<std::string, 5> sceneNames = {
        "Empty",
        "ButtonDemo",
        "TextBoxDemo",
        "MenuBarDemo",
        "FullDemo"
    };
    size_t currentScene = 0;
    bool sceneChanged = true;

    auto sceneButton = gui::Button::create(theme);
    connectDebugSignals(sceneButton.get(), "sceneButton");
    sceneButton->setPosition(0.0f, window.getSize().y - 20.0f);
    sceneButton->onMousePress.connect([&](gui::Widget* /*w*/, sf::Mouse::Button button, const sf::Vector2f& /*pos*/) {
        if (button == sf::Mouse::Right && currentScene > 0) {
            --currentScene;
            sceneChanged = true;
        } else if (button == sf::Mouse::Left && currentScene + 1 < sceneNames.size()) {
            ++currentScene;
            sceneChanged = true;
        }
    });

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            myGui.handleEvent(event);
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::Resized) {
                //window.setView(sf::View(sf::FloatRect({0.0f, 0.0f}, sf::Vector2f(window.getSize()))));
            }
        }

        if (sceneChanged) {
            sceneButton->setLabel("Scene: " + sceneNames[currentScene] + " (left/right click here)");
            myGui.removeAllChildren();
            widgetNames.clear();
            assert(widgetNames.emplace(sceneButton.get(), "sceneButton").second);
            myGui.addChild(sceneButton);
            sceneChanged = false;

            std::cout << "Building scene " << sceneNames[currentScene] << "\n";
            if (sceneNames[currentScene] == "Empty") {
            } else if (sceneNames[currentScene] == "ButtonDemo") {
                createButtonDemo(myGui, theme);
            } else if (sceneNames[currentScene] == "TextBoxDemo") {
                createTextBoxDemo(myGui, theme);
            } else if (sceneNames[currentScene] == "MenuBarDemo") {
                createMenuBarDemo(myGui, theme);
            } else if (sceneNames[currentScene] == "FullDemo") {
                createFullDemo(myGui, theme);
            } else {
                std::cerr << "Failed to build scene: name not found.\n";
            }
        }

        window.clear(sf::Color(80, 80, 80));
        window.draw(myGui);
        window.display();
    }

    return 0;
}
