#include <Board.h>
#include <Chunk.h>
#include <ChunkRender.h>
#include <Editor.h>
#include <ResourceManager.h>

#include <cmath>
#include <spdlog/spdlog.h>
#include <string>

sf::Texture* Editor::tilesetGrid_;
sf::Texture* Editor::tilesetNoGrid_;
unsigned int Editor::tileWidth_;

namespace {

constexpr int constLog2(int x) {
    return x == 1 ? 0 : 1 + constLog2(x / 2);
}

}

void Editor::setupTextureData(sf::Texture* tilesetGrid, sf::Texture* tilesetNoGrid, unsigned int tileWidth) {
    tilesetGrid_ = tilesetGrid;
    tilesetNoGrid_ = tilesetNoGrid;
    tileWidth_ = tileWidth;
    ChunkGroup::setup(tileWidth);
}

Editor::Editor(Board& board, ResourceManager& resource, const sf::View& initialView) :
    board_(board),
    editView_(static_cast<float>(tileWidth_ * Chunk::WIDTH), initialView),
    zoomLevel_(1.0f),
    mousePos_(),
    mouseOnScreen_(false),
    windowSize_(initialView.getSize()),
    cursor_({static_cast<float>(tileWidth_), static_cast<float>(tileWidth_)}),
    cursorCoords_(0, 0),
    cursorLabel_("", resource.getFont("resources/consolas.ttf"), 22),
    selectionStart_({sf::Vector2i(), false}),
    selectionEnd_(),
    chunkGroup_() {

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

void Editor::processEvent(const sf::Event& event) {
    if (event.type == sf::Event::MouseMoved) {
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            editView_.setCenter({
                editView_.getCenter().x + (mousePos_.x - event.mouseMove.x) * zoomLevel_,
                editView_.getCenter().y + (mousePos_.y - event.mouseMove.y) * zoomLevel_
            });
        }
        mousePos_.x = event.mouseMove.x;
        mousePos_.y = event.mouseMove.y;
        mouseOnScreen_ = true;
        updateCursor();
    } else if (event.type == sf::Event::MouseEntered) {
        mouseOnScreen_ = true;
    } else if (event.type == sf::Event::MouseLeft) {
        mouseOnScreen_ = false;
    } else if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Right) {
            if (!selectionStart_.second) {
                deselectAll();
                selectionStart_.first = mapMouseToTile({event.mouseButton.x, event.mouseButton.y});
                selectionStart_.second = true;
                selectionEnd_ = selectionStart_.first;
            }
        }
    } else if (event.type == sf::Event::MouseButtonReleased) {
        if (event.mouseButton.button == sf::Mouse::Right) {
            if (selectionStart_.second) {
                updateSelection(mapMouseToTile({event.mouseButton.x, event.mouseButton.y}));
                selectionStart_.second = false;
            }
        }
    } else if (event.type == sf::Event::MouseWheelScrolled) {
        float zoomMult = 1.0f + (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::RShift)) * 5.0f;
        float zoomDelta = event.mouseWheelScroll.delta * zoomMult * zoomLevel_ * -0.04f;
        constexpr float minZoom = 0.2f, maxZoom = 31.0f;
        static_assert(maxZoom < (1 << ChunkRender::LEVELS_OF_DETAIL), "Maximum zoom level must not exceed the total levels of detail.");

        zoomLevel_ += zoomDelta;
        zoomLevel_ = std::min(std::max(zoomLevel_, minZoom), maxZoom);
        editView_.setSize(windowSize_.x * zoomLevel_, windowSize_.y * zoomLevel_);
    } else if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Escape) {
            deselectAll();
        }
    } else if (event.type == sf::Event::Resized) {
        windowSize_.x = event.size.width;
        windowSize_.y = event.size.height;
        editView_.setSize(static_cast<sf::Vector2f>(windowSize_) * zoomLevel_);
    }
}

