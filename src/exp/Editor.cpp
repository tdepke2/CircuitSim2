#include <Board.h>
#include <Chunk.h>
#include <ChunkRender.h>
#include <Editor.h>
#include <ResourceManager.h>

#include <cmath>
#include <spdlog/spdlog.h>
#include <string>

unsigned int Editor::tileWidth_;

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

void Editor::updateSelection(const sf::Vector2i& selectionEnd) {
    for (int y = selectionArea_.top; y < selectionArea_.top + selectionArea_.height; ++y) {
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
    }
}

void Editor::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (!mouseOnScreen_) {
        return;
    }
    target.draw(cursor_, states);
    target.draw(cursorLabel_, states);
}
