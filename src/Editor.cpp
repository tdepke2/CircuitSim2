#include <Board.h>
#include <Chunk.h>
#include <Command.h>
#include <commands/FillArea.h>
#include <commands/PlaceTiles.h>
#include <DebugScreen.h>
#include <Editor.h>
#include <Locator.h>
#include <LodRenderer.h>
#include <ResourceBase.h>
#include <Tile.h>
#include <tiles/Blank.h>
#include <tiles/Gate.h>
#include <tiles/Input.h>
#include <tiles/Label.h>
#include <tiles/Led.h>
#include <tiles/Wire.h>
#include <TileWidth.h>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <limits>
#include <spdlog/spdlog.h>
#include <string>

namespace {

constexpr int constLog2(int x) {
    return x == 1 ? 0 : 1 + constLog2(x / 2);
}

/**
 * Iterate over an area of tiles on the board, from `first` to `last` inclusive.
 * This is done per-chunk which is significantly faster than doing it per-tile.
 */
template<typename Func>
void forEachTile(Board& board, const sf::Vector2i& first, const sf::Vector2i& second, Func f) {
    constexpr int widthLog2 = constLog2(Chunk::WIDTH);
    for (int yChunk = (first.y >> widthLog2); yChunk <= (second.y >> widthLog2); ++yChunk) {
        for (int xChunk = (first.x >> widthLog2); xChunk <= (second.x >> widthLog2); ++xChunk) {
            auto& chunk = board.accessChunk(ChunkCoords::pack(xChunk, yChunk));
            for (int i = 0; i < Chunk::WIDTH * Chunk::WIDTH; ++i) {
                int x = i % Chunk::WIDTH + xChunk * Chunk::WIDTH;
                int y = i / Chunk::WIDTH + yChunk * Chunk::WIDTH;
                if (x >= first.x && x <= second.x && y >= first.y && y <= second.y) {
                    f(chunk, i, x, y);
                }
            }
        }
    }
}

}

Editor::StaticInit* Editor::staticInit_ = nullptr;

Editor::StaticInit::StaticInit() {
    spdlog::debug("Editor::StaticInit initializing.");
    ResourceBase* resource = Locator::getResource();
    const sf::Texture* tilesetGrid = &resource->getTexture("resources/texturePackGrid.png");

    sf::Image tilesetCopy = tilesetGrid->copyToImage();
    const auto highlightStartOffset = tilesetCopy.getSize().x * tilesetCopy.getSize().y * 2;

    tilesetBright = &resource->getTexture("tilesetBright", true);
    if (!tilesetBright->create(tilesetCopy.getSize().x, tilesetCopy.getSize().y / 2)) {
        spdlog::error("Failed to create tilesetBright texture (size {} by {}).", tilesetCopy.getSize().x, tilesetCopy.getSize().y / 2);
    }
    tilesetBright->update(tilesetCopy.getPixelsPtr() + highlightStartOffset, tilesetCopy.getSize().x, tilesetCopy.getSize().y / 2, 0, 0);
    tilesetBright->setSmooth(true);
    if (!tilesetBright->generateMipmap()) {
        spdlog::warn("\"tilesetBright\": Unable to generate mipmap for texture.");
    }
    DebugScreen::instance()->registerTexture("tilesetBright", tilesetBright);

    sf::Image transparentBlank;
    transparentBlank.create(TileWidth::TEXELS * 2, TileWidth::TEXELS * 2, {0, 0, 0, 0});
    tilesetCopy.copy(transparentBlank, 0, tilesetCopy.getSize().y / 2);

    tilesetBrightNoBlanks = &resource->getTexture("tilesetBrightNoBlanks", true);
    if (!tilesetBrightNoBlanks->create(tilesetCopy.getSize().x, tilesetCopy.getSize().y / 2)) {
        spdlog::error("Failed to create tilesetBrightNoBlanks texture (size {} by {}).", tilesetCopy.getSize().x, tilesetCopy.getSize().y / 2);
    }
    tilesetBrightNoBlanks->update(tilesetCopy.getPixelsPtr() + highlightStartOffset, tilesetCopy.getSize().x, tilesetCopy.getSize().y / 2, 0, 0);
    tilesetBrightNoBlanks->setSmooth(true);
    if (!tilesetBrightNoBlanks->generateMipmap()) {
        spdlog::warn("\"tilesetBrightNoBlanks\": Unable to generate mipmap for texture.");
    }
    DebugScreen::instance()->registerTexture("tilesetBrightNoBlanks", tilesetBrightNoBlanks);
}

