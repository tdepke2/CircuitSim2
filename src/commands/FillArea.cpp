#include <Board.h>
#include <commands/FillArea.h>
#include <Tile.h>

namespace commands {

FillArea::FillArea(Board& board, TilePool& pool) :
    WriteTiles(board, pool) {
}

std::string FillArea::getMessage() const {
    return "fill " + std::to_string(getTileCount()) + " tiles";
}

void FillArea::execute() {
    Command::execute();
    for (size_t i = getLastExecuteSize(); i < getTilePositions().size(); ++i) {
        auto tile = getBoard().accessTile(getTilePositions()[i]);
        accessTile(i).swapWith(tile);
        tile.setHighlight(true);
    }
    setLastExecuteSize(getTilePositions().size());
}

void FillArea::undo() {
    for (size_t i = getLastExecuteSize(); i > 0; --i) {
        auto tile = getBoard().accessTile(getTilePositions()[i - 1]);
        accessTile(i - 1).swapWith(tile);
        tile.setHighlight(true);
    }
    setLastExecuteSize(0);
}

} // namespace commands
