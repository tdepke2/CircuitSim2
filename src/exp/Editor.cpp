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
    cursorCoords_(-1, -1),
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
    } else if (event.type == sf::Event::MouseEntered) {
        mouseOnScreen_ = true;
    } else if (event.type == sf::Event::MouseLeft) {
        mouseOnScreen_ = false;
    } else if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Right) {
            if (!selectionStart_.second) {
                selectionStart_.first = mapMouseToTile({event.mouseButton.x, event.mouseButton.y});
                selectionStart_.second = true;
                board_.accessTile(selectionStart_.first).setHighlight(true);    // FIXME for test
            }
        }
    } else if (event.type == sf::Event::MouseButtonReleased) {
        if (event.mouseButton.button == sf::Mouse::Right) {
            if (selectionStart_.second) {
                selectionStart_.second = false;
                board_.accessTile(mapMouseToTile({event.mouseButton.x, event.mouseButton.y})).setHighlight(true);    // FIXME for test
            }
        }
    } else if (event.type == sf::Event::MouseWheelScrolled) {
        float zoomMult = 1.0f + (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::RShift)) * 5.0f;
        float zoomDelta = event.mouseWheelScroll.delta * zoomMult * zoomLevel_ * -0.04f;
        constexpr float maxZoom = 31.0f;
        static_assert(maxZoom < (1 << ChunkRender::LEVELS_OF_DETAIL), "Maximum zoom level must not exceed the total levels of detail.");

        zoomLevel_ += zoomDelta;
        zoomLevel_ = std::min(std::max(zoomLevel_, 0.2f), maxZoom);
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

sf::Vector2i Editor::mapMouseToTile(const sf::Vector2i& mousePos) {
    sf::Vector2f pos = editView_.getCenter() - editView_.getSize() * 0.5f + sf::Vector2f(mousePos) * zoomLevel_;
    pos.x -= std::fmod(pos.x, static_cast<float>(tileWidth_));
    pos.y -= std::fmod(pos.y, static_cast<float>(tileWidth_));
    return {
        editView_.getCenterOffset().x * Chunk::WIDTH + static_cast<int>(std::lround(pos.x / tileWidth_)),
        editView_.getCenterOffset().y * Chunk::WIDTH + static_cast<int>(std::lround(pos.y / tileWidth_))
    };
}

void Editor::updateCursor() {
    sf::Vector2i cursorCoords = mapMouseToTile(mousePos_);
    if (cursorCoords_ != cursorCoords) {
        spdlog::debug("Cursor coords = {}, {}.", cursorCoords.x, cursorCoords.y);
        cursor_.setPosition(static_cast<sf::Vector2f>((cursorCoords - editView_.getCenterOffset() * Chunk::WIDTH) * static_cast<int>(tileWidth_)));
        cursorCoords_ = cursorCoords;
        cursorLabel_.setString("(" + std::to_string(cursorCoords.x) + ", " + std::to_string(cursorCoords.y) + ")");
    }
    cursorLabel_.setPosition(editView_.getCenter() + editView_.getSize() * 0.5f - cursorLabel_.getLocalBounds().getSize() * zoomLevel_);
    cursorLabel_.setScale(zoomLevel_, zoomLevel_);

    // FIXME some new problems here, left mouse and drag doesn't update cursor correctly across chunk boundary
}

void Editor::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (!mouseOnScreen_) {
        return;
    }
    target.draw(cursor_, states);
    target.draw(cursorLabel_, states);
}
