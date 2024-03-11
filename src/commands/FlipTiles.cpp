#include <Board.h>
#include <commands/FlipTiles.h>

namespace commands {

FlipTiles::FlipTiles(Board& board, bool acrossVertical, bool highlightTiles) :
    board_(board),
    tilePositions_(),
    acrossVertical_(acrossVertical),
    highlightTiles_(highlightTiles) {
}

void FlipTiles::pushBackTile(const sf::Vector2i& pos) {
    tilePositions_.push_back(pos);
}

std::string FlipTiles::getMessage() const {
    return "flip " + std::to_string(tilePositions_.size()) + " tiles";
}

bool FlipTiles::isGroupingAllowed() const {
    return false;
}

void FlipTiles::execute() {
    Command::execute();
    for (size_t i = 0; i < tilePositions_.size(); ++i) {
        auto tile = board_.accessTile(tilePositions_[i]);
        tile.flip(acrossVertical_);
        if (highlightTiles_) {
            tile.setHighlight(true);
        }
    }
}

void FlipTiles::undo() {
    for (size_t i = 0; i < tilePositions_.size(); ++i) {
        auto tile = board_.accessTile(tilePositions_[i]);
        tile.flip(acrossVertical_);
        if (highlightTiles_) {
            tile.setHighlight(true);
        }
    }
}

} // namespace commands
