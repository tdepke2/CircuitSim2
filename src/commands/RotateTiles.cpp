#include <Board.h>
#include <commands/RotateTiles.h>

namespace commands {

RotateTiles::RotateTiles(Board& board, bool clockwise, bool highlightTiles) :
    board_(board),
    tilePositions_(),
    clockwise_(clockwise),
    highlightTiles_(highlightTiles) {
}

void RotateTiles::pushBackTile(const sf::Vector2i& pos) {
    tilePositions_.push_back(pos);
}

std::string RotateTiles::getMessage() const {
    return "rotate " + std::to_string(tilePositions_.size()) + " tiles";
}

bool RotateTiles::isGroupingAllowed() const {
    return false;
}

void RotateTiles::execute() {
    Command::execute();
    int dirDelta = (clockwise_ ? 1 : 3);
    for (size_t i = 0; i < tilePositions_.size(); ++i) {
        auto tile = board_.accessTile(tilePositions_[i]);
        tile.setDirection(static_cast<Direction::t>((tile.getDirection() + dirDelta) % 4));
        if (highlightTiles_) {
            tile.setHighlight(true);
        }
    }
}

void RotateTiles::undo() {
    int dirDelta = (clockwise_ ? 3 : 1);
    for (size_t i = 0; i < tilePositions_.size(); ++i) {
        auto tile = board_.accessTile(tilePositions_[i]);
        tile.setDirection(static_cast<Direction::t>((tile.getDirection() + dirDelta) % 4));
        if (highlightTiles_) {
            tile.setHighlight(true);
        }
    }
}

} // namespace commands