Editor::Editor(Board& board, const sf::View& initialView) :
    board_(board),
    editView_(static_cast<float>(TileWidth::TEXELS * Chunk::WIDTH), initialView),
    zoomLevel_(1.0f),
    mousePos_(),
    mouseOnScreen_(false),
    windowSize_(initialView.getSize()),
    cursor_({static_cast<float>(TileWidth::TEXELS), static_cast<float>(TileWidth::TEXELS)}),
    cursorCoords_({sf::Vector2i(), false}),
    cursorLabel_("", Locator::getResource()->getFont("resources/consolas.ttf"), 22),
    cursorState_(CursorState::empty),
    cursorVisible_(false),
    selectionStart_({sf::Vector2i(), false}),
    selectionEnd_(),
    tileSubBoard_(),
    copySubBoard_(),
    tilePool_(),
    editHistory_(),
    lastEditSize_(0) {

    static StaticInit staticInit;
    staticInit_ = &staticInit;

    cursor_.setFillColor({255, 80, 255, 100});
    cursorLabel_.setOutlineColor(sf::Color::Black);
    cursorLabel_.setOutlineThickness(2.0f);
}

const OffsetView& Editor::getEditView() const {
    return editView_;
}

float Editor::getZoom() const {
    return zoomLevel_;
}

void Editor::goToTile(int x, int y) {
    spdlog::debug("Go to tile ({}, {}).", x, y);
    editView_ = findTileView(x, y);
}

void Editor::processEvent(const sf::Event& event) {
    if (event.type == sf::Event::MouseMoved) {
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            editView_.setCenter({
                editView_.getCenter().x + (mousePos_.x - event.mouseMove.x) * zoomLevel_,
                editView_.getCenter().y + (mousePos_.y - event.mouseMove.y) * zoomLevel_
            });
            editView_.clampToView(findTileView(board_.getTileLowerBound()), std::less<float>());
            editView_.clampToView(findTileView(board_.getTileUpperBound()), std::greater<float>());
        }
        mousePos_.x = event.mouseMove.x;
        mousePos_.y = event.mouseMove.y;
        updateCursor();
        if (mousePos_.x >= 0 && mousePos_.x < static_cast<int>(windowSize_.x) && mousePos_.y >= 0 && mousePos_.y < static_cast<int>(windowSize_.y)) {
            mouseOnScreen_ = true;
            cursorVisible_ = cursorCoords_.second;
        }
        if (sf::Mouse::isButtonPressed(sf::Mouse::Right)) {
            if (selectionStart_.second) {
                updateSelection(cursorCoords_.first);
            }
            if ((cursorState_ == CursorState::pickTile || cursorState_ == CursorState::pasteArea) && cursorCoords_.second) {
                pasteToBoard(cursorCoords_.first, true);
            }
        }
    } else if (event.type == sf::Event::MouseEntered) {
        mouseOnScreen_ = true;
        cursorVisible_ = cursorCoords_.second;
    } else if (event.type == sf::Event::MouseLeft) {
        mouseOnScreen_ = false;
        if (!sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            cursorVisible_ = false;
        }
    } else if (event.type == sf::Event::MouseButtonPressed) {
        const auto cursorCoords = mapMouseToNearestTile({event.mouseButton.x, event.mouseButton.y});
        if (event.mouseButton.button == sf::Mouse::Right) {
            if (cursorState_ == CursorState::pickTile || cursorState_ == CursorState::pasteArea) {
                if (cursorCoords.second) {
                    pasteToBoard(cursorCoords.first, false);
                }
            } else if (!selectionStart_.second) {
                if (!sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) && !sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)) {
                    board_.removeAllHighlights();
                }
                selectionStart_.first = cursorCoords.first;
                selectionStart_.second = true;
                selectionEnd_ = selectionStart_.first;
            }
        }
    } else if (event.type == sf::Event::MouseButtonReleased) {
        const auto cursorCoords = mapMouseToNearestTile({event.mouseButton.x, event.mouseButton.y});
        if (event.mouseButton.button == sf::Mouse::Left) {
            cursorVisible_ = mouseOnScreen_ && cursorCoords_.second;
        } else if (event.mouseButton.button == sf::Mouse::Right) {
            if (selectionStart_.second) {
                updateSelection(cursorCoords.first);
                selectionStart_.second = false;
            }
        }
    } else if (event.type == sf::Event::MouseWheelScrolled) {
        float zoomMult = 1.0f + (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::RShift)) * 5.0f;
        float zoomDelta = event.mouseWheelScroll.delta * zoomMult * zoomLevel_ * -0.04f;
        constexpr float minZoom = 0.2f, maxZoom = 32.0f;
        static_assert(maxZoom <= (1 << LodRenderer::LEVELS_OF_DETAIL), "Maximum zoom level must not exceed the total levels of detail.");

        zoomLevel_ += zoomDelta;
        zoomLevel_ = std::min(std::max(zoomLevel_, minZoom), maxZoom);
        editView_.setSize(windowSize_.x * zoomLevel_, windowSize_.y * zoomLevel_);
    } else if (event.type == sf::Event::KeyPressed) {
        handleKeyPress(event.key);
    } else if (event.type == sf::Event::Resized) {
        windowSize_.x = event.size.width;
        windowSize_.y = event.size.height;
        editView_.setSize(static_cast<sf::Vector2f>(windowSize_) * zoomLevel_);
    }
}

