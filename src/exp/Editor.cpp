#include <Board.h>
#include <Chunk.h>
#include <ChunkRender.h>
#include <Editor.h>
#include <ResourceManager.h>

#include <cmath>
#include <spdlog/spdlog.h>
#include <string>

unsigned int Editor::tileWidth_;

namespace {

constexpr int constLog2(int x) {
    return x == 1 ? 0 : 1 + constLog2(x / 2);
}

}

void Editor::setup(unsigned int tileWidth) {
    tileWidth_ = tileWidth;
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
    selectionArea_(0, 0, 0, 0) {

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
        if (selectionStart_.second) {
            updateSelection(mapMouseToTile(mousePos_));
        }
    } else if (event.type == sf::Event::MouseEntered) {
        mouseOnScreen_ = true;
    } else if (event.type == sf::Event::MouseLeft) {
        mouseOnScreen_ = false;
    } else if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Right) {
            if (!selectionStart_.second) {
                // FIXME need to call deselect here instead of setting selectionArea?
                selectionArea_ = sf::IntRect(0, 0, 0, 0);

                selectionStart_.first = mapMouseToTile({event.mouseButton.x, event.mouseButton.y});
                selectionStart_.second = true;
                selectionEnd_ = selectionStart_.first;
                updateSelection(selectionStart_.first);
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
    } else if (event.type == sf::Event::Resized) {
        windowSize_.x = event.size.width;
        windowSize_.y = event.size.height;
        editView_.setSize(static_cast<sf::Vector2f>(windowSize_) * zoomLevel_);
    }
}

