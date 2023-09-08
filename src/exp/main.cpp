#include <Board.h>
#include <Tile.h>
#include <tiles/Blank.h>
#include <tiles/Wire.h>

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

    auto tile = board.accessTile(0, 0);
    std::cout << "attempt to getHighlight()\n";
    tile.setHighlight(true);
    std::cout << tile.getHighlight() << "\n";
    tile.setType(tiles::Wire::instance(), TileId::wireCrossover, Direction::north, State::high, State::middle);
    std::cout << "dir=" << static_cast<int>(tile.getDirection()) << ", state=" << static_cast<int>(tile.getState()) << "\n";

    board.debugPrintChunk(0);

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
