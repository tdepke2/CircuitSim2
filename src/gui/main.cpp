#include <gui/Button.h>
#include <gui/Gui.h>
#include <gui/Panel.h>
#include <gui/Signal.h>
#include <gui/themes/DefaultTheme.h>

#include <iostream>
#include <SFML/Graphics.hpp>
#include <unordered_map>

void func() {
    std::cout << "func called\n";
}
void func2(int n) {
    std::cout << "func2 called with " << n << "\n";
}

int main() {
    std::cout << "Starting gui demo\n";
    sf::RenderWindow window(sf::VideoMode(400, 300), "GUI Demo");

    sf::CircleShape shape(100.0f);
    shape.setFillColor(sf::Color::Green);

    gui::Gui myGui;

    // FIXME: The lifetime of the theme is bound to the gui (due to font and stuff), make it stored in the gui? maybe not...
    // perhaps instead make it not a pointer (and pass ref to widget ctors) to make it clear.
    auto theme = gui::DefaultTheme::create();

    auto button = gui::Button::create(theme);
    button->setLabel(sf::String("hello!"));
    //button->setSize(sf::Vector2f(40, 30));
    auto ret = button->onClick.connect(&func);
    std::cout << "connect gave " << ret << "\n";
    //ret = button->onClick.connect(&func2);
    //std::cout << "connect gave " << ret << "\n";
    ret = button->onClick.connect([&]{ window.close(); });
    std::cout << "connect gave " << ret << "\n";
    button->onClick.disconnect(ret);
    ret = button->onClick.connect([&]{ button->setLabel("nice to meet you"); });
    std::cout << "connect gave " << ret << "\n";
    myGui.addChild(button);

    auto panel = gui::Panel::create(theme);
    panel->setSize(sf::Vector2f(200, 150));
    panel->setPosition(50.0f, 50.0f);
    myGui.addChild(panel);

    auto button2 = gui::Button::create(theme);
    button2->setPosition(5.0f, 5.0f);
    button2->setLabel(sf::String("click me"));
    button2->onClick.connect([&]{ std::cout << "button2 clicked\n"; button2->setLabel(button2->getLabel() + "?"); });
    panel->addChild(button2);

    // Value ctors
    gui::Callback<int> cb(&func);
    gui::Callback<int> cb2(&func2);
    // Copy ctor
    gui::Callback<int> cb3 = cb;
    gui::Callback<int> cb4 = cb2;
    // Move ctor
    gui::Callback<int> cb5 = gui::Callback<int>(&func);
    gui::Callback<int> cb6 = gui::Callback<int>(&func2);
    // Swap function
    swap(cb5, cb6);
    swap(cb, cb6);
    swap(cb2, cb5);
    // Copy assignment
    cb3 = cb5;
    cb4 = cb6;
    // Move assignment
    cb3 = gui::Callback<int>(&func);
    cb4 = gui::Callback<int>(&func2);
    // Test with unordered_map and dtor
    std::unordered_map<unsigned int, gui::Callback<int>> stuff;
    stuff.emplace(1, &func);
    stuff.emplace(2, &func2);
    stuff.emplace(3, gui::Callback<int>(&func));
    stuff.clear();
    cb3.value<gui::Callback<int>::Tag::noArgs>()();
    cb4.value<gui::Callback<int>::Tag::withArgs>()(123);
    // Throws runtime error as intended
    //cb3.value<gui::Callback<int>::Tag::withArgs>()(123);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            myGui.handleEvent(event);
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        window.clear();
        window.draw(shape);
        window.draw(myGui);
        window.display();
    }

    return 0;
}
