#include <Board.h>
#include <ChunkRender.h>
#include <DebugScreen.h>
#include <ResourceManager.h>
#include <Tile.h>
#include <tiles/Blank.h>
#include <tiles/Wire.h>

#include <iostream>
#include <SFML/Graphics.hpp>
#include <spdlog/spdlog.h>
#include <string>

int main() {
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("Using spdlog v{}.{}.{}", SPDLOG_VER_MAJOR, SPDLOG_VER_MINOR, SPDLOG_VER_PATCH);
    spdlog::info("Logging level set to {}.", spdlog::level::to_string_view(spdlog::get_level()));
    spdlog::info("Initializing RenderWindow.");
    sf::RenderWindow window(sf::VideoMode(800, 600), "Test", sf::Style::Default, sf::ContextSettings(0, 0, 4));
    window.setVerticalSyncEnabled(true);

    ResourceManager resource;
    DebugScreen debugScreen(resource.getFont("resources/consolas.ttf"), 16);
    debugScreen.setVisible(true);

    Board::setupTextures(resource, "resources/texturePackGrid.png", "resources/texturePackNoGrid.png", 32);
    Board board;
    board.debugSetDrawChunkBorder(true);

    try {
        board.loadFromFile("boards/ComputerGame.txt");//"boards/AllTexStates.txt");//"boards/components/Add3Module.txt");

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
    board.debugRedrawChunks();

    sf::View fullWindowView(window.getDefaultView());
    sf::View boardView(window.getDefaultView());
    float zoomLevel = 1.0f;
    sf::Vector2i lastMousePos;
    sf::Clock frameTimer;

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
                constexpr float maxZoom = 20.0f;
                static_assert(maxZoom < (1 << ChunkRender::LEVELS_OF_DETAIL), "Maximum zoom level must not exceed the total levels of detail.");

                if (zoomLevel + zoomDelta > 0.2f && zoomLevel + zoomDelta < maxZoom) {
                    zoomLevel += zoomDelta;
                    boardView.setSize(window.getSize().x * zoomLevel, window.getSize().y * zoomLevel);
                }
            } else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::F3) {
                    debugScreen.setVisible(!debugScreen.isVisible());
                    board.debugSetDrawChunkBorder(debugScreen.isVisible());
                }
            } else if (event.type == sf::Event::Resized) {
                fullWindowView.reset({0.0f, 0.0f, static_cast<float>(event.size.width), static_cast<float>(event.size.height)});
                boardView.setSize(event.size.width * zoomLevel, event.size.height * zoomLevel);
            } else if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        debugScreen.getField(DebugScreen::Field::frameTime).setString(
            fmt::format("Frame: {}ms", frameTimer.restart().asMilliseconds())
        );
        debugScreen.getField(DebugScreen::Field::view).setString(
            fmt::format("View: {:.2f} by {:.2f} at ({:.2f}, {:.2f})", boardView.getSize().x, boardView.getSize().y, boardView.getCenter().x, boardView.getCenter().y)
        );
        debugScreen.getField(DebugScreen::Field::zoom).setString(
            fmt::format("Zoom: {}", zoomLevel)
        );
        debugScreen.getField(DebugScreen::Field::chunk).setString(
            fmt::format("Chunk: {} visible", board.debugGetChunksDrawn())
        );

        window.clear();

        window.setView(boardView);
        board.setRenderArea(boardView, zoomLevel, debugScreen);
        board.updateRender();
        window.draw(board);

        window.setView(fullWindowView);
        window.draw(debugScreen);

        window.display();
    }

    return 0;
}
