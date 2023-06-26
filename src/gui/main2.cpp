#include <gui/Button.h>
#include <gui/Gui.h>
#include <gui/Panel.h>
#include <gui/TextBox.h>
#include <gui/themes/DefaultTheme.h>

#include <array>
#include <cassert>
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

int main() {
    sf::RenderWindow window(sf::VideoMode(400, 300), "GUI Test");

    gui::Gui myGui(window);

    gui::DefaultTheme theme(myGui);

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
            widgetNames.clear();
            assert(widgetNames.emplace(sceneButton.get(), "sceneButton").second);
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
