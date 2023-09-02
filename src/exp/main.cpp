#include <Board.h>

#include <iostream>
#include <SFML/Graphics.hpp>

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Test");

    sf::Texture tilesetGrid;
    if (!tilesetGrid.loadFromFile("resources/texturePackGrid.png")) {
        std::cerr << "Failed to load texture file.\n";
        return -1;
    }
    Board board(&tilesetGrid);

    try {
        board.loadFromFile("boards/components/Add3Module.txt");
    } catch (std::exception& ex) {
        std::cerr << ex.what() << "\n";
    }

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        window.clear();
        window.draw(board);
        window.display();
    }

    return 0;
}
