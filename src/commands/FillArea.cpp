#include <commands/FillArea.h>

namespace commands {

FillArea::FillArea(Board& board, TilePool& pool) :
    WriteTiles(board, pool) {
}

std::string FillArea::getMessage() const {
    return "fill " + std::to_string(getTileCount()) + " tiles";
}

void FillArea::execute() {
    WriteTiles::execute();
}

void FillArea::undo() {
    WriteTiles::undo();
}

} // namespace commands
