#include "gui/Signal.h"
#include <gui/Button.h>
#include <gui/Gui.h>
#include <gui/themes/DefaultTheme.h>

#include <iostream>
#include <SFML/Graphics.hpp>

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
    auto ret = button->onPress.connect(&func);
    std::cout << "connect gave " << ret << "\n";
    //ret = button->onPress.connect(&func2);
    //std::cout << "connect gave " << ret << "\n";
    myGui.addChild(button);

    gui::Callback<int> cb(&func);
    gui::Callback<int> cb2(&func2);

    gui::Callback<int> cb3 = cb;
    cb3 = cb;
    cb = cb3;
    //cb3 = gui::Callback<int>(&func);

    std::unordered_map<unsigned int, gui::Callback<int>> stuff;
    stuff.emplace(1, &func);
    stuff.emplace(2, &func2);
    stuff.emplace(3, gui::Callback<int>(&func));

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