void Editor::update() {
    updateCursor();

    // FIXME need to updateSelection() if the cursor moved, also why not call updateCursor() as soon as a new event is processed instead of here?

    /*static sf::Clock c;
    static int stage = 0;
    if (c.getElapsedTime().asSeconds() >= 0.5f) {
        c.restart();
        auto t1 = std::chrono::high_resolution_clock::now();
        if (stage == 0) {
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
        spdlog::debug("Cursor coords = {}, {}.", cursorCoords.x, cursorCoords.y);
        cursorCoords_ = cursorCoords;
        cursorLabel_.setString("(" + std::to_string(cursorCoords.x) + ", " + std::to_string(cursorCoords.y) + ")");
    }
    cursor_.setPosition(static_cast<sf::Vector2f>((cursorCoords - editView_.getCenterOffset() * Chunk::WIDTH) * static_cast<int>(tileWidth_)));
    cursorLabel_.setPosition(editView_.getCenter() + editView_.getSize() * 0.5f - cursorLabel_.getLocalBounds().getSize() * zoomLevel_);
    cursorLabel_.setScale(zoomLevel_, zoomLevel_);
}

void Editor::updateSelection(const sf::Vector2i& newSelectionEnd) {
    /*for (int y = selectionArea_.top; y < selectionArea_.top + selectionArea_.height; ++y) {
        for (int x = selectionArea_.left; x < selectionArea_.left + selectionArea_.width; ++x) {
            board_.accessTile(x, y).setHighlight(false);
        }
    }

    // FIXME will need to improve to reduce number of tiles modified

    selectionArea_ = {
        std::min(selectionStart_.first.x, selectionEnd.x),
        std::min(selectionStart_.first.y, selectionEnd.y),
        std::abs(selectionStart_.first.x - selectionEnd.x) + 1,
        std::abs(selectionStart_.first.y - selectionEnd.y) + 1
    };

    for (int y = selectionArea_.top; y < selectionArea_.top + selectionArea_.height; ++y) {
        for (int x = selectionArea_.left; x < selectionArea_.left + selectionArea_.width; ++x) {
            board_.accessTile(x, y).setHighlight(true);
        }
    }*/

    /*auto a = ChunkCoords::pack(selectionArea_.left >> 5, selectionArea_.top >> 5);
    auto b = ChunkCoords::pack((selectionArea_.left + selectionArea_.width - 1) >> 5, (selectionArea_.top + selectionArea_.height - 1) >> 5);
    for (int y = ChunkCoords::y(a); y <= ChunkCoords::y(b); ++y) {
        for (int x = ChunkCoords::x(a); x <= ChunkCoords::x(b); ++x) {
            auto& chunk = board_.getChunk(ChunkCoords::pack(x, y));
            for (int i = 0; i < Chunk::WIDTH * Chunk::WIDTH; ++i) {
                chunk.accessTile(i).setHighlight(false);
            }
        }
    }

    selectionArea_ = {
        std::min(selectionStart_.first.x, selectionEnd.x),
        std::min(selectionStart_.first.y, selectionEnd.y),
        std::abs(selectionStart_.first.x - selectionEnd.x) + 1,
        std::abs(selectionStart_.first.y - selectionEnd.y) + 1
    };

    a = ChunkCoords::pack(selectionArea_.left >> 5, selectionArea_.top >> 5);
    b = ChunkCoords::pack((selectionArea_.left + selectionArea_.width - 1) >> 5, (selectionArea_.top + selectionArea_.height - 1) >> 5);
    for (int y = ChunkCoords::y(a); y <= ChunkCoords::y(b); ++y) {
        for (int x = ChunkCoords::x(a); x <= ChunkCoords::x(b); ++x) {
            auto& chunk = board_.getChunk(ChunkCoords::pack(x, y));
            for (int i = 0; i < Chunk::WIDTH * Chunk::WIDTH; ++i) {
                chunk.accessTile(i).setHighlight(true);
            }
        }
    }*/

    if (selectionEnd_ == newSelectionEnd) {
        return;
    }

    bool stayedInQuadrant = false;
    if ((selectionEnd_.x - selectionStart_.first.x < 0) == (newSelectionEnd.x - selectionStart_.first.x < 0) &&
        (selectionEnd_.y - selectionStart_.first.y < 0) == (newSelectionEnd.y - selectionStart_.first.y < 0)) {
        stayedInQuadrant = true;
    }
    std::string q = "n/a";
    if (stayedInQuadrant) {
        sf::Vector2i a1 = selectionStart_.first, b1 = selectionStart_.first;
        sf::Vector2i a2 = selectionStart_.first, b2 = selectionStart_.first;
        bool highlight1 = true, highlight2 = true;
        if (newSelectionEnd.x >= selectionStart_.first.x) {
            // right quads
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
            // left quads
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
            // bottom quads
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
            // top quads
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

        if (newSelectionEnd.x >= selectionStart_.first.x) {
            if (newSelectionEnd.y >= selectionStart_.first.y) {
                // quad bottom right
                q = "bottom right";
            } else {
                // quad top right
                q = "top right";
            }
        } else {
            if (newSelectionEnd.y >= selectionStart_.first.y) {
                // quad bottom left
                q = "bottom left";
            } else {
                // quad top left
                q = "top left";
            }
        }

        if (!highlight1 || highlight2) {
            highlightArea(a1, b1, highlight1);
            highlightArea(a2, b2, highlight2);
        } else {
            highlightArea(a2, b2, highlight2);
            highlightArea(a1, b1, highlight1);
        }

        selectionEnd_ = newSelectionEnd;
    } else {
        highlightArea(selectionStart_.first, selectionEnd_, false);
        selectionEnd_ = newSelectionEnd;
        highlightArea(selectionStart_.first, selectionEnd_, true);
    }
    spdlog::debug("Stayed in quad? {}, q = {}", stayedInQuadrant, q);

    selectionArea_ = {
        std::min(selectionStart_.first.x, selectionEnd_.x),
        std::min(selectionStart_.first.y, selectionEnd_.y),
        std::abs(selectionStart_.first.x - selectionEnd_.x) + 1,
        std::abs(selectionStart_.first.y - selectionEnd_.y) + 1
    };
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
    target.draw(cursorLabel_, states);
}
