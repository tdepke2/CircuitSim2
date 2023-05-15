#include <gui/Button.h>
#include <gui/Gui.h>
#include <gui/themes/DefaultTheme.h>

#include <iostream>
#include <SFML/Graphics.hpp>

int main() {
    std::cout << "Starting gui demo\n";
    sf::RenderWindow window(sf::VideoMode(800, 600), "GUI Demo");

    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);

    gui::Gui myGui;
    auto theme = gui::DefaultTheme::create();
    auto button = gui::Button::create(theme);
    myGui.addChild(button);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
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
