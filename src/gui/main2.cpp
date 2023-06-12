#include <gui/Button.h>
#include <gui/Gui.h>
#include <gui/Panel.h>
#include <gui/TextBox.h>
#include <gui/themes/DefaultTheme.h>

#include <iostream>
#include <map>
#include <memory>
#include <SFML/Graphics.hpp>

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

void connectDebugSignals(gui::Widget* widget, const std::string& name) {
    widgetNames.emplace(widget, name);
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

void createButtonDemo(gui::Gui& myGui, const gui::Theme& theme) {
    auto greetButton = gui::Button::create(theme);
    connectDebugSignals(greetButton.get(), "greetButton");
    greetButton->setLabel(sf::String("hello!"));
    greetButton->setRotation(18.0f);
    greetButton->setOrigin(greetButton->getSize() * 0.5f);
    greetButton->setPosition(160.0f, 130.0f);
    greetButton->setScale(1.6f, 2.5f);
    greetButton->onClick.connect([=]{ greetButton->setLabel("nice to meet you"); });
    myGui.addChild(greetButton);

    auto hideButton = gui::Button::create(theme);
    connectDebugSignals(hideButton.get(), "hideButton");
    hideButton->setLabel("click to hide");
    hideButton->setPosition(40.0f, 40.0f);
    hideButton->onClick.connect([=]{ hideButton->setVisible(false); });
    myGui.addChild(hideButton);

    auto disableButton = gui::Button::create(theme);
    connectDebugSignals(disableButton.get(), "disableButton");
    disableButton->setLabel("click to disable");
    disableButton->setPosition(200.0f, 40.0f);
    disableButton->onClick.connect([=]{ disableButton->setEnabled(false); });
    myGui.addChild(disableButton);

    auto resetButton = gui::Button::create(theme);
    connectDebugSignals(resetButton.get(), "resetButton");
    resetButton->setLabel("click to reset others");
    resetButton->setPosition(40.0f, 100.0f);
    resetButton->onClick.connect([=]{ hideButton->setVisible(true); disableButton->setEnabled(true); });
    myGui.addChild(resetButton);

    auto buttonA = gui::Button::create(theme);
    connectDebugSignals(buttonA.get(), "buttonA");
    buttonA->setLabel("A (front)");
    buttonA->setPosition(40.0f, 200.0f);
    buttonA->onClick.connect([=]{ buttonA->moveToFront(); });
    myGui.addChild(buttonA);

    auto buttonB = gui::Button::create(theme);
    connectDebugSignals(buttonB.get(), "buttonB");
    buttonB->setLabel("B (front)");
    buttonB->setPosition(80.0f, 200.0f);
    buttonB->onClick.connect([=]{ buttonB->moveToFront(); });
    myGui.addChild(buttonB);

    auto buttonC = gui::Button::create(theme);
    connectDebugSignals(buttonC.get(), "buttonC");
    buttonC->setLabel("C (back)");
    buttonC->setPosition(60.0f, 215.0f);
    buttonC->onClick.connect([=]{ buttonC->moveToBack(); });
    myGui.addChild(buttonC);


    // FIXME add some more for style stuff?
}

int main() {
    sf::RenderWindow window(sf::VideoMode(400, 300), "GUI Test");

    gui::Gui myGui(window);

    gui::DefaultTheme theme;

    const std::array<std::string, 2> sceneNames = {
        "Empty",
        "ButtonDemo"
    };
    size_t currentScene = 0;
    bool sceneChanged = true;

    auto sceneButton = gui::Button::create(theme);
    connectDebugSignals(sceneButton.get(), "sceneButton");
    sceneButton->setPosition(0.0f, window.getSize().y - 20.0f);
    sceneButton->onMousePress.connect([&](gui::Widget* w, sf::Mouse::Button button, const sf::Vector2f& pos) {
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
            myGui.addChild(sceneButton);
            sceneChanged = false;

            std::cout << "Building scene " << sceneNames[currentScene] << "\n";
            if (currentScene == 1) {
                createButtonDemo(myGui, theme);
            }
        }

        window.clear(sf::Color(80, 80, 80));
        window.draw(myGui);
        window.display();
    }

    return 0;
}
