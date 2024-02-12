#include <Board.h>
#include <DebugScreen.h>
#include <Editor.h>
#include <OffsetView.h>
#include <ResourceManager.h>
#include <Tile.h>
#include <TilePool.h>
#include <tiles/Blank.h>
#include <tiles/Input.h>
#include <tiles/Wire.h>

#include <cmath>
#include <SFML/Graphics.hpp>
#include <spdlog/spdlog.h>
#include <string>

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

    TilePool pool;
    //pool.accessTile(0);
    auto result = pool.allocateTile();
    spdlog::info("Allocate 1: tile {}, id {}", static_cast<int>(result.first.getId()), result.second);
    result.first.setType(tiles::Wire::instance());
    spdlog::info("Tile 1 (after change): {}", static_cast<int>(pool.accessTile(result.second).getId()));

    auto result2 = pool.allocateTile();
    spdlog::info("Allocate 2: tile {}, id {}", static_cast<int>(result2.first.getId()), result2.second);
    result2.first.setType(tiles::Input::instance());
    spdlog::info("Tile 2 (after change): {}", static_cast<int>(pool.accessTile(result2.second).getId()));

    pool.freeTile(result.second);
    spdlog::info("Free 1: done");
    //spdlog::info("Tile 1: {}", static_cast<int>(pool.accessTile(result.second).getId()));
    spdlog::info("Tile 2: {}", static_cast<int>(pool.accessTile(result2.second).getId()));

    pool.freeTile(result2.second);
    spdlog::info("Free 2: done");

    auto result3 = pool.allocateTile();
    spdlog::info("Allocate 3: tile {}, id {}", static_cast<int>(result3.first.getId()), result3.second);
    result3.first.setType(tiles::Input::instance(), TileId::inButton);
    spdlog::info("Tile 3 (after change): {}", static_cast<int>(pool.accessTile(result3.second).getId()));

    spdlog::info("Allocating to fill chunk area...");
    for (int i = 3; i < 1024; ++i) {
        pool.allocateTile();
    }

    auto result1024 = pool.allocateTile();
    spdlog::info("Allocate 1024: tile {}, id {}", static_cast<int>(result1024.first.getId()), result1024.second);
    result1024.first.setType(tiles::Wire::instance(), TileId::wireCrossover);
    spdlog::info("Tile 1024 (after change): {}", static_cast<int>(pool.accessTile(result1024.second).getId()));

    pool.debugPrintInfo();

    spdlog::info("Freeing to clear chunk area...");
    for (int i = 2; i < 1024; ++i) {
        pool.freeTile(i);
    }

    auto result1025 = pool.allocateTile();
    spdlog::info("Allocate 1025: tile {}, id {}", static_cast<int>(result1025.first.getId()), result1025.second);
    result1025.first.setType(tiles::Wire::instance(), TileId::wireJunction);
    spdlog::info("Tile 1025 (after change): {}", static_cast<int>(pool.accessTile(result1025.second).getId()));

    pool.debugPrintInfo();


    // FIXME: need to test null items in the middle, and removing multiple nulls at once.
    // what if we are near boundary with little to no tiles allocated, and we allocate/free?


    return 0;

    try {
        board.loadFromFile("boards/NewBoard/board.txt");
        //board.loadFromFile("boards/Calculator.txt");//"boards/AllTexStates.txt");//"boards/components/Add3Module.txt");

    } catch (std::exception& ex) {
        spdlog::error(ex.what());
    }

    Editor editor(board, resource, window.getDefaultView());

    auto tile = board.accessTile(0, 0);
    tile.setHighlight(true);
    //tile.setType(tiles::Wire::instance(), TileId::wireCrossover, Direction::north, State::high, State::middle);
    //std::cout << "dir=" << static_cast<int>(tile.getDirection()) << ", state=" << static_cast<int>(tile.getState()) << "\n";
    board.accessTile(2, 2).setHighlight(true);
    board.accessTile(-1, -1).setHighlight(true);
    board.accessTile(-20, -6).setType(tiles::Wire::instance(), TileId::wireCrossover, Direction::north, State::high, State::middle);

    board.debugPrintChunk(0x8000000080000000);

    sf::View fullWindowView(window.getDefaultView());
    //OffsetView boardView(TILE_WIDTH * Chunk::WIDTH, window.getDefaultView());
    //float zoomLevel = 1.0f;
    //sf::Vector2i lastMousePos;
    sf::Clock frameTimer;


    // FIXME
    // there's some perf issues with fullscreen at max zoom, too much indexing into FlatMap?
    //     Maybe we should run a periodic clean up to trim RenderBlocks that are far enough off screen, for all LODs.
    //     Will need chunk load/unload process working in order to test this.


    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            DebugScreen::instance()->processEvent(event);
            editor.processEvent(event);

            if (event.type == sf::Event::MouseMoved) {
                /*if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                    sf::Vector2f newCenter(
                        boardView.getCenter().x + (lastMousePos.x - event.mouseMove.x) * zoomLevel,
                        boardView.getCenter().y + (lastMousePos.y - event.mouseMove.y) * zoomLevel
                    );
                    boardView.setCenter(newCenter);
                }
                lastMousePos.x = event.mouseMove.x;
                lastMousePos.y = event.mouseMove.y;*/
            } else if (event.type == sf::Event::MouseWheelScrolled) {
                /*float zoomMult = 1.0f + (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::RShift)) * 5.0f;
                float zoomDelta = event.mouseWheelScroll.delta * zoomMult * zoomLevel * -0.04f;
                constexpr float maxZoom = 31.0f;
                static_assert(maxZoom < (1 << LodRenderer::LEVELS_OF_DETAIL), "Maximum zoom level must not exceed the total levels of detail.");

                zoomLevel += zoomDelta;
                zoomLevel = std::min(std::max(zoomLevel, 0.2f), maxZoom);
                boardView.setSize(window.getSize().x * zoomLevel, window.getSize().y * zoomLevel);*/
            } else if (event.type == sf::Event::KeyPressed) {
                /*if (event.key.control) {
                    if (event.key.code == sf::Keyboard::S) {
                        board.saveToFile();
                    }
                }*/
            } else if (event.type == sf::Event::Resized) {
                fullWindowView.reset({0.0f, 0.0f, static_cast<float>(event.size.width), static_cast<float>(event.size.height)});
                //boardView.setSize(event.size.width * zoomLevel, event.size.height * zoomLevel);
            } else if (event.type == sf::Event::Closed) {
                window.close();
            }
        }
        DebugScreen::instance()->profilerEvent("main process_events_done");

        editor.update();

        /*static auto state = State::low;
        //spdlog::debug("Toggling tiles to state {}", static_cast<int>(state));
        for (int i = 28; i < 36; ++i) {
            board.accessTile(31, i).setType(tiles::Input::instance(), TileId::inSwitch, state, 's');
        }
        for (int i = 28; i < 36; ++i) {
            board.accessTile(32, i).setType(tiles::Input::instance(), TileId::inSwitch, state, 's');
        }
        state = (state == State::low ? State::high : State::low);*/

        DebugScreen::instance()->profilerEvent("main update_debug");
        DebugScreen::instance()->getField(DebugScreen::Field::frameTime).setString(
            fmt::format("Mode: {} (use arrow keys to change), Frame: {}ms", DebugScreen::instance()->getModeString(), frameTimer.restart().asMilliseconds())
        );
        sf::Vector2f boardViewApproxCenter(editor.getEditView().getCenter() + sf::Vector2f(editor.getEditView().getCenterOffset()) * editor.getEditView().getViewDivisor());
        DebugScreen::instance()->getField(DebugScreen::Field::view).setString(
            fmt::format("View: {:.2f} by {:.2f} at ({:.2f}, {:.2f})", editor.getEditView().getSize().x, editor.getEditView().getSize().y, boardViewApproxCenter.x, boardViewApproxCenter.y)
        );
        DebugScreen::instance()->getField(DebugScreen::Field::zoom).setString(
            fmt::format("Zoom: {}", editor.getZoom())
        );
        DebugScreen::instance()->getField(DebugScreen::Field::chunk).setString(
            fmt::format("Chunk: {} visible", board.debugGetChunksDrawn())
        );

        DebugScreen::instance()->profilerEvent("main draw");
        window.clear({20, 20, 20, 255});

        board.debugSetDrawChunkBorder(DebugScreen::instance()->isVisible());

        window.setView(editor.getEditView().getView());
        board.setRenderArea(editor.getEditView(), editor.getZoom());
        /*
        // Debug to show the area outside the view.
        sf::View halfView = editor.getEditView().getView();
        halfView.setSize(halfView.getSize() * 2.0f);
        sf::Transform halfViewTrans = editor.getEditView().getView().getInverseTransform() * halfView.getTransform();
        */
        window.draw(board);//, halfViewTrans);
        window.draw(editor);//, halfViewTrans);

        window.setView(fullWindowView);
        window.draw(*DebugScreen::instance());

        DebugScreen::instance()->profilerEvent("main display");
        window.display();
        DebugScreen::instance()->profilerEvent("main finish_loop");
    }

    return 0;
}
