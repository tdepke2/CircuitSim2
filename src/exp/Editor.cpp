#include <Chunk.h>
#include <Editor.h>

#include <cmath>
#include <spdlog/spdlog.h>
#include <string>

unsigned int Editor::tileWidth_;

void Editor::setup(unsigned int tileWidth) {
    tileWidth_ = tileWidth;
}

Editor::Editor(Board& board, const sf::Font& font) :
    board_(board),
    offsetView_(0.0f),
    zoomLevel_(0.0f),
    mousePos_(),
    cursor_({static_cast<float>(tileWidth_), static_cast<float>(tileWidth_)}),
    cursorCoords_(0, 0),
    cursorLabel_("", font) {

    cursor_.setFillColor({255, 80, 255, 100});
    cursorLabel_.setOutlineColor(sf::Color::Black);
    cursorLabel_.setOutlineThickness(1.0f);
}

void Editor::processEvent(const sf::Event& event) {
    if (event.type == sf::Event::MouseMoved) {
        mousePos_.x = event.mouseMove.x;
        mousePos_.y = event.mouseMove.y;
    }
}

void Editor::update(const OffsetView& offsetView, float zoom) {
    offsetView_ = offsetView;
    zoomLevel_ = zoom;
    updateCursor();
}

void Editor::updateCursor() {
    sf::Vector2f pos = offsetView_.getCenter() - offsetView_.getSize() * 0.5f + sf::Vector2f(mousePos_) * zoomLevel_;
    pos.x -= std::fmod(pos.x, static_cast<float>(tileWidth_));
    pos.y -= std::fmod(pos.y, static_cast<float>(tileWidth_));
    cursor_.setPosition(pos);
    sf::Vector2i cursorCoords = {
        offsetView_.getCenterOffset().x * Chunk::WIDTH + static_cast<int>(std::lround(pos.x / tileWidth_)),
        offsetView_.getCenterOffset().y * Chunk::WIDTH + static_cast<int>(std::lround(pos.y / tileWidth_))
    };
    if (cursorCoords_ != cursorCoords) {
        spdlog::debug("Cursor coords = {}, {}.", cursorCoords.x, cursorCoords.y);
        cursorLabel_.setString("(" + std::to_string(cursorCoords.x) + ", " + std::to_string(cursorCoords.y) + ")");
    }
    cursorCoords_ = cursorCoords;
    cursorLabel_.setPosition(offsetView_.getCenter() + offsetView_.getSize() * 0.5f - cursorLabel_.getLocalBounds().getSize());
}

void Editor::draw(sf::RenderTarget& target, sf::RenderStates states) const {

    target.draw(cursor_, states);
    target.draw(cursorLabel_, states);
}
