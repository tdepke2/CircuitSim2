#include <gui/Button.h>
#include <gui/Gui.h>

#include <SFML/Graphics.hpp>
#include <iostream>

int main() {
    std::cout << "Starting gui demo\n";
    sf::RenderWindow window(sf::VideoMode(800, 600), "GUI Demo");

    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);

    gui::Gui myGui;
    auto button = gui::Button::create();
    myGui.addWidget(button);

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
