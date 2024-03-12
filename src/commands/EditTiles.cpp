#include <commands/EditTiles.h>

namespace commands {

EditTiles::EditTiles(Board& board, TilePool& pool) :
    PlaceTiles(board, pool) {
}

std::string EditTiles::getMessage() const {
    return "edit " + std::to_string(getTileCount()) + " tiles";
}

} // namespace commands
