#include <Board.h>
#include <Tile.h>
#include <tiles/Blank.h>
#include <tiles/Wire.h>

#include <iostream>
#include <SFML/Graphics.hpp>

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Test", sf::Style::Default, sf::ContextSettings(0, 0, 4));

    sf::Texture tilesetGrid;
    if (!tilesetGrid.loadFromFile("resources/texturePackGrid.png")) {
        std::cerr << "Failed to load texture file.\n";
        return -1;
    }
    tilesetGrid.setSmooth(true);
    if (!tilesetGrid.generateMipmap()) {
        std::cerr << "Warn: \"resources/texturePackGrid.png\": Unable to generate mipmap for texture.\n";
    }
    // FIXME this fixed the issue with weird pixel alignment, but now the lack of texture borders is noticeable



    Chunk::setupTextureData(tilesetGrid.getSize().x, 32);
    Board board(&tilesetGrid);

    try {
        board.loadFromFile("boards/AllTexStates.txt");//"boards/components/Add3Module.txt");

    } catch (std::exception& ex) {
        std::cerr << ex.what() << "\n";
    }

    auto tile = board.accessTile(0, 0);
    std::cout << "attempt to getHighlight()\n";
    tile.setHighlight(true);
    std::cout << tile.getHighlight() << "\n";
    //tile.setType(tiles::Wire::instance(), TileId::wireCrossover, Direction::north, State::high, State::middle);
    //std::cout << "dir=" << static_cast<int>(tile.getDirection()) << ", state=" << static_cast<int>(tile.getState()) << "\n";

    board.debugPrintChunk(0);
    board.debugRedrawChunk(0);

    sf::View fullWindowView(window.getDefaultView());
    sf::View boardView(window.getDefaultView());
    float zoomLevel = 1.0f;
    sf::Vector2i lastMousePos;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::MouseMoved) {
                if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                    sf::Vector2f newCenter(
                        boardView.getCenter().x + (lastMousePos.x - event.mouseMove.x) * zoomLevel,
                        boardView.getCenter().y + (lastMousePos.y - event.mouseMove.y) * zoomLevel
                    );
                    boardView.setCenter(newCenter);
                }
                lastMousePos.x = event.mouseMove.x;
                lastMousePos.y = event.mouseMove.y;
            } else if (event.type == sf::Event::MouseWheelScrolled) {
                float zoomMult = 1.0f + (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::RShift)) * 5.0f;
                float zoomDelta = event.mouseWheelScroll.delta * zoomMult * zoomLevel * -0.04f;
                if (zoomLevel + zoomDelta > 0.2f && zoomLevel + zoomDelta < 20.0f) {
                    zoomLevel += zoomDelta;
                    boardView.setSize(window.getSize().x * zoomLevel, window.getSize().y * zoomLevel);
                }
            } else if (event.type == sf::Event::Resized) {
                fullWindowView.reset({0.0f, 0.0f, static_cast<float>(event.size.width), static_cast<float>(event.size.height)});
                boardView.setSize(event.size.width * zoomLevel, event.size.height * zoomLevel);
            } else if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        window.clear();
        window.setView(boardView);
        window.draw(board);
        window.display();
    }

    return 0;
}
