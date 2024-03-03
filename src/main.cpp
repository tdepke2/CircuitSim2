#include <Board.h>
#include <DebugScreen.h>
#include <Editor.h>
#include <entities/Label.h>
#include <Locator.h>
#include <OffsetView.h>
#include <ResourceManager.h>
#include <Tile.h>
#include <tiles/Blank.h>
#include <tiles/Input.h>
#include <tiles/Label.h>
#include <tiles/Wire.h>

#include <cmath>
#include <SFML/Graphics.hpp>
#include <spdlog/spdlog.h>
#include <string>





#include <TilePool.h>
#include <commands/WriteTiles.h>






int main() {
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("Using spdlog v{}.{}.{}", SPDLOG_VER_MAJOR, SPDLOG_VER_MINOR, SPDLOG_VER_PATCH);
    spdlog::info("Logging level set to {}.", spdlog::level::to_string_view(spdlog::get_level()));
    spdlog::info("Initializing RenderWindow.");
    sf::RenderWindow window(sf::VideoMode(800, 600), "Test", sf::Style::Default, sf::ContextSettings(0, 0, 4));
    //window.setVerticalSyncEnabled(true);

    Locator::provide(std::unique_ptr<ResourceManager>(new ResourceManager()));

    DebugScreen::init(Locator::getResource()->getFont("resources/consolas.ttf"), 16, window.getSize());
    DebugScreen::instance()->setVisible(true);

    Board board;
    board.debugSetDrawChunkBorder(true);

    try {
        board.loadFromFile("boards/NewBoard/board.txt");
        //board.loadFromFile("boards/Calculator.txt");//"boards/AllTexStates.txt");//"boards/components/Add3Module.txt");

    } catch (std::exception& ex) {
        spdlog::error(ex.what());
    }

    Editor editor(board, window.getDefaultView());

    auto tile = board.accessTile(0, 0);
    tile.setHighlight(true);
    //tile.setType(tiles::Wire::instance(), TileId::wireCrossover, Direction::north, State::high, State::middle);
    //std::cout << "dir=" << static_cast<int>(tile.getDirection()) << ", state=" << static_cast<int>(tile.getState()) << "\n";
    board.accessTile(2, 2).setHighlight(true);
    board.accessTile(-1, -1).setHighlight(true);
    board.accessTile(-20, -6).setType(tiles::Wire::instance(), TileId::wireCrossover, Direction::north, State::high, State::middle);

    auto labelTile = board.accessTile(-8, -8);
    labelTile.setType(tiles::Label::instance());
    labelTile.call<tiles::Label>(&tiles::Label::modifyEntity)->setString("hello!");

    /*board.accessTile(-2, -2).setType(tiles::Input::instance(), TileId::inButton, State::high, 'G');

    TilePool testPool;
    commands::WriteTiles writeTest(board, testPool);
    auto tile2 = writeTest.pushBackTile({-8, -8});
    //tile2.setType(tiles::Wire::instance(), TileId::wireJunction, Direction::north, State::high);
    tile2.setType(tiles::Label::instance());
    tile2.call<tiles::Label>(&tiles::Label::modifyEntity)->setString("this is my label");
    spdlog::debug("writeTest created, executing...");
    writeTest.execute();
    //writeTest.execute();*/

    //spdlog::debug("tile is {}", static_cast<int>(board.accessTile(-2, -2).getId()));
    //board.accessTile(-2, -2).call<tiles::Label>(&tiles::Label::modifyEntity)->setString("this is my label 2");

    /*
    //board.accessTile(1, 1).setType(tiles::Wire::instance(), TileId::wireCorner, Direction::west, State::middle);
    auto tile3 = board.accessTile(1, 1);
    tile3.setType(tiles::Label::instance());
    tile3.call<tiles::Label>(&tiles::Label::modifyEntity)->setString("this is my label");
    //board.accessTile(2, 2).setType(tiles::Input::instance(), TileId::inButton, State::high, 'G');
    auto tile4 = board.accessTile(2, 2);
    tile4.setType(tiles::Label::instance());
    tile4.call<tiles::Label>(&tiles::Label::modifyEntity)->setString("hey it's another one");
    spdlog::debug("swapping tiles...");
    board.accessTile(1, 1).swapWith(board.accessTile(2, 2));
    //board.accessTile(1, 1).swapWith(board.accessTile(2, 2));
    */

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

    Locator::provide(std::unique_ptr<ResourceManager>(nullptr));

    return 0;
}
