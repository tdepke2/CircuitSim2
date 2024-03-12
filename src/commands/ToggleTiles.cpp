#include <Board.h>
#include <commands/ToggleTiles.h>
#include <tiles/Wire.h>

namespace commands {

ToggleTiles::ToggleTiles(Board& board, bool highlightTiles) :
    board_(board),
    toggleData_(),
    highlightTiles_(highlightTiles) {
}

void ToggleTiles::pushBackTile(const sf::Vector2i& pos) {
    const Tile tile = board_.accessTile(pos);
    ToggleData data;
    data.pos = pos;
    data.state1 = tile.getState();
    if (tile.getType() == tiles::Wire::instance()) {
        data.state2 = tile.call<tiles::Wire>(&tiles::Wire::getState2);
    }
    data.newState = (tile.getState() == State::high ? State::low : State::high);
    toggleData_.emplace_back(std::move(data));
}

std::string ToggleTiles::getMessage() const {
    return "toggle " + std::to_string(toggleData_.size()) + " tiles";
}

bool ToggleTiles::isGroupingAllowed() const {
    return false;
}

void ToggleTiles::execute() {
    Command::execute();
    for (size_t i = 0; i < toggleData_.size(); ++i) {
        auto tile = board_.accessTile(toggleData_[i].pos);
        tile.setState(toggleData_[i].newState);
        if (highlightTiles_) {
            tile.setHighlight(true);
        }
    }
}

void ToggleTiles::undo() {
    for (size_t i = 0; i < toggleData_.size(); ++i) {
        auto tile = board_.accessTile(toggleData_[i].pos);
        tile.setState(toggleData_[i].state1);
        if (tile.getType() == tiles::Wire::instance()) {
            tile.call<tiles::Wire>(&tiles::Wire::setState2, toggleData_[i].state2);
        }
        if (highlightTiles_) {
            tile.setHighlight(true);
        }
    }
}

} // namespace commands