void Editor::update() {
    updateCursor();

    const sf::Texture* tileset = staticInit_->tilesetBright;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)) {
        tileset = staticInit_->tilesetBrightNoBlanks;
    }
    if (cursorState_ == CursorState::pickTile) {
        tileSubBoard_.setRenderArea(editView_, zoomLevel_, cursorCoords_.first);
        tileSubBoard_.drawChunks(tileset);
    } else if (cursorState_ == CursorState::pasteArea) {
        copySubBoard_.setRenderArea(editView_, zoomLevel_, cursorCoords_.first);
        copySubBoard_.drawChunks(tileset);
    }
}

void Editor::handleKeyPress(const sf::Event::KeyEvent& key) {
    // Most key bindings are listed in the gui dropdown menus.
    if (key.control) {
        if (key.code == sf::Keyboard::N) {
            //fileOption(0);
        } else if (key.code == sf::Keyboard::O) {
            //fileOption(1);
        } else if (key.code == sf::Keyboard::S) {
            //fileOption(2);
        } else if (key.code == sf::Keyboard::A) {
            selectAll();
        } else if (key.code == sf::Keyboard::Z) {
            undoEdit();
        } else if (key.code == sf::Keyboard::Y) {
            redoEdit();
        } else if (key.code == sf::Keyboard::X) {
            copyArea();
            deleteArea();
        } else if (key.code == sf::Keyboard::C) {
            copyArea();
        } else if (key.code == sf::Keyboard::V) {
            pasteArea();
        }
        return;
    }

    if (key.code == sf::Keyboard::Enter) {
        //viewOption(0);
    } else if (key.code == sf::Keyboard::Tab) {
        //runOption(!key.shift);
    } else if (key.code == sf::Keyboard::Escape) {
        deselectAll();
    } else if (key.code == sf::Keyboard::R) {
        rotateArea(!key.shift);
    } else if (key.code == sf::Keyboard::F) {
        flipArea(!key.shift);
    } else if (key.code == sf::Keyboard::E) {
        editTile(!key.shift);
    } else if (key.code == sf::Keyboard::Delete) {
        deleteArea();
    } else if (key.code == sf::Keyboard::W) {
        wireTool();
    } else if (key.code == sf::Keyboard::Q) {
        queryTool();
    } else if (key.code == sf::Keyboard::Space) {
        pickTile(TileId::blank);
    } else if (key.code == sf::Keyboard::T) {
        pickTile(!key.shift ? TileId::wireStraight : TileId::wireTee);
    } else if (key.code == sf::Keyboard::C) {
        pickTile(!key.shift ? TileId::wireCorner : TileId::wireCrossover);
    } else if (key.code == sf::Keyboard::J) {
        pickTile(TileId::wireJunction);
    } else if (key.code == sf::Keyboard::S) {
        pickTile(!key.shift ? TileId::inSwitch : TileId::inButton);
    } else if (key.code == sf::Keyboard::L) {
        pickTile(!key.shift ? TileId::outLed : TileId::label);
    } else if (key.code == sf::Keyboard::D) {
        pickTile(TileId::gateDiode);
    } else if (key.code == sf::Keyboard::B) {
        pickTile(!key.shift ? TileId::gateBuffer : TileId::gateNot);
    } else if (key.code == sf::Keyboard::A) {
        pickTile(!key.shift ? TileId::gateAnd : TileId::gateNand);
    } else if (key.code == sf::Keyboard::O) {
        pickTile(!key.shift ? TileId::gateOr : TileId::gateNor);
    } else if (key.code == sf::Keyboard::X) {
        pickTile(!key.shift ? TileId::gateXor : TileId::gateXnor);
    } else if (key.code == sf::Keyboard::Num1) {
        goToTile(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
    } else if (key.code == sf::Keyboard::Num2) {
        goToTile(0, std::numeric_limits<int>::max());
    } else if (key.code == sf::Keyboard::Num3) {
        goToTile(std::numeric_limits<int>::max(), std::numeric_limits<int>::max());
    } else if (key.code == sf::Keyboard::Num4) {
        goToTile(std::numeric_limits<int>::min(), 0);
    } else if (key.code == sf::Keyboard::Num5) {
        goToTile(0, 0);
    } else if (key.code == sf::Keyboard::Num6) {
        goToTile(std::numeric_limits<int>::max(), 0);
    } else if (key.code == sf::Keyboard::Num7) {
        goToTile(std::numeric_limits<int>::min(), std::numeric_limits<int>::min());
    } else if (key.code == sf::Keyboard::Num8) {
        goToTile(0, std::numeric_limits<int>::min());
    } else if (key.code == sf::Keyboard::Num9) {
        goToTile(std::numeric_limits<int>::max(), std::numeric_limits<int>::min());
    }
}

void Editor::undoEdit() {
    spdlog::warn("Editor::undoEdit(), edit history has {} commands, lastEditSize is {}.", editHistory_.size(), lastEditSize_);
    if (lastEditSize_ == 0) {
        return;
    }
    board_.removeAllHighlights();
    --lastEditSize_;
    spdlog::info("Undo {}.", editHistory_[lastEditSize_]->getMessage());
    editHistory_[lastEditSize_]->undo();
}
void Editor::redoEdit() {
    spdlog::warn("Editor::redoEdit(), edit history has {} commands, lastEditSize is {}.", editHistory_.size(), lastEditSize_);
    if (lastEditSize_ >= editHistory_.size()) {
        return;
    }
    board_.removeAllHighlights();
    spdlog::info("Redo {}.", editHistory_[lastEditSize_]->getMessage());
    editHistory_[lastEditSize_]->execute();
    ++lastEditSize_;
}
void Editor::selectAll() {
    spdlog::warn("Editor::selectAll() NYI");
}
void Editor::deselectAll() {
    board_.removeAllHighlights();
    setCursorState(CursorState::empty);
}
void Editor::rotateArea(bool clockwise) {
    spdlog::warn("Editor::rotateArea() NYI");
}
void Editor::flipArea(bool vertical) {
    spdlog::warn("Editor::flipArea() NYI");
}
void Editor::editTile(bool toggleState) {
    spdlog::warn("Editor::editTile() NYI");
}
void Editor::copyArea() {
    auto bounds = board_.getHighlightedBounds();
    spdlog::info("Copy bounds is ({}, {}) to ({}, {}).", bounds.first.x, bounds.first.y, bounds.second.x, bounds.second.y);
    if (bounds.first.x > bounds.second.x) {
        return;
    }
    copySubBoard_.clear();
    copySubBoard_.copyFromBoard(board_, bounds.first, bounds.second, true);
    setCursorState(CursorState::pasteArea);
}
void Editor::pasteArea() {
    if (copySubBoard_.getVisibleSize() != sf::Vector2u(0, 0)) {
        setCursorState(CursorState::pasteArea);
    }
}
void Editor::deleteArea() {
    auto bounds = board_.getHighlightedBounds();
    if (bounds.first.x > bounds.second.x) {
        return;
    }
    auto command = makeCommand<commands::FillArea>(board_, tilePool_);
    forEachTile(board_, bounds.first, bounds.second, [&command](Chunk& chunk, int i, int x, int y) {
        Tile tile = chunk.accessTile(i);
        if (tile.getHighlight()) {
            command->pushBackTile({x, y}).setType(tiles::Blank::instance());
        }
    });
    executeCommand(std::move(command));
    board_.removeAllHighlights();
}
void Editor::wireTool() {
    spdlog::warn("Editor::wireTool() NYI");
}
void Editor::queryTool() {
    spdlog::warn("Editor::queryTool() NYI");
}
void Editor::pickTile(TileId::t id) {
    if (id == TileId::blank) {
        tileSubBoard_.accessTile(0, 0).setType(tiles::Blank::instance());
    } else if (id <= TileId::wireCrossover) {
        tileSubBoard_.accessTile(0, 0).setType(tiles::Wire::instance(), id);
    } else if (id <= TileId::inButton) {
        tileSubBoard_.accessTile(0, 0).setType(tiles::Input::instance(), id);
    } else if (id <= TileId::outLed) {
        tileSubBoard_.accessTile(0, 0).setType(tiles::Led::instance());
    } else if (id <= TileId::gateXnor) {
        tileSubBoard_.accessTile(0, 0).setType(tiles::Gate::instance(), id);
    } else if (id <= TileId::label) {
        tileSubBoard_.accessTile(0, 0).setType(tiles::Label::instance());
    } else {
        assert(false);
    }
    tileSubBoard_.setVisibleSize({1, 1});
    setCursorState(CursorState::pickTile);
}

void Editor::setCursorState(CursorState state) {
    if (selectionStart_.second) {
        selectionStart_.second = false;
    }
    cursorState_ = state;
}

OffsetView Editor::findTileView(int x, int y) const {
    OffsetView view(editView_.getViewDivisor());
    view.setSize(editView_.getSize());
    view.setCenter(0.0f, 0.0f);
    constexpr int widthLog2 = constLog2(Chunk::WIDTH);
    view.setCenterOffset(
        (x >> widthLog2) - static_cast<int>(view.getSize().x / 2.0f / view.getViewDivisor()) - 1,
        (y >> widthLog2) - static_cast<int>(view.getSize().y / 2.0f / view.getViewDivisor()) - 1
    );
    view.setCenter(
        view.getCenter().x + ((x & (Chunk::WIDTH - 1)) + 0.5f) * TileWidth::TEXELS,
        view.getCenter().y + ((y & (Chunk::WIDTH - 1)) + 0.5f) * TileWidth::TEXELS
    );
    return view;
}

OffsetView Editor::findTileView(const sf::Vector2i& pos) const {
    return findTileView(pos.x, pos.y);
}

std::pair<sf::Vector2i, bool> Editor::mapMouseToNearestTile(const sf::Vector2i& mousePos) const {
    using Vector2ll = sf::Vector2<long long>;
    sf::Vector2f pos = editView_.getCenter() - editView_.getSize() * 0.5f + sf::Vector2f(mousePos) * zoomLevel_;
    Vector2ll tilePos = {
        static_cast<long long>(editView_.getCenterOffset().x) * Chunk::WIDTH + static_cast<long long>(std::floor(pos.x / TileWidth::TEXELS)),
        static_cast<long long>(editView_.getCenterOffset().y) * Chunk::WIDTH + static_cast<long long>(std::floor(pos.y / TileWidth::TEXELS))
    };
    const auto lowerBound = static_cast<Vector2ll>(board_.getTileLowerBound());
    const auto upperBound = static_cast<Vector2ll>(board_.getTileUpperBound());
    Vector2ll tilePosInBounds = {
        std::min(std::max(tilePos.x, lowerBound.x), upperBound.x),
        std::min(std::max(tilePos.y, lowerBound.y), upperBound.y)
    };
    if (tilePosInBounds == tilePos) {
        return {static_cast<sf::Vector2i>(tilePosInBounds), true};
    } else {
        return {static_cast<sf::Vector2i>(tilePosInBounds), false};
    }
}

void Editor::updateCursor() {
    const auto cursorCoords = mapMouseToNearestTile(mousePos_);
    if (cursorCoords_.first != cursorCoords.first) {
        //spdlog::debug("Cursor coords = {}, {}.", cursorCoords.first.x, cursorCoords.first.y);
        cursorLabel_.setString("(" + std::to_string(cursorCoords.first.x) + ", " + std::to_string(cursorCoords.first.y) + ")");
    }
    if (!cursorCoords.second) {
        cursorVisible_ = false;
    }
    cursorCoords_ = cursorCoords;
    cursor_.setPosition(static_cast<sf::Vector2f>((cursorCoords.first - editView_.getCenterOffset() * Chunk::WIDTH) * static_cast<int>(TileWidth::TEXELS)));
    cursorLabel_.setPosition(editView_.getCenter() + editView_.getSize() * 0.5f - cursorLabel_.getLocalBounds().getSize() * zoomLevel_);
    cursorLabel_.setScale(zoomLevel_, zoomLevel_);
}

void Editor::updateSelection(const sf::Vector2i& newSelectionEnd) {
    if (selectionEnd_ == newSelectionEnd) {
        return;
    }

    // The selection area is broken up into quadrants, with the selection start
    // at the center of the four. When the selection end moves, we need to
    // highlight two rectangular areas (each area either adds or removes the
    // highlight) that represent the difference to match the new selection area.

    // Check for trivial case: the new selection end moved to a different quadrant.
    if ((selectionEnd_.x - selectionStart_.first.x < 0) != (newSelectionEnd.x - selectionStart_.first.x < 0) ||
        (selectionEnd_.y - selectionStart_.first.y < 0) != (newSelectionEnd.y - selectionStart_.first.y < 0)) {

        highlightArea(selectionStart_.first, selectionEnd_, false);
        selectionEnd_ = newSelectionEnd;
        highlightArea(selectionStart_.first, selectionEnd_, true);
        return;
    }

    // Nontrivial case: the new selection end stayed in the same quadrant.
    sf::Vector2i a1 = selectionStart_.first, b1 = selectionStart_.first;
    sf::Vector2i a2 = selectionStart_.first, b2 = selectionStart_.first;
    bool highlight1 = true, highlight2 = true;
    if (newSelectionEnd.x >= selectionStart_.first.x) {
        // Right quadrants.
        if (newSelectionEnd.x > selectionEnd_.x) {
            a1 = {selectionEnd_.x + 1, selectionStart_.first.y};
            b1 = newSelectionEnd;
            highlight1 = true;
        } else if (newSelectionEnd.x < selectionEnd_.x) {
            a1 = {selectionEnd_.x, selectionStart_.first.y};
            b1 = {newSelectionEnd.x + 1, selectionEnd_.y};
            highlight1 = false;
        }
    } else {
        // Left quadrants.
        if (newSelectionEnd.x < selectionEnd_.x) {
            a1 = {selectionEnd_.x - 1, selectionStart_.first.y};
            b1 = newSelectionEnd;
            highlight1 = true;
        } else if (newSelectionEnd.x > selectionEnd_.x) {
            a1 = {selectionEnd_.x, selectionStart_.first.y};
            b1 = {newSelectionEnd.x - 1, selectionEnd_.y};
            highlight1 = false;
        }
    }
    if (newSelectionEnd.y >= selectionStart_.first.y) {
        // Bottom quadrants.
        if (newSelectionEnd.y > selectionEnd_.y) {
            a2 = {selectionStart_.first.x, selectionEnd_.y + 1};
            b2 = newSelectionEnd;
            highlight2 = true;
        } else if (newSelectionEnd.y < selectionEnd_.y) {
            a2 = {selectionStart_.first.x, selectionEnd_.y};
            b2 = {selectionEnd_.x, newSelectionEnd.y + 1};
            highlight2 = false;
        }
    } else {
        // Top quadrants.
        if (newSelectionEnd.y < selectionEnd_.y) {
            a2 = {selectionStart_.first.x, selectionEnd_.y - 1};
            b2 = newSelectionEnd;
            highlight2 = true;
        } else if (newSelectionEnd.y > selectionEnd_.y) {
            a2 = {selectionStart_.first.x, selectionEnd_.y};
            b2 = {selectionEnd_.x, newSelectionEnd.y - 1};
            highlight2 = false;
        }
    }

    // Ensure that we remove highlights first before adding any.
    if (!highlight1 || highlight2) {
        highlightArea(a1, b1, highlight1);
        highlightArea(a2, b2, highlight2);
    } else {
        highlightArea(a2, b2, highlight2);
        highlightArea(a1, b1, highlight1);
    }
    auto tile = board_.accessTile(selectionStart_.first);
    if (!tile.getHighlight()) {
        tile.setHighlight(true);
    }

    selectionEnd_ = newSelectionEnd;
}

void Editor::highlightArea(sf::Vector2i a, sf::Vector2i b, bool highlight) {
    sf::Vector2i aCopy = a;
    a.x = std::min(aCopy.x, b.x);
    a.y = std::min(aCopy.y, b.y);
    b.x = std::max(aCopy.x, b.x);
    b.y = std::max(aCopy.y, b.y);

    forEachTile(board_, a, b, [highlight](Chunk& chunk, int i, int, int) {
        chunk.accessTile(i).setHighlight(highlight);
    });
}

template<typename T, typename... Args>
std::unique_ptr<T> Editor::makeCommand(Args&&... args) {
    if (editHistory_.size() > 0 && lastEditSize_ == editHistory_.size()) {
        // If the command has the same type as the previous one and happened
        // recently, we can just add on to it instead of building a new command.

        Command& lastCommand = *editHistory_.back();
        if (lastCommand.isGroupingAllowed() && lastCommand.getTimeSinceLastExecute().count() < 1000 && typeid(lastCommand) == typeid(T)) {
            spdlog::debug("Last command has same type, returning it.");
            auto lastCommandPtr = static_cast<T*>(editHistory_.back().release());
            editHistory_.pop_back();
            --lastEditSize_;
            return std::unique_ptr<T>(lastCommandPtr);
        }

        // FIXME: eventually we may want to replace the above typeid checking with a function that gets a command-type from an enum.
        // could use a string instead, then we don't need an enum that is aware of all the commands.
        // the command-type id could be passed as first argument to command ctor?
    }
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

void Editor::executeCommand(std::unique_ptr<Command>&& command) {
    command->execute();
    if (lastEditSize_ < editHistory_.size()) {
        editHistory_.resize(lastEditSize_);
    }
    editHistory_.emplace_back(std::move(command));
    lastEditSize_ = editHistory_.size();
}

void Editor::pasteToBoard(const sf::Vector2i& tilePos, bool deltaCheck) {
    static sf::Vector2i lastTilePos = {0, 0};
    if (lastTilePos == tilePos && deltaCheck) {
        return;
    }
    lastTilePos = tilePos;

    const bool forcePaste = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::RShift);
    const bool ignoreBlanks = sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl);

    if (!board_.accessTile(tilePos).getHighlight()) {
        // Not pasting into a selection.
        if (cursorState_ == CursorState::pickTile) {
            auto command = makeCommand<commands::PlaceTiles>(board_, tilePool_);
            tileSubBoard_.accessTile(0, 0).cloneTo(command->pushBackTile(tilePos));
            executeCommand(std::move(command));
        } else if (cursorState_ == CursorState::pasteArea) {
            if (!forcePaste) {
                using Vector2ll = sf::Vector2<long long>;
                const auto secondPos = static_cast<Vector2ll>(tilePos) + static_cast<Vector2ll>(copySubBoard_.getVisibleSize()) - Vector2ll(1ll, 1ll);
                const auto upperBound = static_cast<Vector2ll>(board_.getTileUpperBound());
                if (secondPos.x > upperBound.x || secondPos.y > upperBound.y) {
                    return;
                }

                bool foundNonBlank = false;
                forEachTile(board_, tilePos, static_cast<sf::Vector2i>(secondPos), [this,tilePos,ignoreBlanks,&foundNonBlank](Chunk& chunk, int i, int x, int y) {
                    if (chunk.accessTile(i).getId() != TileId::blank) {
                        if (!ignoreBlanks || copySubBoard_.accessTile(x - tilePos.x, y - tilePos.y).getId() != TileId::blank) {
                            foundNonBlank = true;
                        }
                    }
                });
                if (foundNonBlank) {
                    return;
                }
            }
            auto command = makeCommand<commands::PlaceTiles>(board_, tilePool_);
            copySubBoard_.pasteToBoard(*command, tilePos, ignoreBlanks);
            executeCommand(std::move(command));
        }
        board_.removeAllHighlights();
    } else {
        // Pasting into selection.
        auto bounds = board_.getHighlightedBounds();
        if (bounds.first.x > bounds.second.x) {
            return;
        }
        if (cursorState_ == CursorState::pickTile) {
            auto command = makeCommand<commands::FillArea>(board_, tilePool_);
            forEachTile(board_, bounds.first, bounds.second, [this,&command](Chunk& chunk, int i, int x, int y) {
                Tile tile = chunk.accessTile(i);
                if (tile.getHighlight()) {
                    tileSubBoard_.accessTile(0, 0).cloneTo(command->pushBackTile({x, y}));
                }
            });
            executeCommand(std::move(command));
        } else if (cursorState_ == CursorState::pasteArea) {
            auto command = makeCommand<commands::FillArea>(board_, tilePool_);
            for (long long y = bounds.first.y; y <= bounds.second.y - static_cast<long long>(copySubBoard_.getVisibleSize().y) + 1ll; y += copySubBoard_.getVisibleSize().y) {
                for (long long x = bounds.first.x; x <= bounds.second.x - static_cast<long long>(copySubBoard_.getVisibleSize().x) + 1ll; x += copySubBoard_.getVisibleSize().x) {
                    const sf::Vector2i endPos = {
                        static_cast<int>(x) + static_cast<int>(copySubBoard_.getVisibleSize().x) - 1,
                        static_cast<int>(y) + static_cast<int>(copySubBoard_.getVisibleSize().y) - 1
                    };
                    bool foundNonHighlight = false;
                    forEachTile(board_, {static_cast<int>(x), static_cast<int>(y)}, endPos, [&foundNonHighlight](Chunk& chunk, int i, int, int) {
                        if (!chunk.accessTile(i).getHighlight()) {
                            foundNonHighlight = true;
                        }
                    });
                    if (!foundNonHighlight) {
                        copySubBoard_.pasteToBoard(*command, {static_cast<int>(x), static_cast<int>(y)}, ignoreBlanks);
                    }
                }
            }
            executeCommand(std::move(command));
        }
    }
}

void Editor::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (!cursorVisible_) {
        return;
    }
    if (cursorState_ == CursorState::empty) {
        target.draw(cursor_, states);
    } else if (cursorState_ == CursorState::pickTile) {
        target.draw(tileSubBoard_, states);
    } else if (cursorState_ == CursorState::pasteArea) {
        target.draw(copySubBoard_, states);

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::RShift)) {
            const sf::Vector2f outlineSize = static_cast<sf::Vector2f>(copySubBoard_.getVisibleSize()) * static_cast<float>(TileWidth::TEXELS);
            sf::VertexArray outline(sf::LineStrip, 5);
            outline[0].position = {0.0f, 0.0f};
            outline[1].position = {outlineSize.x, 0.0f};
            outline[2].position = {outlineSize.x, outlineSize.y};
            outline[3].position = {0.0f, outlineSize.y};
            outline[4].position = {0.0f, 0.0f};
            for (size_t i = 0; i < outline.getVertexCount(); ++i) {
                outline[i].color = sf::Color::Red;
            }
            sf::RenderStates states2 = states;
            states2.transform.translate(copySubBoard_.getRenderPosition());
            target.draw(outline, states2);
        }
    }
    target.draw(cursorLabel_, states);
}