void Editor::update() {
    updateCursor();
    chunkGroup_.setSize({2, 3});
    chunkGroup_.setPosition(cursor_.getPosition());
    // FIXME below will cause some issues, it's possible to view the chunkGroup at a position outside the normal bounds by panning the view down and to the right.
    chunkGroup_.setRenderArea(OffsetView(editView_.getViewDivisor(), sf::View({0.0f, 0.0f}, editView_.getSize())), zoomLevel_);
    sf::RenderStates states;
    states.texture = tilesetGrid_;
    chunkGroup_.drawChunks(states);

    /*static sf::Clock c;
    static int stage = 0;
    if (c.getElapsedTime().asSeconds() >= 0.5f) {
        c.restart();
        auto t1 = std::chrono::high_resolution_clock::now();
        if (stage == 0) {
            deselectAll();
            selectionStart_.first = {1, 1};
            selectionEnd_ = selectionStart_.first;
            updateSelection(selectionStart_.first);
        } else {
            updateSelection(selectionStart_.first * stage * 32);
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        spdlog::debug("stage {} took {}ms", stage, std::chrono::duration<double, std::milli>(t2 - t1).count());
        stage = (stage + 1) % 16;
    }*/
}

void Editor::deselectAll() {
    board_.removeAllHighlights();
}

sf::Vector2i Editor::mapMouseToTile(const sf::Vector2i& mousePos) const {
    sf::Vector2f pos = editView_.getCenter() - editView_.getSize() * 0.5f + sf::Vector2f(mousePos) * zoomLevel_;
    return {
        editView_.getCenterOffset().x * Chunk::WIDTH + static_cast<int>(std::floor(pos.x / tileWidth_)),
        editView_.getCenterOffset().y * Chunk::WIDTH + static_cast<int>(std::floor(pos.y / tileWidth_))
    };
}

void Editor::updateCursor() {
    sf::Vector2i cursorCoords = mapMouseToTile(mousePos_);
    if (cursorCoords_ != cursorCoords) {
        //spdlog::debug("Cursor coords = {}, {}.", cursorCoords.x, cursorCoords.y);
        cursorCoords_ = cursorCoords;
        cursorLabel_.setString("(" + std::to_string(cursorCoords.x) + ", " + std::to_string(cursorCoords.y) + ")");
    }
    cursor_.setPosition(static_cast<sf::Vector2f>((cursorCoords - editView_.getCenterOffset() * Chunk::WIDTH) * static_cast<int>(tileWidth_)));
    cursorLabel_.setPosition(editView_.getCenter() + editView_.getSize() * 0.5f - cursorLabel_.getLocalBounds().getSize() * zoomLevel_);
    cursorLabel_.setScale(zoomLevel_, zoomLevel_);

    if (selectionStart_.second) {
        updateSelection(cursorCoords_);
    }
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

    // Highlighting is done per-chunk as this is significantly faster than doing it per-tile.
    constexpr int widthLog2 = constLog2(Chunk::WIDTH);
    for (int yChunk = a.y >> widthLog2; yChunk <= b.y >> widthLog2; ++yChunk) {
        for (int xChunk = a.x >> widthLog2; xChunk <= b.x >> widthLog2; ++xChunk) {
            auto& chunk = board_.accessChunk(ChunkCoords::pack(xChunk, yChunk));
            for (int i = 0; i < Chunk::WIDTH * Chunk::WIDTH; ++i) {
                int x = i % Chunk::WIDTH + xChunk * Chunk::WIDTH;
                int y = i / Chunk::WIDTH + yChunk * Chunk::WIDTH;
                if (x >= a.x && x <= b.x && y >= a.y && y <= b.y) {
                    chunk.accessTile(i).setHighlight(highlight);
                }
            }
        }
    }
}

void Editor::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (!mouseOnScreen_) {
        return;
    }
    target.draw(cursor_, states);
    target.draw(chunkGroup_, states);
    target.draw(cursorLabel_, states);
}
