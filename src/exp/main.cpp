#include <Board.h>
#include <ChunkRender.h>
#include <DebugScreen.h>
#include <OffsetView.h>
#include <ResourceManager.h>
#include <Tile.h>
#include <tiles/Blank.h>
#include <tiles/Wire.h>

#include <cmath>
#include <iostream>
#include <SFML/Graphics.hpp>
#include <spdlog/spdlog.h>
#include <string>



#include <LegacyFileFormat.h>

int main() {
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("Using spdlog v{}.{}.{}", SPDLOG_VER_MAJOR, SPDLOG_VER_MINOR, SPDLOG_VER_PATCH);
    spdlog::info("Logging level set to {}.", spdlog::level::to_string_view(spdlog::get_level()));
    spdlog::info("Initializing RenderWindow.");
    sf::RenderWindow window(sf::VideoMode(800, 600), "Test", sf::Style::Default, sf::ContextSettings(0, 0, 4));
    //window.setVerticalSyncEnabled(true);

    ResourceManager resource;
    DebugScreen::init(resource.getFont("resources/consolas.ttf"), 16, window.getSize());
    DebugScreen::instance()->setVisible(true);

    constexpr unsigned int TILE_WIDTH = 32;
    Board::setupTextures(resource, "resources/texturePackGrid.png", "resources/texturePackNoGrid.png", TILE_WIDTH);
    Board board;
    board.debugSetDrawChunkBorder(true);



    std::unique_ptr<FileStorage> fileStorage(new LegacyFileFormat());
    fileStorage->saveToFile(board);



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
    board.accessTile(-20, -6).setType(tiles::Wire::instance(), TileId::wireCrossover, Direction::north, State::high, State::middle);

    board.debugPrintChunk(0x8000000080000000);

    sf::View fullWindowView(window.getDefaultView());
    OffsetView boardView(TILE_WIDTH * Chunk::WIDTH, window.getDefaultView());
    float zoomLevel = 1.0f;
    sf::Vector2i lastMousePos;
    sf::Clock frameTimer;


    // FIXME
    // there's some perf issues with fullscreen at max zoom, too much indexing into FlatMap?
    //     Maybe we should run a periodic clean up to trim RenderBlocks that are far enough off screen, for all LODs.
    //     Will need chunk load/unload process working in order to test this.


    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            DebugScreen::instance()->processEvent(event);

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
                constexpr float maxZoom = 31.0f;
                static_assert(maxZoom < (1 << ChunkRender::LEVELS_OF_DETAIL), "Maximum zoom level must not exceed the total levels of detail.");

                zoomLevel += zoomDelta;
                zoomLevel = std::min(std::max(zoomLevel, 0.2f), maxZoom);
                //if (zoomLevel + zoomDelta > 0.2f && zoomLevel + zoomDelta < maxZoom) {
                    //zoomLevel += zoomDelta;
                    boardView.setSize(window.getSize().x * zoomLevel, window.getSize().y * zoomLevel);
                //}
            } else if (event.type == sf::Event::Resized) {
                fullWindowView.reset({0.0f, 0.0f, static_cast<float>(event.size.width), static_cast<float>(event.size.height)});
                boardView.setSize(event.size.width * zoomLevel, event.size.height * zoomLevel);
            } else if (event.type == sf::Event::Closed) {
                window.close();
            }
        }
        DebugScreen::instance()->profilerEvent("main process_events_done");

        DebugScreen::instance()->profilerEvent("main update_debug");
        DebugScreen::instance()->getField(DebugScreen::Field::frameTime).setString(
            fmt::format("Mode: {} (use arrow keys to change), Frame: {}ms", DebugScreen::instance()->getModeString(), frameTimer.restart().asMilliseconds())
        );
        sf::Vector2f boardViewApproxCenter(boardView.getCenter() + sf::Vector2f(boardView.getCenterOffset()) * boardView.getViewDivisor());
        DebugScreen::instance()->getField(DebugScreen::Field::view).setString(
            fmt::format("View: {:.2f} by {:.2f} at ({:.2f}, {:.2f})", boardView.getSize().x, boardView.getSize().y, boardViewApproxCenter.x, boardViewApproxCenter.y)
        );
        DebugScreen::instance()->getField(DebugScreen::Field::zoom).setString(
            fmt::format("Zoom: {}", zoomLevel)
        );
        DebugScreen::instance()->getField(DebugScreen::Field::chunk).setString(
            fmt::format("Chunk: {} visible", board.debugGetChunksDrawn())
        );

        DebugScreen::instance()->profilerEvent("main draw");
        window.clear();

        board.debugSetDrawChunkBorder(DebugScreen::instance()->isVisible());

        window.setView(boardView.getView());
        board.setRenderArea(boardView, zoomLevel);
        //sf::View bc = boardView;
        //bc.setSize(bc.getSize() * 2.0f);
        //window.setView(bc);
        window.draw(board);

        window.setView(fullWindowView);
        window.draw(*DebugScreen::instance());

        DebugScreen::instance()->profilerEvent("main display");
        window.display();
        DebugScreen::instance()->profilerEvent("main finish_loop");
    }

    return 0;
}
