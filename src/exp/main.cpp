#include <Board.h>
#include <Tile.h>
#include <tiles/Blank.h>
#include <tiles/Wire.h>

#include <iostream>
#include <SFML/Graphics.hpp>
#include <string>

void clampToEdge(sf::Image& image, const sf::Vector2u& topLeft, const sf::Vector2u& bottomRight, const sf::Vector2u& borderSize) {
    sf::Vector2u borderTopLeft(topLeft - borderSize), borderBottomRight(bottomRight + borderSize);
    for (unsigned int y = borderTopLeft.y; y < borderBottomRight.y; ++y) {
        for (unsigned int x = borderTopLeft.x; x < borderBottomRight.x; ++x) {
            if (x < topLeft.x || x >= bottomRight.x || y < topLeft.y || y >= bottomRight.y) {
                unsigned int xTarget = std::min(std::max(x, topLeft.x), bottomRight.x - 1);
                unsigned int yTarget = std::min(std::max(y, topLeft.y), bottomRight.y - 1);
                image.setPixel(x, y, image.getPixel(xTarget, yTarget));
            }
        }
    }
}

sf::Texture loadTileset(const std::string& filename, unsigned int tileWidth) {
    sf::Image tileset;
    if (!tileset.loadFromFile(filename)) {
        std::cerr << "Failed to load texture file.\n";
        exit(-1);
    }
    sf::Texture result;
    result.create(tileset.getSize().x * 2, tileset.getSize().y * 4);
    sf::Image fullTileset;
    fullTileset.create(tileset.getSize().x * 2, tileset.getSize().y * 2, sf::Color::Red);

    for (unsigned int y = 0; y < tileset.getSize().y; y += tileWidth) {
        for (unsigned int x = 0; x < tileset.getSize().x; x += tileWidth) {
            sf::Vector2u tileTopLeft(x * 2 + tileWidth / 2, y * 2 + tileWidth / 2);
            fullTileset.copy(tileset, tileTopLeft.x, tileTopLeft.y, sf::IntRect(x, y, tileWidth, tileWidth));
            clampToEdge(fullTileset, tileTopLeft, tileTopLeft + sf::Vector2u(tileWidth, tileWidth), sf::Vector2u(tileWidth / 2, tileWidth / 2));
        }
    }
    result.update(fullTileset, 0, 0);

    for (unsigned int y = 0; y < fullTileset.getSize().y; ++y) {
        for (unsigned int x = 0; x < fullTileset.getSize().x; ++x) {
            fullTileset.setPixel(x, y, fullTileset.getPixel(x, y) + sf::Color(100, 100, 100));
        }
    }
    result.update(fullTileset, 0, tileset.getSize().y * 2);
    std::cout << "Built tileset texture with size " << result.getSize().x << " x " << result.getSize().y << "\n";
    return result;
}

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Test", sf::Style::Default, sf::ContextSettings(0, 0, 4));

    sf::Texture tilesetGrid = loadTileset("resources/texturePackNoGrid.png", 32);

    tilesetGrid.setSmooth(true);
    if (!tilesetGrid.generateMipmap()) {
        std::cerr << "Warn: \"resources/texturePackGrid.png\": Unable to generate mipmap for texture.\n";
    }

    Chunk::setupTextureData(tilesetGrid.getSize(), 32);
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
    board.accessTile(2, 2).setHighlight(true);

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
